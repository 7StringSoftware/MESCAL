#pragma once

#include "CustomEffect.h"

class CustomEffectDemo  : public juce::Component
{
public:
    CustomEffectDemo()
    {
        setSize(sourceImage.getWidth(), sourceImage.getHeight());
    }

    void paint(juce::Graphics& g) override
    {
        //
        // Apply the effect: source image -> blur -> output image
        //
        //
        customEffect.applyEffect(outputImage, juce::AffineTransform{}, true);

        //
        // Paint the output image
        //
        g.drawImageAt(outputImage, 0, 0);

        g.setColour(juce::Colours::yellow);
        g.drawRect(getLocalBounds());
    }

    void resized() override
    {
        outputImage = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true };
    }

private:
    juce::Image sourceImage = juce::ImageFileFormat::loadFrom(BinaryData::VanGoghstarry_night_jpg, BinaryData::VanGoghstarry_night_jpgSize);
    juce::Image outputImage;
    CustomEffect customEffect;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomEffectDemo)
};
