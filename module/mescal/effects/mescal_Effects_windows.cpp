#include "mescal_Effects_windows.h"

namespace mescal
{
    struct Effect::Pimpl
    {
        Pimpl(Type effectType_) :
            effectType(effectType_)
        {
        }

        ~Pimpl()
        {
        }

        void createResources()
        {
            juce::SharedResourcePointer<juce::DirectX> directX;

            if (!adapter)
            {
                adapter = directX->adapters.getDefaultAdapter();
            }

            if (adapter && !deviceContext)
            {
                winrt::com_ptr<ID2D1DeviceContext1> deviceContext1;
                if (const auto hr = adapter->direct2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
                    deviceContext1.put());
                    FAILED(hr))
                {
                    jassertfalse;
                    return;
                }

                deviceContext = deviceContext1.as<ID2D1DeviceContext2>();
            }

            if (deviceContext && !d2dEffect)
            {
                if (const auto hr = deviceContext->CreateEffect(*effectGuids[(size_t)effectType], d2dEffect.put());
                    FAILED(hr))
                {
                    jassertfalse;
                }
            }

            if (d2dEffect)
            {
                uint32_t maxNumInputs = 0;

                if (auto hr = d2dEffect->GetValue(D2D1_PROPERTY_MAX_INPUTS, &maxNumInputs); FAILED(hr))
                {
                    maxNumInputs = 1;
                }

                //
                // Some effects can accept MAX_INT inputs but only the first two seem to be usable
                //
                maxNumInputs = juce::jmin(2u, maxNumInputs);

                inputs.resize(maxNumInputs);

#if 0
                if (effectType == Type::spotSpecularLighting)
                {
                    static bool go = true;
                    if (go)
                    {
                        go = false;
                        dumpPropertiesRecursive(d2dEffect.get(), SpotSpecular::lightPosition);

                    }
                }
#endif
            }
        }

        static juce::String getName(ID2D1Properties* properties, int propertyIndex)
        {
            WCHAR stringBuffer[256];

            [[maybe_unused]] auto hr = properties->GetPropertyName(propertyIndex, stringBuffer, sizeof(stringBuffer) / sizeof(WCHAR));
            jassert(SUCCEEDED(hr));

            return juce::String{ stringBuffer };
        }

#if JUCE_DEBUG
        static void dumpPropertiesRecursive(ID2D1Properties* properties, int propertyIndex, int depth = 0)
        {
            juce::String indent = juce::String::repeatedString(" ", depth * 3);
            auto name = getName(properties, propertyIndex);

            auto count = properties->GetPropertyCount();
            for (uint32_t subpropertyIndex = 0; subpropertyIndex < count; ++subpropertyIndex)
            {
                winrt::com_ptr<ID2D1Properties> subproperties;
                if (auto hr = properties->GetSubProperties(subpropertyIndex, subproperties.put()); SUCCEEDED(hr))
                {
                    dumpPropertiesRecursive(subproperties.get(), subpropertyIndex, depth + 1);
                }
            }

            {
                std::array<D2D1_SUBPROPERTY, 7> indices
                {
                    D2D1_SUBPROPERTY_DISPLAYNAME,
                    D2D1_SUBPROPERTY_ISREADONLY,
                    D2D1_SUBPROPERTY_MIN,
                    D2D1_SUBPROPERTY_MAX,
                    D2D1_SUBPROPERTY_DEFAULT,
                    D2D1_SUBPROPERTY_FIELDS,
                    D2D1_SUBPROPERTY_INDEX
                };

                for (auto index : indices)
                {
                    auto type = properties->GetType(index);
                    DBG(indent << " " << juce::String::toHexString((int)index) << " type " << (int)type << " '" << getName(properties, index) << "'");

                    switch (type)
                    {
                    case D2D1_PROPERTY_TYPE_STRING:
                    {
                        juce::HeapBlock<WCHAR> stringBuffer{ 1024 };
                        properties->GetValue(index, (BYTE*)stringBuffer.getData(), 512);
                        DBG(indent << juce::String{ stringBuffer });
                        break;
                    }

                    case D2D1_PROPERTY_TYPE_BOOL:
                    {
                        bool f;

                        properties->GetValue(index, (BYTE*)&f, sizeof(f));
                        DBG(indent << (int)f);

                        break;
                    }

                    case D2D1_PROPERTY_TYPE_FLOAT:
                    {
                        float f;

                        properties->GetValue(index, (BYTE*)&f, sizeof(f));
                        DBG(indent << f);

                        break;
                    }

                    case D2D1_PROPERTY_TYPE_VECTOR2:
                    {
                        D2D1_VECTOR_2F vector2;

                        properties->GetValue(index, (BYTE*)&vector2, sizeof(vector2));

                        break;
                    }
                    case D2D1_PROPERTY_TYPE_VECTOR3:
                    {
                        D2D1_VECTOR_3F vector3;

                        properties->GetValue(index, (BYTE*)&vector3, sizeof(vector3));

                        break;
                    }

                    default:
                    {
                        break;
                    }
                    }
                }
            }
        }
#endif

