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
        SignedDistanceField<float>::createSignedDistanceField(rectangles, sourceImage);
        customEffect.applyEffect(sourceImage, outputImage, juce::AffineTransform{}, true);

        //
        // Paint the output image
        //
        g.setColour(juce::Colours::black);
        g.fillAll();
        g.drawImageAt(outputImage, 0, 0, false);
        //g.fillAll(juce::Colours::black);
        //g.drawImageAt(outputImage, 0, 0, false);
        //g.setColour(juce::Colours::orange);
        //g.fillRectList(rectangles);
    }

    void resized() override
    {
        sourceImage = juce::Image{ juce::Image::ARGB, 1024, 1024, true, juce::SoftwareImageType{} };
        outputImage = juce::Image{ juce::Image::ARGB, 1024, 1024, true, juce::NativeImageType{} };
        outputImage.setBackupEnabled(false);

        rectangles.clear();
        for (float x = 0.0f; x < 1024.0f; x += 16.0f)
        {
            for (float y = 0.0f; y < 1024.0f; y += 16.0f)
            {
                rectangles.addWithoutMerging({ x, y, 12.0f, 12.0f});
            }
        }
    }

private:
    juce::Image sourceImage;
    juce::Image outputImage;
    CustomEffect customEffect;
    juce::RectangleList<float> rectangles;
    juce::VBlankAttachment vblank{ this,[this] { repaint(); } };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomEffectDemo)
};
