#pragma once

#include <JuceHeader.h>

class MescalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MescalLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle, juce::Slider&) override;

private:
    juce::Image inputScratchpad, outputScratchpad;
    std::vector<juce::Image> images;

    mescal::Effect::Ptr createInnerShadow(juce::Image const& sourceImage, juce::Colour const& shadowColor, float shadowSize, juce::AffineTransform transform);
    void paint3DButtonImages(juce::Colour backgroundColor, bool buttonHighlighted, bool buttonDown);
    void paint3DSliderImages(juce::Rectangle<int> area, juce::Slider& slider);
    mescal::Effect::Ptr create3DButtonEffectGraph(bool buttonDown, bool buttonHighlighted);
    mescal::Effect::Ptr create3DSliderEffectGraph(juce::Rectangle<float> area, juce::Slider& slider, float sliderPos);

    void drawLinearHorizontalSlider(juce::Graphics&, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle, juce::Slider&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MescalLookAndFeel)
};
