#pragma once

#include <JuceHeader.h>

class MescalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MescalLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle, juce::Slider&) override;

    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override;

    int getSliderThumbRadius(juce::Slider& slider) override
    {
        auto radius = 20.0f;
        auto width = (float)slider.getWidth();
        auto height = (float)slider.getHeight();

        if (slider.getSliderStyle() == juce::Slider::IncDecButtons)
        {
            if (slider.isHorizontal())
            {
                radius = height;
            }
            else
            {
                radius = width;
            }
        }
        else
        {
            if (slider.isHorizontal())
            {
                switch (slider.getTextBoxPosition())
                {
                case juce::Slider::NoTextBox:
                    radius = height * 0.45f;
                    break;

                case juce::Slider::TextBoxLeft:
                case juce::Slider::TextBoxRight:
                    radius = height * 0.45f;
                    break;

                case juce::Slider::TextBoxAbove:
                case juce::Slider::TextBoxBelow:
                    radius = height * 0.24f;
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
                    radius = width * 0.35f;
                    break;

                case juce::Slider::TextBoxAbove:
                case juce::Slider::TextBoxBelow:
                    radius = width * 0.22f;
                    break;
                }
            }
        }

        return juce::roundToInt(radius);
    }

    void drawLabel(juce::Graphics& g, juce::Label& label) override;

private:
    static mescal::Effect::Ptr addShadow(juce::Image const& sourceImage, juce::Colour const& shadowColor, float shadowSize, juce::AffineTransform transform);

    static mescal::Effect::Ptr create3DInnerShadow(juce::Image const& sourceImage,
        juce::Colour topColor,
        juce::AffineTransform topShadowTransform,
        juce::Colour bottomColor,
        juce::AffineTransform bottomShadowTransform,
        float shadowSize);

    std::vector<juce::Image> images;
    juce::Image getImage(int index, juce::Rectangle<int> size);
    juce::Image getImage(int index, juce::Rectangle<float> size)
    {
        return getImage(index, size.toNearestIntEdges());
    }

    void clear(juce::Graphics& g);

    struct InnerShadow
    {
        mescal::Effect::Ptr flood = mescal::Effect::create(mescal::Effect::Type::flood);
        mescal::Effect::Ptr arithmeticComposite = mescal::Effect::create(mescal::Effect::Type::arithmeticComposite);
        mescal::Effect::Ptr upperShadow = mescal::Effect::create(mescal::Effect::Type::shadow);
        mescal::Effect::Ptr upperTransform = mescal::Effect::create(mescal::Effect::Type::affineTransform2D);
        mescal::Effect::Ptr lowerShadow = mescal::Effect::create(mescal::Effect::Type::shadow);
        mescal::Effect::Ptr lowerTransform = mescal::Effect::create(mescal::Effect::Type::affineTransform2D);
        mescal::Effect::Ptr blend = mescal::Effect::create(mescal::Effect::Type::blend);
        mescal::Effect::Ptr alphaMask = mescal::Effect::create(mescal::Effect::Type::alphaMask);

        void configure(mescal::Effect::Input input,
            juce::Colour topColor,
            juce::AffineTransform topShadowTransform,
            juce::Colour bottomColor,
            juce::AffineTransform bottomShadowTransform,
            float shadowSize);

        auto getEffect() const
        {
            return alphaMask;
        }
    } innerShadow;

    struct DropShadow
    {
        mescal::Effect::Ptr shadow = mescal::Effect::create(mescal::Effect::Type::shadow);
        mescal::Effect::Ptr transformEffect = mescal::Effect::create(mescal::Effect::Type::affineTransform2D);

        void configure(mescal::Effect::Input input, juce::Colour shadowColor, float shadowSize, juce::AffineTransform transform);

        auto getEffect() const
        {
            return transformEffect;
        }
    } dropShadow;

    struct Glow
    {
        mescal::Effect::Ptr shadow = mescal::Effect::create(mescal::Effect::Type::shadow);
        mescal::Effect::Ptr blur = mescal::Effect::create(mescal::Effect::Type::gaussianBlur);
        mescal::Effect::Ptr blend = mescal::Effect::create(mescal::Effect::Type::blend);
        mescal::Effect::Ptr composite = mescal::Effect::create(mescal::Effect::Type::composite);

        void configure(mescal::Effect::Input input, juce::Colour glowColor, float glowSize);

        auto getEffect() const
        {
            return blend;
        };
    } glow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MescalLookAndFeel)
};
