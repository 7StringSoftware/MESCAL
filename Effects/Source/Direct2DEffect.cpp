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
    Pimpl()
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
    }

    juce::DxgiAdapter::Ptr adapter;
    winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
    winrt::com_ptr<ID2D1Effect> d2dEffect;
    juce::Direct2DPixelData::Ptr outputPixelData;
};

Direct2DEffect::Direct2DEffect() :
    pimpl(std::make_unique<Pimpl>())
{
}

Direct2DEffect::~Direct2DEffect()
{
}

#include "Direct2DEdgeDetectionEffect.cpp"
#include "Direct2DEmbossEffect.cpp"