        int getNumProperties()
        {
            if (!d2dEffect)
                createResources();

            if (!d2dEffect)
                return 0;

            return d2dEffect->GetPropertyCount();
        }

        juce::String getPropertyName(int index)
        {
            WCHAR nameBuffer[256];

            if (!d2dEffect)
                createResources();

            if (!d2dEffect)
                return {};

            if (auto hr = d2dEffect->GetPropertyName(index, nameBuffer, sizeof(nameBuffer) / sizeof(WCHAR)); FAILED(hr))
            {
                jassertfalse;
                return {};
            }

            return juce::String{ nameBuffer };
        }

        PropertyInfo getPropertyInfo(int index)
        {
            PropertyInfo info;

            info.name = getPropertyName(index);
            auto type = d2dEffect->GetType(index);
            switch (type)
            {
            case D2D1_PROPERTY_TYPE_FLOAT:
            {
                winrt::com_ptr<ID2D1Properties> subproperties;
                if (auto hr = d2dEffect->GetSubProperties(index, subproperties.put()); SUCCEEDED(hr))
                {
                    float min = subproperties->GetValue<float>(D2D1_SUBPROPERTY_MIN);
                    float max = subproperties->GetValue<float>(D2D1_SUBPROPERTY_MAX);
                    info.range = juce::Range<float>{ min, max };
                }
                break;
            }

            case D2D1_PROPERTY_TYPE_ENUM:
            case D2D1_PROPERTY_TYPE_VECTOR3:
            case D2D1_PROPERTY_TYPE_VECTOR4:
            {
                winrt::com_ptr<ID2D1Properties> subproperties;
                if (auto hr = d2dEffect->GetSubProperties(index, subproperties.put()); SUCCEEDED(hr))
                {
                    winrt::com_ptr<ID2D1Properties> subpropertyFields;
                    if (hr = subproperties->GetSubProperties(D2D1_SUBPROPERTY_FIELDS, subpropertyFields.put()); SUCCEEDED(hr))
                    {
                        auto count = subpropertyFields->GetPropertyCount();
                        for (uint32_t i = 0; i < count; ++i)
                        {
                            WCHAR nameBuffer[256];
                            if (hr = subpropertyFields->GetPropertyName(i, nameBuffer, sizeof(nameBuffer) / sizeof(WCHAR)); SUCCEEDED(hr))
                            {
                                info.enumeration.add(juce::String{ nameBuffer });
                            }
                        }
                    }
                }
                break;
            }
            }

            return info;
        }

        template <typename T> T blockToValue(juce::MemoryBlock const& block)
        {
            return *reinterpret_cast<T const*>(block.getData());
        };

        template<> juce::String blockToValue<juce::String>(juce::MemoryBlock const& block)
        {
            return block.toString();
        }

        juce::AffineTransform matrixToAffineTransform(juce::MemoryBlock const& block)
        {
            auto matrix3x2 = (D2D1_MATRIX_3X2_F const* const)block.getData();
            juce::AffineTransform transform;
            transform.mat00 = matrix3x2->m11;
            transform.mat01 = matrix3x2->m12;
            transform.mat02 = matrix3x2->dx;
            transform.mat10 = matrix3x2->m21;
            transform.mat11 = matrix3x2->m22;
            transform.mat12 = matrix3x2->dy;
            return transform;
        }

