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

        if (destinationImage.isValid())
            destinationPixelData = dynamic_cast<juce::Direct2DPixelData*>(destinationImage.getPixelData());

        if (!deviceContext && sourcePixelData)
        {
            if (auto adapter = sourcePixelData->getAdapter())
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

            return singleChannelToARGB(source);
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

juce::Image FormatConverter::singleChannelToARGB(const juce::Image& source)
{
    //juce::Image temp{ juce::Image::ARGB, source.getWidth(), source.getHeight(), true };
    juce::Image destination{ juce::Image::ARGB, source.getWidth(), source.getHeight(), true };


    //auto tempBitmap = dynamic_cast<juce::Direct2DPixelData*>(temp.getPixelData())->getAdapterD2D1Bitmap();

    pimpl->createResources(source, destination);

    auto sourceBitmap = pimpl->sourcePixelData->getAdapterD2D1Bitmap();
    auto destinationBitmap = pimpl->destinationPixelData->getAdapterD2D1Bitmap();

    winrt::com_ptr<ID2D1Bitmap1> tempBitmap;
    if (const auto hr = pimpl->deviceContext->CreateBitmap(D2D1_SIZE_U{ (uint32_t)source.getWidth(), (uint32_t)source.getHeight() },
        nullptr, 0, D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_STRAIGHT)),
        tempBitmap.put());
        FAILED(hr))
    {
        jassertfalse;
        return {};
    }

    if (sourceBitmap && destinationBitmap)
    {
        auto& deviceContext = pimpl->deviceContext;

#if 0
        winrt::com_ptr<ID2D1BitmapBrush1> brush;
        if (const auto hr = deviceContext->CreateBitmapBrush(sourceBitmap, brush.put());
            FAILED(hr))
        {
            jassertfalse;
            return {};
        }

        deviceContext->SetTarget(destinationBitmap);
        deviceContext->BeginDraw();
        deviceContext->Clear();// { 1.0f, 1.0f, 1.0f, 1.0f });
        deviceContext->FillRectangle(juce::D2DUtilities::toRECT_F(destination.getBounds().toFloat()), brush.get());
        auto hr = deviceContext->EndDraw();
        deviceContext->SetTarget(nullptr);
#endif

#if 0
        winrt::com_ptr<ID2D1Effect> effect;
        deviceContext->CreateEffect(CLSID_D2D1Grayscale, effect.put());
        effect->SetInput(0, sourceBitmap);

        deviceContext->SetTarget(destinationBitmap);
        deviceContext->BeginDraw();
        deviceContext->Clear();
        deviceContext->DrawImage(effect.get());
        auto hr = deviceContext->EndDraw();
        deviceContext->SetTarget(nullptr);
#endif

#if 0
        deviceContext->SetTarget(tempBitmap);
        deviceContext->BeginDraw();
        deviceContext->Clear({ 1.0f, 1.0f, 1.0f, 1.0f });
        deviceContext->EndDraw();
        deviceContext->SetTarget(nullptr);

        winrt::com_ptr<ID2D1Effect> effect;
        deviceContext->CreateEffect(CLSID_D2D1AlphaMask, effect.put());
        effect->SetInput(0, sourceBitmap);
        effect->SetInput(1, tempBitmap);
        deviceContext->SetTarget(destinationBitmap);
        deviceContext->BeginDraw();
        deviceContext->Clear();
        deviceContext->DrawImage(effect.get());
        auto hr = deviceContext->EndDraw();
        deviceContext->SetTarget(nullptr);
#endif


#if 1
        winrt::com_ptr<ID2D1SolidColorBrush> brush;
        if (const auto hr = deviceContext->CreateSolidColorBrush({ 1.0f, 1.0f, 1.0f, 1.0f }, brush.put());
            FAILED(hr))
        {
            jassertfalse;
            return {};
        }

        deviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

        deviceContext->SetTarget(tempBitmap.get());
        deviceContext->BeginDraw();
        deviceContext->Clear();
        deviceContext->FillOpacityMask(sourceBitmap,
            brush.get(),
            juce::D2DUtilities::toRECT_F(destination.getBounds().toFloat()),
            juce::D2DUtilities::toRECT_F(source.getBounds().toFloat()));
        auto hr = deviceContext->EndDraw();
        deviceContext->SetTarget(nullptr);

        winrt::com_ptr<ID2D1Effect> premutiplyEffect;
        deviceContext->CreateEffect(CLSID_D2D1Premultiply, premutiplyEffect.put());
        premutiplyEffect->SetInput(0, tempBitmap.get());
        deviceContext->SetTarget(destinationBitmap);
        deviceContext->BeginDraw();
        deviceContext->DrawImage(premutiplyEffect.get());
        hr = deviceContext->EndDraw();
        deviceContext->SetTarget(nullptr);
#endif

#if 0
        if (FAILED(hr))
            return {};

        winrt::com_ptr<ID2D1Effect> effect;
        deviceContext->CreateEffect(CLSID_D2D1Premultiply, effect.put());
        effect->SetValue(D2D1_COLORMATRIX_PROP_ALPHA_MODE, D2D1_COLORMATRIX_ALPHA_MODE_STRAIGHT);
        effect->SetValue(D2D1_COLORMATRIX_PROP_CLAMP_OUTPUT, FALSE);
        effect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, D2D1_MATRIX_5X4_F{
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 0.0f, 0.0f, 0.0f
            });

        effect->SetInput(0, tempBitmap.get());
        deviceContext->SetTarget(destinationBitmap);
        deviceContext->BeginDraw();
        deviceContext->Clear();
        deviceContext->DrawImage(effect.get());
        hr = deviceContext->EndDraw();
        deviceContext->SetTarget(nullptr);

