#pragma once

#include "SignedDistanceField.h"
#include "CustomEffect.h"

class CustomEffectDemo  : public juce::Component
{
public:
    CustomEffectDemo()
    {
        setSize(1024, 1024);
    }

    void paint(juce::Graphics& g) override
    {
        //
        // Apply the effect: source image -> blur -> output image
        //
        SignedDistanceField<float>::createSignedDistanceField(rectangles, distanceFieldImage);
        customEffect.applyEffect(distanceFieldImage, outputImage, juce::AffineTransform{}, true);

        //
        // Paint the output image
        //
        g.setColour(juce::Colours::black);
        g.fillAll();
        g.setColour(juce::Colours::orange);
        //g.addTransform(juce::AffineTransform::scale(32.0f));
        g.drawImageAt(outputImage, 0, 0, false);
        //g.setColour(juce::Colours::orange);
        //g.fillRectList(rectangles);
    }

    void resized() override
    {
        distanceFieldImage = juce::Image{ juce::Image::ARGB, 1024, 1024, true, juce::SoftwareImageType{} };
        outputImage = juce::Image{ juce::Image::ARGB, 1024, 1024, true, juce::NativeImageType{} };
        outputImage.setBackupEnabled(false);

        rectangles.clear();

        for (auto size = 0.5f; size < 512.0f; size *= 2.0f)
        {
            rectangles.addWithoutMerging(juce::Rectangle<float>{ size, size }.withCentre({ size, size }));
        }

#if 0
        float step = (float)outputImage.getWidth() * 0.0625f;
        for (float x = 0.0f; x < (float)outputImage.getWidth(); x += step)
        {
            for (float y = 0.0f; y < outputImage.getHeight(); y += step)
            {
                rectangles.addWithoutMerging({ x, y, step * 0.75f, step * 0.75f});
            }
        }
#endif
    }

private:
    juce::Image distanceFieldImage;
    juce::Image outputImage;
    CustomEffect customEffect;
    juce::RectangleList<float> rectangles;
    juce::VBlankAttachment vblank{ this,[this] { repaint(); } };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomEffectDemo)
};
