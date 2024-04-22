#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#include <JuceHeader.h>
#include <juce_graphics/native/juce_DirectX_windows.h>
#include <juce_graphics/native/juce_Direct2DImage_windows.h>
#include <JuceHeader.h>
#include "Direct2DEffect.h"
#include "Direct2DEdgeDetectionEffect.h"
#include "Direct2DEmbossEffect.h"

struct Direct2DEffect::Pimpl
{
    Pimpl(GUID const effectGuid_) : effectGuid(effectGuid_)
    {
    }

    virtual ~Pimpl()
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

        if (auto hr = deviceContext->CreateEffect(effectGuid, d2dEffect.put()); FAILED(hr))
        {
            jassertfalse;
        }
    }

    virtual void configureEffect() = 0;

    GUID const effectGuid;
    juce::DxgiAdapter::Ptr adapter;
    winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
    winrt::com_ptr<ID2D1Effect> d2dEffect;
    juce::Direct2DPixelData::Ptr outputPixelData;
};

Direct2DEffect::Direct2DEffect()
{
}

Direct2DEffect::~Direct2DEffect()
{
}

void Direct2DEffect::applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha)
{
    auto sourcePixelData = dynamic_cast<juce::Direct2DPixelData*>(sourceImage.getPixelData());
    if (!sourcePixelData)
    {
        return;
    }

    auto pimpl = getPimpl();
    pimpl->createResources(sourceImage);
    if (!pimpl->deviceContext || !pimpl->adapter || !pimpl->adapter->dxgiAdapter)
    {
        return;
    }

    auto& outputPixelData = pimpl->outputPixelData;
    if (!outputPixelData || outputPixelData->width < sourceImage.getWidth() || outputPixelData->height < sourceImage.getHeight())
    {
        outputPixelData = juce::Direct2DPixelData::make(juce::Image::ARGB, sourceImage.getWidth(), sourceImage.getHeight(), true, pimpl->adapter);
    }

    if (auto hr = pimpl->deviceContext->CreateEffect(CLSID_D2D1Emboss, pimpl->d2dEffect.put()); FAILED(hr))
    {
        jassertfalse;
        return;
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

#include "Direct2DEdgeDetectionEffect.cpp"
#include "Direct2DEmbossEffect.cpp"