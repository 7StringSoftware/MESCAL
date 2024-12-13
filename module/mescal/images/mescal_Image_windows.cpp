#include "mescal_Image_windows.h"

namespace mescal
{
    class MescalImageContext : public juce::Direct2DImageContext
    {
    public:
        MescalImageContext(juce::ComSmartPtr<ID2D1DeviceContext1> deviceContext, juce::ComSmartPtr<ID2D1Bitmap1> bitmap, const juce::RectangleList<int>& paintAreas)
            : Direct2DImageContext(deviceContext, bitmap, paintAreas)
        {
            startFrame(1.0f);
        }

        ~MescalImageContext() override
        {
            endFrame();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MescalImageContext)
    };

    class MescalPixelData : public juce::Direct2DPixelData
    {
    public:
        MescalPixelData(int width, int height, bool clearImage)
            : juce::Direct2DPixelData(juce::Image::ARGB, width, height, clearImage)
        {
        }

        ~MescalPixelData() override
        {
        }

        juce::SharedResourcePointer<juce::DirectX> directX;
        juce::ComSmartPtr<ID2D1DeviceContext1> deviceContext = [&]()
            {
                auto adapter = directX->adapters.getDefaultAdapter();
                return juce::Direct2DDeviceContext::create(adapter);
            }();
        juce::ComSmartPtr<ID2D1Bitmap1> bitmap = [&]() -> juce::ComSmartPtr<ID2D1Bitmap1>
            {
                if (deviceContext)
                {
                    return juce::Direct2DBitmap::createBitmap(deviceContext,
                        juce::Image::ARGB,
                        D2D_SIZE_U{ (UINT)width, (UINT)height },
                        D2D1_BITMAP_OPTIONS_TARGET);
                }

                return {};
            }();

        const juce::RectangleList<int> paintAreas = juce::Rectangle<int>{ width, height };
    };

    MescalImageType::MescalImageType()
    {
    }

    MescalImageType::~MescalImageType()
    {
    }

    juce::ImagePixelData::Ptr MescalImageType::create(juce::Image::PixelFormat, int width, int height, bool clearImage) const
    {
        return new MescalPixelData{ width, height, clearImage };
    }

    int MescalImageType::getTypeID() const
    {
        return 1;
    }
}

