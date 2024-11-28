#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <d2d1_3helper.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_graphics/native/juce_Direct2DMetrics_windows.h>
#include <juce_graphics/native/juce_EventTracing.h>
#include <juce_graphics/native/juce_DirectX_windows.h>
#include <juce_graphics/native/juce_Direct2DImage_windows.h>
#include <juce_graphics/native/juce_Direct2DGraphicsContext_windows.h>
#include <juce_graphics/native/juce_Direct2DImageContext_windows.h>
#include "FormatConverter.h"

struct FormatConverter::Pimpl
{
    Pimpl(FormatConverter& owner_) : owner(owner_)
    {
    }

    void release()
    {
        resources->release();
    }

    void createResources()
    {
        resources->create();
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

        case juce::Image::RGB:
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

#if 0
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
#endif
        }
    }
#endif

    FormatConverter& owner;
    struct Resources
    {
        HRESULT create()
        {
            if (!adapter)
            {
                adapter = directX->adapters.getDefaultAdapter();
            }

            if (adapter && !deviceContext)
            {
                juce::ComSmartPtr<ID2D1DeviceContext1> deviceContext1;
                if (const auto hr = adapter->direct2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
                    deviceContext1.resetAndGetPointerAddress());
                    FAILED(hr))
                {
                    jassertfalse;
                    return hr;
                }

                deviceContext1->QueryInterface<ID2D1DeviceContext2>(deviceContext.resetAndGetPointerAddress());
            }

            return S_OK;
        }

        void release()
        {
            deviceContext = nullptr;
            adapter = nullptr;
        }

        juce::SharedResourcePointer<juce::DirectX> directX;
        juce::DxgiAdapter::Ptr adapter;
        juce::ComSmartPtr<ID2D1DeviceContext2> deviceContext;
    };

    juce::SharedResourcePointer<Resources> resources;
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
    return source.convertedToFormat(outputFormat);

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
            return fromSingleChannel(source, outputFormat);
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
            return toSingleChannel(source);
        }

        case juce::Image::RGB:
        case juce::Image::ARGB:
        {
            auto destination = juce::Image{ source.getPixelData()->createType()->create(outputFormat, source.getWidth(), source.getHeight(), true) };

            {
                juce::Graphics g{ destination };
                g.drawImageAt(source, 0, 0);
            }

            return destination;
        }
        break;
        }
    }
    }

    jassertfalse;
    return {};
}

juce::Image FormatConverter::toSingleChannel(const juce::Image& source)
{
    auto argbSource = source.convertedToFormat(juce::Image::SingleChannel);
    return argbSource;
    pimpl->createResources();

    auto& deviceContext = pimpl->resources->deviceContext;
    if (deviceContext && argbSource.isValid())
    {
        if (auto sourcePixelData = dynamic_cast<juce::Direct2DPixelData*>(argbSource.getPixelData()))
        {
            auto sourceBitmap = sourcePixelData->getFirstPageForContext(deviceContext);
            auto destinationPixelData = new juce::Direct2DPixelData{ juce::Image::SingleChannel, sourcePixelData->width, sourcePixelData->height, true };
            juce::Image destinationImage{ destinationPixelData };
            
            auto destinationBitmap = destinationPixelData->getFirstPageForContext(deviceContext);
            if (sourceBitmap && destinationBitmap)
            {
                deviceContext->SetTarget(destinationBitmap);
                deviceContext->BeginDraw();
                deviceContext->DrawImage(sourceBitmap);
                deviceContext->EndDraw();
                deviceContext->SetTarget(nullptr);

                pimpl->release();

                return destinationImage;
            }
        }
    }

    return {};
}

juce::Image FormatConverter::fromSingleChannel(const juce::Image& source, const juce::Image::PixelFormat destinationFormat)
{
    switch (destinationFormat)
    {
    case juce::Image::PixelFormat::RGB:
    {
        return source.convertedToFormat(juce::Image::RGB);
        break;
    }

    case juce::Image::PixelFormat::ARGB:
    {
        juce::Image destination{ juce::Image::ARGB, source.getWidth(), source.getHeight(), true };
        {
            juce::Graphics g{ destination };
            g.drawImageAt(source, 0, 0);
        }
        return destination;
    }
    }


    juce::Direct2DPixelData::Ptr destinationPixelData = new juce::Direct2DPixelData{ destinationFormat, source.getWidth(), source.getHeight(), true };

    pimpl->createResources();

    auto& deviceContext = pimpl->resources->deviceContext;
    if (deviceContext && source.isValid())
    {
        if (auto sourcePixelData = dynamic_cast<juce::Direct2DPixelData*>(source.getPixelData()))
        {
            winrt::com_ptr<ID2D1Effect> flood;
            deviceContext->CreateEffect(CLSID_D2D1Flood, flood.put());
            flood->SetValue(D2D1_FLOOD_PROP_COLOR, D2D1_VECTOR_4F{ 1.0f, 1.0f, 1.0f, 1.0f });

            winrt::com_ptr<ID2D1Effect> alphaMaskEffect;
            deviceContext->CreateEffect(CLSID_D2D1AlphaMask, alphaMaskEffect.put());

            alphaMaskEffect->SetInputEffect(0, flood.get());
            alphaMaskEffect->SetInput(1, sourcePixelData->getFirstPageForContext(deviceContext));

            deviceContext->SetTarget(destinationPixelData->getFirstPageForContext(deviceContext));
            deviceContext->BeginDraw();
            deviceContext->Clear();
            deviceContext->DrawImage(alphaMaskEffect.get());
            auto hr = deviceContext->EndDraw();
            deviceContext->SetTarget(nullptr);

            return juce::Image{ destinationPixelData };
        }
    }

    return {};
}

#if MESCAL_UNIT_TESTS

class FormatConverterUnitTest : public juce::UnitTest
{
public:
    FormatConverterUnitTest() : UnitTest("FormatConverterUnitTest") {}

    static std::array<juce::Image::PixelFormat, 3> constexpr formats{ juce::Image::SingleChannel, juce::Image::RGB, juce::Image::ARGB };
    juce::StringArray const formatNames{ "Unknown", "RGB", "ARGB", "SingleChannel" };

    void runTest() override
    {
        for (auto const sourceFormat : formats)
        {
            for (auto const destFormat : formats)
            {
                test(formatNames[(int)sourceFormat] + " to " + formatNames[(int)destFormat],
                    sourceFormat,
                    destFormat);
            }
        }
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
        DBG("  --- Software " << formatNames[(int)sourceFormat]);
        converter.print(softwareSource);
        DBG("  --- Software " << formatNames[(int)destinationFormat]);
        converter.print(softwareDestination);

        auto nativeSource = juce::NativeImageType{}.convert(softwareSource);
        auto nativeDestination = converter.convert(nativeSource, destinationFormat);
        DBG("  --- Software " << formatNames[(int)sourceFormat]);
        converter.print(softwareSource);
        DBG("  --- D2D " << formatNames[(int)destinationFormat]);
        converter.print(nativeDestination);

        bool ok = true;
        DBG("  --- checking conversion " << testName);
        for (int y = 0; y < softwareDestination.getHeight(); ++y)
        {
            for (int x = 0; x < softwareDestination.getWidth(); ++x)
            {
                auto softwarePixel = softwareDestination.getPixelAt(x, y);
                auto nativePixel = nativeDestination.getPixelAt(x, y);

                expect(softwarePixel == nativePixel);

                if (softwarePixel != nativePixel)
                {
                    ok = false;
                    break;
                }
            }

            if (!ok)
                break;
        }
    }
};

static FormatConverterUnitTest formatConverterUnitTest;

#endif
