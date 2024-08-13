
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
                uint32_t minNumInputs, maxNumInputs;

                auto hr = d2dEffect->GetValue(D2D1_PROPERTY_MIN_INPUTS, &minNumInputs);
                hr = d2dEffect->GetValue(D2D1_PROPERTY_MAX_INPUTS, &maxNumInputs);
            }
        }

        void setProperty(int index, const PropertyValue& value)
        {
            [[maybe_unused]] HRESULT hr = S_OK;

            if (!d2dEffect)
                createResources();

            if (!d2dEffect)
                return;

            if (std::holds_alternative<int>(value))
            {
                hr = d2dEffect->SetValue(index, std::get<int>(value));
                DBG("setProperty " << index << " (int) " << std::get<int>(value));
            }
            else if (std::holds_alternative<float>(value))
            {
                d2dEffect->SetValue(index, std::get<float>(value));
                DBG("setProperty " << index << " (float) " << std::get<float>(value));
            }
            else if (std::holds_alternative<Vector2>(value))
            {
                auto vector2 = std::get<Vector2>(value);
                d2dEffect->SetValue(index, D2D1_VECTOR_2F{ vector2[0], vector2[1] });
                DBG("setProperty " << index << " Vector2 " << vector2[0] << ", " << vector2[1]);
            }
            else if (std::holds_alternative<Vector3>(value))
            {
                auto vector3 = std::get<Vector3>(value);
                d2dEffect->SetValue(index, D2D1_VECTOR_3F{ vector3[0], vector3[1], vector3[2] });
                DBG("setProperty " << index << " Vector3 " << vector3[0] << ", " << vector3[1] << ", " << vector3[2]);
            }
            else if (std::holds_alternative<Vector4>(value))
            {
                auto vector4 = std::get<Vector4>(value);
                d2dEffect->SetValue(index, D2D1_VECTOR_4F{ vector4[0], vector4[1], vector4[2], vector4[3]});
                DBG("setProperty " << index << " vector4 " << vector4[0] << ", " << vector4[1] << ", " << vector4[2] << ", " << vector4[3]);
            }
            else
            {
                jassertfalse;
            }

            jassert(SUCCEEDED(hr));
        }

        Type effectType;
        juce::DxgiAdapter::Ptr adapter;
        winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
        winrt::com_ptr<ID2D1Effect> d2dEffect;

        static constexpr std::array<GUID const* const, (size_t)Type::numEffectTypes> effectGuids
        {
            &CLSID_D2D1GaussianBlur,
            &CLSID_D2D1SpotSpecular,
            &CLSID_D2D1Shadow,
            &CLSID_D2D1SpotDiffuse,
            &CLSID_D2D13DPerspectiveTransform,
            &CLSID_D2D1Blend
        };

        std::vector <Effect::Property> properties;
    };

    Effect::Effect(Type effectType_) :
        effectType(effectType_),
        pimpl(std::make_unique<Pimpl>(effectType_))
    {
        pimpl->createResources();

        initProperties();

        getName();
    }

    Effect::Effect(const Effect& other) :
        effectType(other.effectType),
        pimpl(std::make_unique<Pimpl>(other.effectType))
    {
        pimpl->createResources();

        initProperties();

        getName();
    }

    Effect::~Effect()
    {
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
        struct ImageInput : public Input
        {
            ImageInput(juce::Image const& image_) :
                image(juce::NativeImageType{}.convert(image_))
            {
            }

            ~ImageInput() override = default;

            void setEffectInput(int index, Pimpl* pimpl) override
            {
                auto sourcePixelData = dynamic_cast<juce::Direct2DPixelData*>(image.getPixelData());
                if (!sourcePixelData)
                {
                    return;
                }

                pimpl->d2dEffect->SetInput(index, sourcePixelData->getAdapterD2D1Bitmap());
            }

            juce::Image const image;
        };

        inputs.resize(index + 1);
        inputs[index] = std::make_unique<ImageInput>(image);
    }

    void Effect::setInput(int index, mescal::Effect const* const otherEffect)
    {
        struct ChainedEffectInput : public Input
        {
            ChainedEffectInput(Effect const* const otherEffect_) : otherEffect(otherEffect_) {}

            void setEffectInput(int index, Pimpl* pimpl) override
            {
                pimpl->d2dEffect->SetInputEffect(index, otherEffect->pimpl->d2dEffect.get());
            }

            Effect const* const otherEffect;
        };

        inputs.resize(index + 1);
        inputs[index] = std::make_unique<ChainedEffectInput>(otherEffect);
    }

    const std::vector<Effect::Property>& Effect::getProperties() const noexcept
    {
        return pimpl->properties;
    }

    void Effect::setPropertyValue(int index, const PropertyValue& value)
    {
        pimpl->setProperty(index, value);
    }

    void Effect::applyEffect(juce::Image& outputImage, float scaleFactor, float alpha, bool clearDestination)
    {
        pimpl->createResources();
        if (!pimpl->deviceContext || !pimpl->adapter || !pimpl->adapter->dxgiAdapter || !pimpl->d2dEffect)
        {
            return;
        }

        uint32_t sourceImageIndex = 0;
        int width = 0, height = 0;
        for (int index = 0; index < inputs.size(); ++index)
        {
            inputs[index]->setEffectInput(sourceImageIndex++, pimpl.get());
        }

        juce::Direct2DPixelData::Ptr outputPixelData = dynamic_cast<juce::Direct2DPixelData*>(outputImage.getPixelData());
        if (!outputPixelData)
        {
            return;
        }

        pimpl->deviceContext->SetTarget(outputPixelData->getAdapterD2D1Bitmap());
        pimpl->deviceContext->BeginDraw();
        if (clearDestination)
            pimpl->deviceContext->Clear();
        pimpl->deviceContext->DrawImage(pimpl->d2dEffect.get());
        pimpl->deviceContext->EndDraw();
    }

