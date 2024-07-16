
namespace mescal
{
    struct Effect::Pimpl
    {
        Pimpl(Type effectType_) :
            effectType(effectType_)
        {
#if 0
            switch (effectType)
            {
            case Effect::Type::spotSpecularLighting:
            {
                propertyValues.emplace_back(Point3D{ 0.0f, 0.0f, 100.0f }); // lightPosition
                propertyValues.emplace_back(Point3D{ 0.0f, 0.0f, 0.0f }); // focusPointPosition
                propertyValues.emplace_back(1.0f); // focus
                propertyValues.emplace_back(90.0f); // limitingConeAngle
                propertyValues.emplace_back(1.0f); // specularExponent
                propertyValues.emplace_back(1.0f); // specularConstant
                propertyValues.emplace_back(1.0f); // surfaceScale
                propertyValues.emplace_back(RGBColor{ 1.0f, 1.0f, 1.0f }); // lightColor
                break;
            }
            }
#endif
        }

        ~Pimpl()
        {
        }

        void createResources(juce::Image& sourceImage, juce::Image& destinationImage)
        {
            if (!adapter || !deviceContext)
            {
                if (auto pixelData = dynamic_cast<juce::Direct2DPixelData*>(sourceImage.getPixelData()))
                {
                    if (adapter = pixelData->getAdapter())
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
                }
            }

            createEffect();

            outputPixelData = dynamic_cast<juce::Direct2DPixelData*>(destinationImage.getPixelData());

            jassert(outputPixelData && deviceContext);
        }

        void createEffect()
        {
            if (!d2dEffect)
            {
                if (auto hr = deviceContext->CreateEffect(*effectGuids[(size_t)effectType], d2dEffect.put()); FAILED(hr))
                {
                    jassertfalse;
                }
            }
        }

        void setProperty(int index, const PropertyValue& value)
        {
            if (!d2dEffect)
                return;

            propertyValues[index] = value;

            switch (value.index())
            {
            case 0: // float
            {
                DBG("setProperty " << index << " " << std::get<float>(value));
                d2dEffect->SetValue(index, std::get<float>(value));
                break;
            }

            case 1: // juce::Point<float>
            {
                auto point = std::get<juce::Point<float>>(value);
                D2D1_VECTOR_2F v2{ point.x, point.y };
                d2dEffect->SetValue(index, v2);
                break;
            }

            case 2:
            {
                auto color = std::get<juce::Colour>(value);
                D2D1_VECTOR_4F v{ color.getFloatRed(), color.getFloatGreen(), color.getFloatBlue(), color.getFloatAlpha() };
                d2dEffect->SetValue(index, v);
                break;
            }

            case 3: // RGBColor
            {
                auto rgbColor = std::get<RGBColor>(value);
                d2dEffect->SetValue(index, D2D1_VECTOR_3F{ rgbColor.r, rgbColor.g, rgbColor.b });

                DBG("setProperty " << index << " " << rgbColor.r << "," << rgbColor.g << "," << rgbColor.b);
                break;
            }

            case 4: // Point3D
            {
                auto point3D = std::get<Point3D>(value);
                D2D1_VECTOR_3F v3{ point3D.x, point3D.y, point3D.z };
                d2dEffect->SetValue(index, v3);

                DBG("setProperty " << index << " " << point3D.x << "," << point3D.y << "," << point3D.z);
                break;
            }

            }
        }

        Type effectType;
        juce::DxgiAdapter::Ptr adapter;
        winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
        winrt::com_ptr<ID2D1Effect> d2dEffect;
        juce::Direct2DPixelData::Ptr outputPixelData;

        static constexpr std::array<GUID const* const, (size_t)Type::numEffectTypes> effectGuids
        {
            &CLSID_D2D1GaussianBlur,
            &CLSID_D2D1SpotSpecular,
            &CLSID_D2D1SpotDiffuse,
            &CLSID_D2D1Shadow,
            &CLSID_D2D13DPerspectiveTransform
        };

        std::map<uint32_t, PropertyValue> propertyValues;
    };


    Effect::Effect(Type effectType_) :
        effectType(effectType_),
        pimpl(std::make_unique<Pimpl>(effectType_))
    {
    }

    Effect::Effect(const Effect& other) :
        effectType(other.effectType),
        pimpl(std::make_unique<Pimpl>(other.effectType))
    {
    }

    Effect::~Effect()
    {
    }

    size_t Effect::getNumProperties() const noexcept
    {
        return pimpl->propertyValues.size();
    }

    void Effect::setProperty(int index, const PropertyValue& value)
    {
        pimpl->setProperty(index, value);
    }

    const Effect::PropertyValue& Effect::getProperty(int index)
    {
        return pimpl->propertyValues[index];
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

        pimpl->createResources(sourceImage, outputImage);
        if (!pimpl->deviceContext || !pimpl->adapter || !pimpl->adapter->dxgiAdapter || !pimpl->d2dEffect)
        {
            return;
        }

        auto& outputPixelData = pimpl->outputPixelData;
        if (!outputPixelData || outputPixelData->width < sourceImage.getWidth() || outputPixelData->height < sourceImage.getHeight())
        {
            outputPixelData = juce::Direct2DPixelData::make(juce::Image::ARGB, sourceImage.getWidth(), sourceImage.getHeight(), true, pimpl->adapter);
        }

        pimpl->d2dEffect->SetInput(0, sourcePixelData->getAdapterD2D1Bitmap());

        for (auto const& value : pimpl->propertyValues)
        {
            pimpl->setProperty(value.first, value.second);
        }

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
        pimpl->createResources(sourceImage, outputImage);
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

            for (auto const& value : effect.pimpl->propertyValues)
            {
                effect.pimpl->setProperty(value.first, value.second);
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

