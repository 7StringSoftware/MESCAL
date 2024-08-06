
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
        }

        void setProperty(int index, const PropertyValue& value)
        {
            if (!d2dEffect)
                createResources();

            if (!d2dEffect || properties.size() < index)
                return;

            if (std::holds_alternative<int>(value))
            {
                d2dEffect->SetValue(index, std::get<int>(value));
                DBG("setProperty " << index << " (int) " << std::get<int>(value));
            }
            else if (std::holds_alternative<float>(value))
            {
                d2dEffect->SetValue(index, std::get<float>(value));
                DBG("setProperty " << index << " (float) " << std::get<float>(value));
            }
            else if (std::holds_alternative<juce::Point<float>>(value))
            {
                auto point = std::get<juce::Point<float>>(value);
                d2dEffect->SetValue(index, D2D1_VECTOR_2F{ point.x, point.y });
                DBG("setProperty " << index << " Point<float> " << point.toString());
            }
            else if (std::holds_alternative<juce::Colour>(value))
            {
                auto color = std::get<juce::Colour>(value);
                d2dEffect->SetValue(index, D2D1_VECTOR_4F{ color.getFloatRed(), color.getFloatGreen(), color.getFloatBlue(), color.getFloatAlpha() });
                DBG("setProperty " << index << " Colour " << color.toString());
            }
            else if (std::holds_alternative<RGBColor>(value))
            {
                auto rgbColor = std::get<RGBColor>(value);
                d2dEffect->SetValue(index, D2D1_VECTOR_3F{ rgbColor.r, rgbColor.g, rgbColor.b });
                DBG("setProperty " << index << " RGBColor " << rgbColor.r << " " << rgbColor.g << " " << rgbColor.b);
            }
            else if (std::holds_alternative<Point3D>(value))
            {
                auto point3D = std::get<Point3D>(value);
                d2dEffect->SetValue(index, D2D1_VECTOR_3F{ point3D.x, point3D.y, point3D.z });
                DBG("setProperty " << index << " Point3D " << point3D.x << "',  " << point3D.y << " " << point3D.z);

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
            else
            {
                jassertfalse;
            }
        }

        Type effectType;
        juce::DxgiAdapter::Ptr adapter;
        winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
        winrt::com_ptr<ID2D1Effect> d2dEffect;

        static constexpr std::array<GUID const* const, (size_t)Type::numEffectTypes> effectGuids
        {
            &CLSID_D2D1GaussianBlur,
            &CLSID_D2D1SpotSpecular,
            &CLSID_D2D1SpotDiffuse,
            &CLSID_D2D1Shadow,
            &CLSID_D2D13DPerspectiveTransform
        };

        std::vector <Effect::Property> properties;
    };


    Effect::Effect(Type effectType_) :
        effectType(effectType_),
        pimpl(std::make_unique<Pimpl>(effectType_))
    {
        pimpl->createResources();

        initProperties();
    }

    Effect::Effect(const Effect& other) :
        effectType(other.effectType),
        pimpl(std::make_unique<Pimpl>(other.effectType))
    {
        pimpl->createResources();

        initProperties();
    }

    Effect::~Effect()
    {
    }

    juce::String Effect::getName() const noexcept
    {
        return "Effect";
    }

    const std::vector<Effect::Property>& Effect::getProperties() const noexcept
    {
        return pimpl->properties;
    }

    void Effect::setPropertyValue(int index, const PropertyValue& value)
    {
        pimpl->setProperty(index, value);
    }

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

    void EffectChain::addEffect(Effect::Type effectType)
    {
        effects.emplace_back(Effect{ effectType });
    }

    void EffectChain::applyEffects(juce::Image& sourceImage, juce::Image& outputImage, float scaleFactor, float alpha, bool clearDestination)
    {
        auto sourcePixelData = dynamic_cast<juce::Direct2DPixelData*>(sourceImage.getPixelData());
        if (!sourcePixelData)
        {
            return;
        }

        auto& firstEffect = effects.front();
        auto& pimpl = firstEffect.pimpl;
        pimpl->createResources();
        if (!pimpl->deviceContext || !pimpl->adapter || !pimpl->adapter->dxgiAdapter || !pimpl->d2dEffect)
        {
            return;
        }

        pimpl->d2dEffect->SetInput(0, sourcePixelData->getAdapterD2D1Bitmap());
        auto previousEffect = pimpl->d2dEffect;
        for (auto it = effects.begin() + 1; it != effects.end(); ++it)
        {
            auto& effect = *it;

            if (!effect.pimpl->d2dEffect)
            {
                if (auto hr = pimpl->deviceContext->CreateEffect(*pimpl->effectGuids[(size_t)effect.pimpl->effectType], effect.pimpl->d2dEffect.put()); FAILED(hr))
                {
                    jassertfalse;
                }
            }

            effect.pimpl->d2dEffect->SetInputEffect(0, previousEffect.get());

            previousEffect = effect.pimpl->d2dEffect;
        }

        auto outputPixelData = dynamic_cast<juce::Direct2DPixelData*>(outputImage.getPixelData());
        pimpl->deviceContext->SetTarget(outputPixelData->getAdapterD2D1Bitmap());
        pimpl->deviceContext->BeginDraw();
        if (clearDestination)
            pimpl->deviceContext->Clear();
        pimpl->deviceContext->DrawImage(effects.back().pimpl->d2dEffect.get());
        pimpl->deviceContext->EndDraw();
    }
}