#if 0
    void Effect::applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha)
    {
        juce::Image destinationImage{ juce::Image::ARGB, sourceImage.getWidth(), sourceImage.getHeight(), true };
        applyEffect(sourceImage, destinationImage, scaleFactor, alpha, false);
        destContext.drawImageAt(destinationImage, 0, 0);
    }

    void Effect::applyEffect(juce::Image& sourceImage, juce::Image& outputImage, float /*scaleFactor*/, float /*alpha*/, bool clearDestination)
    {
        auto sourcePixelData = dynamic_cast<juce::Direct2DPixelData*>(sourceImage.getPixelData());
        if (!sourcePixelData)
        {
            return;
        }

        pimpl->createResources();
        if (!pimpl->deviceContext || !pimpl->adapter || !pimpl->adapter->dxgiAdapter || !pimpl->d2dEffect)
        {
            return;
        }

        juce::Direct2DPixelData::Ptr outputPixelData = dynamic_cast<juce::Direct2DPixelData*>(outputImage.getPixelData());
        if (!outputPixelData || outputPixelData->width < sourceImage.getWidth() || outputPixelData->height < sourceImage.getHeight())
        {
            outputPixelData = juce::Direct2DPixelData::make(juce::Image::ARGB, sourceImage.getWidth(), sourceImage.getHeight(), true, pimpl->adapter);
        }

        pimpl->d2dEffect->SetInput(0, sourcePixelData->getAdapterD2D1Bitmap());

        pimpl->deviceContext->SetTarget(outputPixelData->getAdapterD2D1Bitmap());
        pimpl->deviceContext->BeginDraw();
        if (clearDestination)
            pimpl->deviceContext->Clear();
        pimpl->deviceContext->DrawImage(pimpl->d2dEffect.get());
        pimpl->deviceContext->EndDraw();
    }
#endif

}

