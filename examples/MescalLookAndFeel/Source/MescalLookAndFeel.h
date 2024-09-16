#pragma once

#include <JuceHeader.h>

class MescalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MescalLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle, juce::Slider&) override;

    int getSliderThumbRadius(juce::Slider& slider) override
    {
        float radius = 20.0f;

        if (slider.isHorizontal())
        {
            switch (slider.getTextBoxPosition())
            {
            case juce::Slider::NoTextBox:
                radius = (float)slider.getHeight() * 12.0f / 22.0f;
                break;

            case juce::Slider::TextBoxLeft:
            case juce::Slider::TextBoxRight:
                radius = (float)slider.getHeight() * 12.0f / 22.0f;
                break;

            case juce::Slider::TextBoxAbove:
            case juce::Slider::TextBoxBelow:
                radius = (float)slider.getHeight() * 0.28f;
                break;
            }
        }
        else
        {
            switch (slider.getTextBoxPosition())
            {
            case juce::Slider::NoTextBox:
            case juce::Slider::TextBoxLeft:
            case juce::Slider::TextBoxRight:
                radius = (float)slider.getWidth() * 0.25f;
                break;

            case juce::Slider::TextBoxAbove:
            case juce::Slider::TextBoxBelow:
                radius = (float)slider.getWidth() * 0.25f;
                break;
            }
        }

        return juce::roundToInt(radius);
    }

private:
    static mescal::Effect::Ptr addShadow(juce::Image const& sourceImage, juce::Colour const& shadowColor, float shadowSize, juce::AffineTransform transform);
    static mescal::Effect::Ptr createInnerShadow(juce::Image const& sourceImage, juce::Colour const& shadowColor, float shadowSize, juce::AffineTransform transform);
    static mescal::Effect::Ptr createInnerGlow(juce::Image const& sourceImage, float glowSize, juce::AffineTransform transform);

    void paint3DButtonImages(juce::Colour backgroundColor, bool buttonHighlighted, bool buttonDown);
    mescal::Effect::Ptr create3DButtonEffectGraph(bool buttonDown, bool buttonHighlighted);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MescalLookAndFeel)
};
