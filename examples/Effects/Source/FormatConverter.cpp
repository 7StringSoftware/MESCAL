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

#if JUCE_DEBUG
    void print(juce::Image const& image)
    {
        juce::Image::BitmapData bitmapData{ image, juce::Image::BitmapData::readOnly };

        switch (image.getFormat())
        {
        case juce::Image::SingleChannel:
        {
            for (int y = 0; y < image.getHeight(); ++y)
            {
                juce::String line;
                for (int x = 0; x < image.getWidth(); ++x)
                {
                    line << juce::String::toHexString(bitmapData.getPixelColour(x, y).getAlpha()).paddedLeft('0', 2);
                }

                DBG(line);
            }
            break;
        }

        case juce::Image::ARGB:
        {
            for (int y = 0; y < image.getHeight(); ++y)
            {
                juce::String line;
                for (int x = 0; x < image.getWidth(); ++x)
                {
                    line << bitmapData.getPixelColour(x, y).toString().paddedLeft('0', 8) << " ";
                }

                DBG(line);
            }
            break;
        }

        case juce::Image::ARGBFloat16:
        {
            auto float16ToFloat32 = [](uint8_t* bytes)
                {
                    uint16_t u16 = *(uint16_t*)bytes;
                    uint16_t sign = u16 & 0x8000;
                    int exponent = ((int)(u16 & 0x7C00) >> 10) - 15;
                    uint16_t mantissa = u16 & 0x03FF;

                    float value = (float)mantissa;
                    value *= std::pow(2.0f, (float)exponent);
                    if (sign) value *= -1.0f;

                    return value;
                };

            auto printFloat = [](float f)
                {
                    return  juce::String{ f, 2 }.paddedLeft(' ', 6);
                };

            for (int y = 0; y < image.getHeight(); ++y)
            {
                juce::String text;
                for (int x = 0; x < image.getWidth(); ++x)
                {
                    auto line = bitmapData.data + bitmapData.lineStride * y;
                    auto pixel = line + bitmapData.pixelStride * x;

                    auto r = float16ToFloat32(pixel + 0);
                    auto g = float16ToFloat32(pixel + 2);
                    auto b = float16ToFloat32(pixel + 4);
                    auto a = float16ToFloat32(pixel + 6);

                    text << printFloat(a) << "/" << printFloat(r) << "/" << printFloat(g) << "/" << printFloat(b) << ", ";
                }

                DBG(text);
            }
            break;
        }
        }
    }
#endif

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

void FormatConverter::print(juce::Image const& image)
{
    pimpl->print(image);
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

    jassertfalse;
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
    juce::Image destination{ juce::Image::ARGB, source.getWidth(), source.getHeight(), true };

    pimpl->createResources(source, destination);

    auto sourceBitmap = pimpl->sourcePixelData->getAdapterD2D1Bitmap();
    auto destinationBitmap = pimpl->destinationPixelData->getAdapterD2D1Bitmap();

    if (sourceBitmap && destinationBitmap)
    {
        auto& deviceContext = pimpl->deviceContext;

        winrt::com_ptr<ID2D1Effect> flood;
        deviceContext->CreateEffect(CLSID_D2D1Flood, flood.put());
        flood->SetValue(D2D1_FLOOD_PROP_COLOR, D2D1_VECTOR_4F{ 1.0f, 1.0f, 1.0f, 1.0f });

        winrt::com_ptr<ID2D1Effect> alphaMaskEffect;
        deviceContext->CreateEffect(CLSID_D2D1AlphaMask, alphaMaskEffect.put());

        alphaMaskEffect->SetInputEffect(0, flood.get());
        alphaMaskEffect->SetInput(1, sourceBitmap);

        deviceContext->SetTarget(destinationBitmap);
        deviceContext->BeginDraw();
        deviceContext->Clear();
        deviceContext->DrawImage(alphaMaskEffect.get());
        auto hr = deviceContext->EndDraw();
        deviceContext->SetTarget(nullptr);

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
        test("SingleChannel to ARGB",
            juce::Image::SingleChannel,
            juce::Image::ARGB);
        test("ARGB to SingleChannel",
            juce::Image::ARGB,
            juce::Image::SingleChannel);
    }

    void test(juce::String testName,
        juce::Image::PixelFormat sourceFormat,
        juce::Image::PixelFormat destinationFormat)
    {
        beginTest(testName);

        juce::Image softwareSource{ sourceFormat, 8, 8, true, juce::SoftwareImageType{} };
        {
            juce::Graphics g{ softwareSource };
            g.setColour(juce::Colours::white.withAlpha(0.75f));
            g.fillEllipse(softwareSource.getBounds().toFloat());
        }

        auto softwareDestination = softwareSource.convertedToFormat(destinationFormat);
        expect(softwareDestination.isValid());

        FormatConverter converter;
        converter.print(softwareSource);
        converter.print(softwareDestination);

        auto nativeSource = juce::NativeImageType{}.convert(softwareSource);
        auto nativeDestination = converter.convert(nativeSource, destinationFormat);
        converter.print(softwareSource);
        converter.print(nativeDestination);

        for (int y = 0; y < softwareDestination.getHeight(); ++y)
        {
            for (int x = 0; x < softwareDestination.getWidth(); ++x)
            {
                auto softwarePixel = softwareDestination.getPixelAt(x, y);
                auto nativePixel = nativeDestination.getPixelAt(x, y);

                expect(softwarePixel == nativePixel);
            }
        }
    }
};

static FormatConverterUnitTest formatConverterUnitTest;

#endif