#endif

        return destination;
    }

    return {};
}

#if MESCAL_UNIT_TESTS

class FormatConverterUnitTest : public juce::UnitTest
{
public:
    FormatConverterUnitTest() : UnitTest("FormatConverterUnitTest") {}

    void runTest() override
    {
        testARGBToSingleChannel();
        testSingleChannelToARGB();
    }

    void testARGBToSingleChannel()
    {
        beginTest("ARGBToSingleChannel");

        juce::Image source{ juce::Image::ARGB, 16, 16, true };
        {
            juce::Graphics g{ source };
            g.setColour(juce::Colours::white);
            g.fillEllipse(source.getBounds().reduced(4).toFloat());
        }

        FormatConverter converter;
        auto destination = converter.convert(source, juce::Image::SingleChannel);

        expect(destination.isValid());

        DBG("source");
        converter.print(source);
        DBG("destination");
        converter.print(destination);

        juce::Image::BitmapData sourceBitmapData{ source, juce::Image::BitmapData::readOnly };
        juce::Image::BitmapData destinationBitmapData{ destination, juce::Image::BitmapData::readOnly };

        for (int y = 0; y < source.getHeight(); ++y)
        {
            for (int x = 0; x < source.getWidth(); ++x)
            {
                auto sourcePixel = sourceBitmapData.getPixelColour(x, y);
                auto destinationPixel = destinationBitmapData.getPixelColour(x, y);

                expectEquals(sourcePixel.getAlpha(), destinationPixel.getAlpha());
            }
        }
    }

    void testSingleChannelToARGB()
    {
        juce::Image source{ juce::Image::SingleChannel, 16, 16, true };
        {
            juce::Graphics g{ source };
            g.setColour(juce::Colours::white.withAlpha(0.75f));
            g.fillEllipse(source.getBounds().reduced(4).toFloat());
        }

        FormatConverter converter;
        auto destination = converter.convert(source, juce::Image::ARGB);

        expect(destination.isValid());

        DBG("source");
        converter.print(source);
        DBG("destination");
        converter.print(destination);

#if 0
        juce::Image::BitmapData sourceBitmapData{ source, juce::Image::BitmapData::readOnly };
        juce::Image::BitmapData destinationBitmapData{ destination, juce::Image::BitmapData::readOnly };

        for (int y = 0; y < source.getHeight(); ++y)
        {
            for (int x = 0; x < source.getWidth(); ++x)
            {
                auto sourcePixel = sourceBitmapData.getPixelColour(x, y);
                auto destinationPixel = destinationBitmapData.getPixelColour(x, y);

                expectEquals(sourcePixel.getAlpha(), destinationPixel.getAlpha());
            }
        }
#endif
    }
};

static FormatConverterUnitTest formatConverterUnitTest;

#endif
