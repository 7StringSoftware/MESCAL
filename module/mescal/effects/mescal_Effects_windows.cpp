
namespace mescal
{

    struct Effect::Pimpl
    {
        Pimpl(EffectType effectType_) :
            effectType(effectType_)
        {
        }

        ~Pimpl()
        {
        }

        void createResources(juce::Image& image)
        {
            if (!adapter || !deviceContext)
            {
                if (auto pixelData = dynamic_cast<juce::Direct2DPixelData*>(image.getPixelData()))
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

            if (auto hr = deviceContext->CreateEffect(*effectGuids[(size_t)effectType], d2dEffect.put()); FAILED(hr))
            {
                jassertfalse;
            }
        }

        void configureEffect()
        {

        }

        EffectType effectType;
        juce::DxgiAdapter::Ptr adapter;
        winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
        winrt::com_ptr<ID2D1Effect> d2dEffect;
        juce::Direct2DPixelData::Ptr outputPixelData;

        static constexpr std::array<GUID const* const, (size_t)EffectType::numEffectTypes> effectGuids
        {
            &CLSID_D2D1GaussianBlur,
            &CLSID_D2D1SpotSpecular,
            &CLSID_D2D1SpotDiffuse
        };
    };

    std::unique_ptr<Effect> Effect::create(EffectType effectType)
    {
        return std::make_unique<Effect>(effectType);
    }

    Effect::Effect(EffectType effectType_) :
        effectType(effectType_),
        pimpl(std::make_unique<Pimpl>(effectType_))
    {
    }

    Effect::~Effect()
    {
    }

    void Effect::setProperty(PropertyIndex index, const PropertyValue& value)
    {
        switch (effectType)
        {
        case EffectType::gaussianBlur:
        {
            switch (std::get<GaussianBlurPropertyIndex>(index))
            {
            case GaussianBlurPropertyIndex::blurAmount:
                break;
            }
            break;
        }
        }
    }

    void Effect::getProperty(PropertyIndex index, PropertyValue& value)
    {

    }

    void Effect::applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha)
    {
        auto sourcePixelData = dynamic_cast<juce::Direct2DPixelData*>(sourceImage.getPixelData());
        if (!sourcePixelData)
        {
            return;
        }

        pimpl->createResources(sourceImage);
        if (!pimpl->deviceContext || !pimpl->adapter || !pimpl->adapter->dxgiAdapter || !pimpl->d2dEffect)
        {
            return;
        }

        auto& outputPixelData = pimpl->outputPixelData;
        if (!outputPixelData || outputPixelData->width < sourceImage.getWidth() || outputPixelData->height < sourceImage.getHeight())
        {
            outputPixelData = juce::Direct2DPixelData::make(juce::Image::ARGB, sourceImage.getWidth(), sourceImage.getHeight(), true, pimpl->adapter);
        }

        pimpl->configureEffect();

        pimpl->d2dEffect->SetInput(0, sourcePixelData->getAdapterD2D1Bitmap());
        pimpl->deviceContext->SetTarget(outputPixelData->getAdapterD2D1Bitmap());
        pimpl->deviceContext->BeginDraw();
        pimpl->deviceContext->DrawImage(pimpl->d2dEffect.get());
        pimpl->deviceContext->EndDraw();

        auto outputImage = juce::Image{ pimpl->outputPixelData }.getClippedImage(sourceImage.getBounds());
        destContext.drawImageAt(outputImage, 0, 0);
    }

    void Effect::applyEffect(juce::Image& sourceImage, juce::Image& outputImage, float scaleFactor, float alpha)
    {
        auto sourcePixelData = dynamic_cast<juce::Direct2DPixelData*>(sourceImage.getPixelData());
        if (!sourcePixelData)
        {
            return;
        }

        pimpl->createResources(sourceImage);
        if (!pimpl->deviceContext || !pimpl->adapter || !pimpl->adapter->dxgiAdapter || !pimpl->d2dEffect)
        {
            return;
        }

        auto& outputPixelData = pimpl->outputPixelData;
        if (!outputPixelData || outputPixelData->width < sourceImage.getWidth() || outputPixelData->height < sourceImage.getHeight())
        {
            outputPixelData = juce::Direct2DPixelData::make(juce::Image::ARGB, sourceImage.getWidth(), sourceImage.getHeight(), true, pimpl->adapter);
        }

        pimpl->configureEffect();

        pimpl->d2dEffect->SetInput(0, sourcePixelData->getAdapterD2D1Bitmap());

        pimpl->deviceContext->SetTarget(outputPixelData->getAdapterD2D1Bitmap());
        pimpl->deviceContext->BeginDraw();
        pimpl->deviceContext->DrawImage(pimpl->d2dEffect.get());
        pimpl->deviceContext->EndDraw();
    }

}