        void setProperty(int index, const PropertyValue value)
        {
            [[maybe_unused]] HRESULT hr = S_OK;

            if (!d2dEffect)
                createResources();

            if (!d2dEffect)
                return;

            if (std::holds_alternative<bool>(value))
            {
                hr = d2dEffect->SetValue(index, (BOOL)std::get<bool>(value));
            }
            else if (std::holds_alternative<int>(value))
            {
                hr = d2dEffect->SetValue(index, std::get<int>(value));
            }
            else if (std::holds_alternative<float>(value))
            {
                d2dEffect->SetValue(index, std::get<float>(value));
            }
            else if (std::holds_alternative<Vector2>(value))
            {
                auto vector2 = std::get<Vector2>(value);
                d2dEffect->SetValue(index, D2D1_VECTOR_2F{ vector2[0], vector2[1] });
            }
            else if (std::holds_alternative<Vector3>(value))
            {
                auto vector3 = std::get<Vector3>(value);
                d2dEffect->SetValue(index, D2D1_VECTOR_3F{ vector3[0], vector3[1], vector3[2] });
            }
            else if (std::holds_alternative<Vector4>(value))
            {
                auto const& vector4 = std::get<Vector4>(value);
                d2dEffect->SetValue(index, D2D1_VECTOR_4F{ vector4[0], vector4[1], vector4[2], vector4[3] });
            }
            else if (std::holds_alternative<juce::AffineTransform>(value))
            {
                auto const& transform = std::get<juce::AffineTransform>(value);
                auto matrix = juce::D2DUtilities::transformToMatrix(transform);
                d2dEffect->SetValue(index, matrix);
            }
            else if (std::holds_alternative<juce::Colour>(value))
            {
                auto color = std::get<juce::Colour>(value);
                d2dEffect->SetValue(index, D2D1_VECTOR_4F{ color.getFloatRed(), color.getFloatGreen(), color.getFloatBlue(), color.getFloatAlpha() });
            }
            else
            {
                jassertfalse;
            }

            jassert(SUCCEEDED(hr));
        }

        PropertyValue getProperty(int index)
        {
            if (!d2dEffect)
                createResources();

            if (!d2dEffect)
                return {};

            auto type = d2dEffect->GetType(index);
            auto size = d2dEffect->GetValueSize(index);
            juce::MemoryBlock valueBlock{ size };

            if (auto hr = d2dEffect->GetValue(index, (uint8_t*)valueBlock.getData(), size); FAILED(hr))
            {
                jassertfalse;
                return {};
            }

            PropertyValue value;
            switch (type)
            {
            default:
                jassertfalse;
                break;

            case D2D1_PROPERTY_TYPE_STRING:
                return valueBlock.toString();

            case D2D1_PROPERTY_TYPE_BOOL:
                return blockToValue<bool>(valueBlock);

            case D2D1_PROPERTY_TYPE_UINT32:
                return blockToValue<uint32_t>(valueBlock);

            case D2D1_PROPERTY_TYPE_INT32:
                return blockToValue<int>(valueBlock);

            case D2D1_PROPERTY_TYPE_FLOAT:
                return blockToValue<float>(valueBlock);

            case D2D1_PROPERTY_TYPE_VECTOR2:
                return blockToValue<Vector2>(valueBlock);

            case D2D1_PROPERTY_TYPE_VECTOR3:
                return blockToValue<Vector3>(valueBlock);

            case D2D1_PROPERTY_TYPE_VECTOR4:
                return blockToValue<Vector4>(valueBlock);

            case D2D1_PROPERTY_TYPE_ENUM:
                return blockToValue<uint8_t>(valueBlock);

            case D2D1_PROPERTY_TYPE_MATRIX_3X2:
                return matrixToAffineTransform(valueBlock);
            }

            return {};
        }

        static void setInputsRecursive(Pimpl* pimpl)
        {
            for (int index = 0; index < pimpl->inputs.size(); ++index)
            {
                auto& input = pimpl->inputs[index];
                if (std::holds_alternative<juce::Image>(input))
                {
                    auto image = std::get<juce::Image>(input);
                    if (image.isValid())
                    {
                        if (juce::Direct2DPixelData::Ptr inputPixelData = dynamic_cast<juce::Direct2DPixelData*>(image.getPixelData()))
                            pimpl->d2dEffect->SetInput(index, inputPixelData->getAdapterD2D1Bitmap());
                    }
                }
                else if (std::holds_alternative<Effect::Ptr>(input))
                {
                    auto otherEffect = std::get<Effect::Ptr>(input);
                    pimpl->d2dEffect->SetInputEffect(index, otherEffect->pimpl->d2dEffect.get());

                    setInputsRecursive(otherEffect->pimpl.get());
                }
            }
        }

