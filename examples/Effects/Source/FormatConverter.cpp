#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <d2d1_3helper.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#include <JuceHeader.h>
#include <juce_graphics/native/juce_DirectX_windows.h>
#include <juce_graphics/native/juce_Direct2DImage_windows.h>
#include "FormatConverter.h"

struct FormatConverter::Pimpl
{
    Pimpl(FormatConverter& owner_) : owner(owner_)
    {
    }

    void release()
    {
        destinationPixelData = nullptr;
        deviceContext = nullptr;
    }

    void createResources(juce::Image destinationImage)
    {
        if (!deviceContext)
        {
            if (destinationPixelData = dynamic_cast<juce::Direct2DPixelData*>(destinationImage.getPixelData()); destinationPixelData != nullptr)
            {
                if (auto adapter = destinationPixelData->getAdapter())
                {
                    winrt::com_ptr<ID2D1DeviceContext1> deviceContext1;
                    if (const auto hr = adapter->direct2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
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

    FormatConverter& owner;
    juce::Direct2DPixelData::Ptr destinationPixelData;
    winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
};

FormatConverter::FormatConverter() :
	pimpl(std::make_unique<Pimpl>(*this))
{
    {
        //juce::Graphics g{ argb };
        //g.setColour(juce::Colours::teal);
        //g.fillEllipse(argb.getBounds().toFloat());
    }

    {
        juce::Graphics g{ singleChannel };
        g.setColour(juce::Colours::white.withAlpha(0.75f));
        g.fillEllipse(singleChannel.getBounds().toFloat());
    }
}

FormatConverter::~FormatConverter()
{
}

void FormatConverter::convert(const juce::Image& source, juce::Image& destination)
{
    pimpl->release();
    pimpl->createResources(destination);

    if (pimpl->deviceContext)
    {
        if (auto sourcePixelData = dynamic_cast<juce::Direct2DPixelData*>(source.getPixelData()))
        {
            if (auto sourceBitmap = sourcePixelData->getAdapterD2D1Bitmap())
            {
#if 1
                winrt::com_ptr<ID2D1Effect> effect;
                pimpl->deviceContext->CreateEffect(CLSID_D2D1ColorMatrix, effect.put());

                effect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX,
                    D2D1::Matrix5x4F{
                        1, 0, 0, 0, 
                        0, 1, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1,
                        0, 0, 0, 0
                    });
                effect->SetInput(0, sourceBitmap);
        
                pimpl->deviceContext->SetTarget(pimpl->destinationPixelData->getAdapterD2D1Bitmap());
                pimpl->deviceContext->BeginDraw();
                pimpl->deviceContext->DrawImage(sourceBitmap);
                auto hr = pimpl->deviceContext->EndDraw();
                pimpl->deviceContext->SetTarget(nullptr);
#endif

#if 0
                winrt::com_ptr<ID2D1SolidColorBrush> brush;
                pimpl->deviceContext->CreateSolidColorBrush({ 1.0f, 1.0f, 1.0f, 1.0f }, brush.put());

                auto format = sourceBitmap->GetPixelFormat();
                format = pimpl->destinationPixelData->getAdapterD2D1Bitmap()->GetPixelFormat();
                
                pimpl->deviceContext->SetTarget(pimpl->destinationPixelData->getAdapterD2D1Bitmap());
                pimpl->deviceContext->BeginDraw();
                pimpl->deviceContext->Clear();
                
                ID2D1Bitmap* foo = sourceBitmap;
                ID2D1Brush* bar = brush.get();
                pimpl->deviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
                auto check = pimpl->deviceContext->GetAntialiasMode();
                pimpl->deviceContext->FillOpacityMask(foo,
                    bar, 
                    juce::D2DUtilities::toRECT_F(destination.getBounds().toFloat()), 
                    juce::D2DUtilities::toRECT_F(source.getBounds().toFloat()));
                auto hr = pimpl->deviceContext->EndDraw();
                pimpl->deviceContext->SetTarget(nullptr);
                
                DBG("hr " << (int)hr);
#endif
            }
        }
    }


    print(source);
    DBG("");
    print(destination);
}
