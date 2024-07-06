
namespace mescal
{
    DEFINE_GUID(CLSID_SampleEffect, 0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    DEFINE_GUID(CLSID_CustomRippleEffect, 0xB7B36C92, 0x3498, 0x4A94, 0x9E, 0x95, 0x9F, 0x24, 0x6F, 0x92, 0x45, 0xBF);


#define XML(X) TEXT(#X) // This macro creates a single string from multiple lines of text.

#if 0
    PCWSTR pszXml =
        XML(
            < ? xml version = '1.0' ? >
            <Effect>
            <!--System Properties-->
            <Property name = 'DisplayName' type = 'string' value = 'SampleEffect' / >
            <Property name = 'Author' type = 'string' value = 'Contoso' / >
            <Property name = 'Category' type = 'string' value = 'Sample' / >
            <Property name = 'Description' type = 'string' value = 'This is a demo effect.' / >
            <Inputs>
            <Input name = 'SourceOne' / >
            <!-- <Input name = 'SourceTwo' / > -->
            <!--Additional inputs go here. -->
            < / Inputs>
            <!--Custom Properties go here. -->
            < / Effect>
        );
#endif

    struct CustomEffect::Pimpl : public juce::ComBaseClassHelper<ID2D1EffectImpl>
    {
        Pimpl()
        {
        }

        ~Pimpl()
        {
            d2dEffect = nullptr;
            deviceContext = nullptr;
            adapter = nullptr;
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

                        deviceContext1->CreateEffect(CLSID_CustomRippleEffect, d2dEffect.put());
                    }
                }
            }
        }

        void configureEffect()
        {

        }

        STDOVERRIDEMETHODIMP Initialize(_In_ ID2D1EffectContext* effectContext, _In_ ID2D1TransformGraph* transformGraph)
        {
            return S_OK;
        }

        STDOVERRIDEMETHODIMP PrepareForRender(D2D1_CHANGE_TYPE changeType)
        {
            return S_OK;
        }

        STDOVERRIDEMETHODIMP SetGraph(_In_ ID2D1TransformGraph* transformGraph)
        {
            return S_OK;
        }

        static HRESULT createEffect(IUnknown** effectImpl)
        {
            return S_OK;
        }

        juce::DxgiAdapter::Ptr adapter;
        winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
        juce::Direct2DPixelData::Ptr outputPixelData;
        winrt::com_ptr<ID2D1Effect> d2dEffect;
    };

    CustomEffect::CustomEffect() :
        pimpl(std::make_unique<Pimpl>())
    {
    }

    CustomEffect::~CustomEffect()
    {
    }

    void CustomEffect::applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha)
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

    void CustomEffect::applyEffect(juce::Image& sourceImage, juce::Image& outputImage, float scaleFactor, float alpha)
    {
#if 0
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
#endif
    }

}
