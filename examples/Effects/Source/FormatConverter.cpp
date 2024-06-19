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
        sourcePixelData = nullptr;
        destinationPixelData = nullptr;
        deviceContext = nullptr;
    }

    void createResources(const juce::Image& sourceImage, juce::Image destinationImage)
    {
        sourcePixelData = dynamic_cast<juce::Direct2DPixelData*>(sourceImage.getPixelData());
        destinationPixelData = dynamic_cast<juce::Direct2DPixelData*>(destinationImage.getPixelData());

        if (!deviceContext && sourcePixelData && destinationPixelData)
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

    FormatConverter& owner;
    winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
    juce::Direct2DPixelData::Ptr sourcePixelData, destinationPixelData;
};

FormatConverter::FormatConverter() :
    pimpl(std::make_unique<Pimpl>(*this))
{
}

FormatConverter::~FormatConverter()
{
}

#if 0
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
#if 0
                winrt::com_ptr<ID2D1Effect> colorMatrixEffect, premultiplyEffect;

                pimpl->deviceContext->CreateEffect(CLSID_D2D1ColorMatrix, colorMatrixEffect.put());
                pimpl->deviceContext->CreateEffect(CLSID_D2D1Premultiply, premultiplyEffect.put());

                colorMatrixEffect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX,
                    D2D1::Matrix5x4F
                    {
                        1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        1.0f, 1.0f, 1.0f, 1.0f,
                        0.0f, 0.0f, 0.0f, 0.0f
                    });
                colorMatrixEffect->SetValue(D2D1_COLORMATRIX_PROP_ALPHA_MODE, D2D1_COLORMATRIX_ALPHA_MODE_STRAIGHT);

                colorMatrixEffect->SetInput(0, sourceBitmap);
                premultiplyEffect->SetInputEffect(0, colorMatrixEffect.get());

                pimpl->deviceContext->SetTarget(pimpl->destinationPixelData->getAdapterD2D1Bitmap());
                pimpl->deviceContext->BeginDraw();
                pimpl->deviceContext->Clear();
                pimpl->deviceContext->DrawImage(premultiplyEffect.get());
                pimpl->deviceContext->EndDraw();
                pimpl->deviceContext->SetTarget(nullptr);
#endif
#if 0
                winrt::com_ptr<ID2D1Effect> effect;
                if (const auto hr = pimpl->deviceContext->CreateEffect(CLSID_D2D1Saturation, effect.put());
                    SUCCEEDED(hr))
                {
                    effect->SetInput(0, sourceBitmap);
                    effect->SetValue(D2D1_SATURATION_PROP_SATURATION, 0.0f);

                    pimpl->deviceContext->SetTarget(pimpl->destinationPixelData->getAdapterD2D1Bitmap());
                    pimpl->deviceContext->BeginDraw();
                    pimpl->deviceContext->Clear();

                    pimpl->deviceContext->DrawImage(effect.get());
                    pimpl->deviceContext->EndDraw();
                    pimpl->deviceContext->SetTarget(nullptr);
                }
#endif
#if 0
                winrt::com_ptr<ID2D1Effect> effect;
                if (const auto hr = pimpl->deviceContext->CreateEffect(CLSID_D2D1Tint, effect.put());
                    SUCCEEDED(hr))
                {
                    effect->SetInput(0, sourceBitmap);
                    effect->SetValue(D2D1_TINT_PROP_COLOR, D2D1::Vector4F(1.0f, 0.0f, 0.0f, 1.0f));
                    effect->SetValue(D2D1_TINT_PROP_FORCE_DWORD, TRUE);

                    pimpl->deviceContext->SetTarget(pimpl->destinationPixelData->getAdapterD2D1Bitmap());
                    pimpl->deviceContext->BeginDraw();
                    pimpl->deviceContext->Clear();

                    pimpl->deviceContext->DrawImage(effect.get());
                    pimpl->deviceContext->EndDraw();
                    pimpl->deviceContext->SetTarget(nullptr);
                }
#endif

#if 1
                winrt::com_ptr<ID2D1SolidColorBrush> colorBrush;
                pimpl->deviceContext->CreateSolidColorBrush({ 1.0f, 1.0f, 1.0f, 1.0f }, colorBrush.put());

                auto format = sourceBitmap->GetPixelFormat();
                format = pimpl->destinationPixelData->getAdapterD2D1Bitmap()->GetPixelFormat();

                auto scratchpadBitmap = dynamic_cast<juce::Direct2DPixelData*>(scratchpad.getPixelData())->getAdapterD2D1Bitmap();
                pimpl->deviceContext->SetTarget(scratchpadBitmap);
                pimpl->deviceContext->BeginDraw();

                pimpl->deviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
                auto check = pimpl->deviceContext->GetAntialiasMode();
                pimpl->deviceContext->FillOpacityMask(sourceBitmap,
                    colorBrush.get(),
                    juce::D2DUtilities::toRECT_F(destination.getBounds().toFloat()),
                    juce::D2DUtilities::toRECT_F(source.getBounds().toFloat()));
                auto hr = pimpl->deviceContext->EndDraw();
                pimpl->deviceContext->SetTarget(nullptr);
#if 0

                winrt::com_ptr<ID2D1Effect> effect;
                pimpl->deviceContext->CreateEffect(CLSID_D2D1Premultiply, effect.put());

                effect->SetInput(0, scratchpadBitmap);
                pimpl->deviceContext->SetTarget(pimpl->destinationPixelData->getAdapterD2D1Bitmap());
                pimpl->deviceContext->BeginDraw();
                pimpl->deviceContext->DrawImage(effect.get());
                pimpl->deviceContext->EndDraw();
                pimpl->deviceContext->SetTarget(nullptr);
#endif


                DBG("hr " << (int)hr);
#endif
            }
        }
    }


    print(source);
    DBG("");
    print(destination);
}
#endif

juce::Image FormatConverter::convert(const juce::Image& source, juce::Image::PixelFormat outputFormat)
{
    switch (source.getFormat())
    {
    case juce::Image::SingleChannel:
    {
        switch (outputFormat)
        {
        case juce::Image::SingleChannel:
        {
            return source.createCopy();
        }

        case juce::Image::RGB:
        case juce::Image::ARGB:
        {

            return {};
        }
        }

        break;
    }

    case juce::Image::RGB:
    case juce::Image::ARGB:
    {
        switch (outputFormat)
        {
        case juce::Image::SingleChannel:
        {
            return convertToSingleChannel(source);
        }

        case juce::Image::RGB:
        case juce::Image::ARGB:
        {

            return source.createCopy();
        }
        }

        break;
    }
    }

    return {};
}

juce::Image FormatConverter::convertToSingleChannel(const juce::Image& source)
{
    juce::Image destination{ juce::Image::SingleChannel, source.getWidth(), source.getHeight(), true };

    pimpl->createResources(source, destination);

    auto sourceBitmap = pimpl->sourcePixelData->getAdapterD2D1Bitmap();
    auto destinationBitmap = pimpl->destinationPixelData->getAdapterD2D1Bitmap();
    if (sourceBitmap && destinationBitmap)
    {
        auto& deviceContext = pimpl->deviceContext;

        deviceContext->SetTarget(destinationBitmap);
        deviceContext->BeginDraw();
        deviceContext->DrawImage(sourceBitmap);
        deviceContext->EndDraw();
        deviceContext->SetTarget(nullptr);

        pimpl->release();

        return destination;
    }

    return {};
}
