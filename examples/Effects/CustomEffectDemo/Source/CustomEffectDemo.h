#pragma once

#include "SignedDistanceField.h"
#include "CustomEffect.h"

class CustomEffectDemo  : public juce::Component
{
public:
    CustomEffectDemo()
    {
        //setSize(sourceImage.getWidth(), sourceImage.getHeight());
        setSize(1024, 1024);
    }

    void paint(juce::Graphics& g) override
    {
        //
        // Apply the effect: source image -> blur -> output image
        //
        //
        //customEffect.applyEffect(outputImage, juce::AffineTransform{}, true);
        SignedDistanceField<float>::createSignedDistanceField(rectangles, outputImage);

        //
        // Paint the output image
        //
        //g.drawImageAt(outputImage, 0, 0);
        g.fillAll(juce::Colours::black);

        g.setColour(juce::Colours::yellow);
        //g.fillRectList(rectangles);

        //g.setTiledImageFill(outputImage,  0, 0, 1.0f);
        g.drawImageAt(outputImage, 0, 0, true);
    }

    void resized() override
    {
        outputImage = juce::Image{ juce::Image::SingleChannel, getWidth(), getHeight(), true, juce::SoftwareImageType{} };

        float x = 0.0f;
        rectangles.clear();
        for (float i = 1.0f; i < 1024.0f; i *= 2.0f)
        {
            rectangles.addWithoutMerging({ x, x, i, i });
            x += i * 0.5f;
        }
    }

private:
    //juce::Image sourceImage = juce::ImageFileFormat::loadFrom(BinaryData::VanGoghstarry_night_jpg, BinaryData::VanGoghstarry_night_jpgSize);
    juce::Image outputImage;
    CustomEffect customEffect;
    juce::RectangleList<float> rectangles;
    juce::VBlankAttachment vblank{ this,[this] { repaint(); } };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomEffectDemo)
};