        Type effectType;
        juce::DxgiAdapter::Ptr adapter;
        winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
        winrt::com_ptr<ID2D1Effect> d2dEffect;
        std::vector<Effect::Input> inputs;

        static constexpr std::array<GUID const* const, (size_t)Type::numEffectTypes> effectGuids
        {
            &CLSID_D2D12DAffineTransform,
            &CLSID_D2D1AlphaMask,
            &CLSID_D2D1ArithmeticComposite,
            &CLSID_D2D1Blend,
            &CLSID_D2D1ChromaKey,
            &CLSID_D2D1Composite,
            &CLSID_D2D1Crop,
            &CLSID_D2D1EdgeDetection,
            &CLSID_D2D1Emboss,
            &CLSID_D2D1Flood,
            &CLSID_D2D1GaussianBlur,
            &CLSID_D2D1HighlightsShadows,
            &CLSID_D2D1Invert,
            &CLSID_D2D1LuminanceToAlpha,
            &CLSID_D2D13DPerspectiveTransform,
            &CLSID_D2D1Shadow,
            &CLSID_D2D1SpotDiffuse,
            &CLSID_D2D1SpotSpecular
        };
    };

    Effect::Effect(Type effectType_) :
        effectType(effectType_),
        pimpl(std::make_shared<Pimpl>(effectType_))
    {
        pimpl->createResources();
    }

    Effect::Effect(const Effect& other) :
        effectType(other.effectType),
        pimpl(other.pimpl)
    {
    }

    Effect::Effect(const Effect&& other) noexcept :
        effectType(other.effectType),
        pimpl(std::move(other.pimpl))
    {
    }

    Effect::~Effect()
    {
    }

    Effect::Ptr Effect::create(Type effectType)
    {
        return new Effect(effectType);
    }

    juce::String Effect::getName() const noexcept
    {
        pimpl->createResources();

        if (pimpl->d2dEffect)
        {
            wchar_t buffer[256];
            pimpl->d2dEffect->GetValue(D2D1_PROPERTY_DISPLAYNAME, (BYTE*)buffer, sizeof(buffer));
            return juce::String{ buffer };
        }

        return {};
    }

    void Effect::setInput(int index, juce::Image const& image)
    {
        pimpl->inputs[index] = image;
    }

    void Effect::addInput(juce::Image const& image)
    {
        for (auto& input : pimpl->inputs)
        {
            if (std::holds_alternative<std::monostate>(input))
            {
                input = image;
                return;
            }
        }
    }

    void Effect::setInput(int index, mescal::Effect::Ptr otherEffect)
    {
        pimpl->inputs[index] = otherEffect;
    }

    void Effect::addInput(mescal::Effect::Ptr otherEffect)
    {
        for (auto& input : pimpl->inputs)
        {
            if (std::holds_alternative<std::monostate>(input))
            {
                input = otherEffect;
                return;
            }
        }
    }

    const std::vector<mescal::Effect::Input>& Effect::getInputs() const noexcept
    {
        return pimpl->inputs;
    }

    int Effect::getNumProperties()
    {
        return pimpl->getNumProperties();
    }

    juce::String Effect::getPropertyName(int index)
    {
        return pimpl->getPropertyName(index);
    }

    void Effect::setPropertyValue(int index, const PropertyValue value)
    {
        pimpl->setProperty(index, value);

        if (onPropertyChange)
            onPropertyChange(this, index, value);
    }

    Effect::PropertyValue Effect::getPropertyValue(int index)
    {
        return pimpl->getProperty(index);
    }

    Effect::PropertyInfo Effect::getPropertyInfo(int index)
    {
        return pimpl->getPropertyInfo(index);
    }

    void Effect::applyEffect(juce::Image& outputImage, const juce::AffineTransform& transform, bool clearDestination)
    {
        pimpl->createResources();
        if (!pimpl->deviceContext || !pimpl->adapter || !pimpl->adapter->dxgiAdapter || !pimpl->d2dEffect)
        {
            return;
        }

        juce::Direct2DPixelData::Ptr outputPixelData = dynamic_cast<juce::Direct2DPixelData*>(outputImage.getPixelData());
        if (!outputPixelData)
        {
            return;
        }

        Pimpl::setInputsRecursive(pimpl.get());

        pimpl->deviceContext->SetTarget(outputPixelData->getAdapterD2D1Bitmap());
        pimpl->deviceContext->BeginDraw();
        if (clearDestination)
            pimpl->deviceContext->Clear();

        if (!transform.isIdentity())
            pimpl->deviceContext->SetTransform(juce::D2DUtilities::transformToMatrix(transform));

        pimpl->deviceContext->DrawImage(pimpl->d2dEffect.get());
        [[maybe_unused]] auto hr = pimpl->deviceContext->EndDraw();
        jassert(SUCCEEDED(hr));
    }

    juce::ReferenceCountedObjectPtr<Effect> Effect::affineTransform2D(juce::AffineTransform transform)
    {
        auto effect = create(Effect::Type::affineTransform2D);
        effect->setPropertyValue(AffineTransform2D::transformMatrix, transform);
        return effect;
    }

    Effect::Ptr Effect::createArithmeticComposite(float c0, float c1, float c2, float c3)
    {
        auto effect = create(Effect::Type::arithmeticComposite);
        effect->setPropertyValue(ArithmeticComposite::coefficients, Vector4{ c0, c1, c2, c3 });
        return effect;
    }

    Effect::GaussianBlur Effect::GaussianBlur::create(float standardDeviationValue)
    {
        auto effect = new Effect{ Effect::Type::gaussianBlur };
        effect->setPropertyValue(standardDeviation, standardDeviationValue);
        return effect;
    }

    Effect::SpotSpecularLighting Effect::SpotSpecularLighting::create()
    {
        return new Effect{ Effect::Type::spotSpecularLighting };
    }

    Effect::SpotSpecularLighting Effect::SpotSpecularLighting::withLightPosition(float x, float y, float z)
    {
        get()->setPropertyValue(lightPosition, Vector3{ x, y, z });
        return SpotSpecularLighting{ *this };
    }

    Effect::SpotSpecularLighting Effect::SpotSpecularLighting::withPointsAt(float x, float y, float z)
    {
        get()->setPropertyValue(pointsAt, Vector3{ x, y, z });
        return SpotSpecularLighting{ *this };
    }

    Effect::SpotSpecularLighting Effect::SpotSpecularLighting::withFocus(float focusValue)
    {
        get()->setPropertyValue(focus, focusValue);
        return SpotSpecularLighting{ *this };
    }

    Effect::SpotSpecularLighting Effect::SpotSpecularLighting::withLimitingConeAngle(float angle)
    {
        get()->setPropertyValue(limitingConeAngle, angle);
        return SpotSpecularLighting{ *this };
    }

    Effect::SpotSpecularLighting Effect::SpotSpecularLighting::withSpecularExponent(float exponent)
    {
        get()->setPropertyValue(specularExponent, exponent);
        return SpotSpecularLighting{ *this };
    }

    Effect::SpotSpecularLighting Effect::SpotSpecularLighting::withSpecularConstant(float constant)
    {
        get()->setPropertyValue(specularConstant, constant);
        return SpotSpecularLighting{ *this };
    }

    Effect::SpotSpecularLighting Effect::SpotSpecularLighting::withSurfaceScale(float scale)
    {
        get()->setPropertyValue(surfaceScale, scale);
        return SpotSpecularLighting{ *this };
    }

    Effect::SpotSpecularLighting Effect::SpotSpecularLighting::withColor(juce::Colour colour)
    {
        get()->setPropertyValue(color, colour);
        return SpotSpecularLighting{ *this };
    }

    Effect::SpotSpecularLighting Effect::SpotSpecularLighting::withKernelUnitLength(float x, float y)
    {
        get()->setPropertyValue(kernelUnitLength, Vector2{ x, y });
        return SpotSpecularLighting{ *this };
    }

    Effect::SpotSpecularLighting Effect::SpotSpecularLighting::withScaleMode(int mode)
    {
        get()->setPropertyValue(scaleMode, mode);
        return SpotSpecularLighting{ *this };
    }

}
