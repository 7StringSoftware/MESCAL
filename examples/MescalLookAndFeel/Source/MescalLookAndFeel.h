#pragma once

#include <JuceHeader.h>

class MescalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MescalLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle, juce::Slider&) override;
    int getSliderThumbRadius(juce::Slider& slider)
    {
        return juce::jmin(12, slider.isHorizontal() ? static_cast<int> ((float)slider.getHeight() * 0.75f)
            : static_cast<int> ((float)slider.getWidth() * 0.75f));
    }

private:
    static mescal::Effect::Ptr addShadow(juce::Image const& sourceImage, juce::Colour const& shadowColor, float shadowSize, juce::AffineTransform transform);
    static mescal::Effect::Ptr createInnerShadow(juce::Image const& sourceImage, juce::Colour const& shadowColor, float shadowSize, juce::AffineTransform transform);

    juce::Rectangle<float> paintSliderTrack(juce::Graphics& g,
        juce::Point<float> start, juce::Point<float> end,
        juce::Colour backgroundColor, juce::Colour outlineColor,
        float trackThickness, bool horizontal);

    void paintSliderThumb(juce::Graphics& g,
        juce::Rectangle<float> trackArea,
        juce::Colour thumbColor, float sliderPos, float thumbSize, float sliderThickness, bool horizontal);

    struct LinearHorizontalSlider
    {
        LinearHorizontalSlider(juce::Rectangle<int> trackArea_, juce::Slider& slider_, float sliderPos_)
            : thumbTravelArea{ trackArea_ }, slider{ slider_ }, sliderPos{ sliderPos_ },
            expandedPaintArea{ thumbTravelArea.toFloat() },
            thumbSize{ (float)thumbTravelArea.getHeight() * 0.65f },
            trackImage{ juce::Image::ARGB, (int)expandedPaintArea.getWidth(), (int)expandedPaintArea.getHeight(), true},
            thumbImage{ juce::Image::ARGB, trackArea_.getHeight(), trackArea_.getHeight(), true},
            outputImage{ juce::Image::ARGB, (int)expandedPaintArea.getWidth(), (int)expandedPaintArea.getHeight(), true },
            thumbOutputImage{ juce::Image::ARGB, thumbImage.getHeight(), thumbImage.getHeight(), true }
        {
            paintImages();
            createGraph();
        }

        void paintImages();
        void createGraph();
        void paint(juce::Graphics& g);

        juce::Rectangle<int> thumbTravelArea;
        juce::Rectangle<float> expandedPaintArea;
        float thumbSize = 0.0f;
        float cornerProportion = ((expandedPaintArea.getWidth() - (float)thumbTravelArea.getWidth()) * 0.5f) / expandedPaintArea.getWidth();
        float outlineThickness = expandedPaintArea.getHeight() * 0.05f;
        juce::Slider& slider;
        float sliderPos = 0.0f;

        juce::Image trackImage;
        juce::Image outputImage;
        juce::Image thumbImage;
        juce::Image thumbOutputImage;
        mescal::Effect::Ptr effectGraph;
        mescal::Effect::Ptr thumbGraph;
    };

    struct LinearVerticalSlider
    {
        LinearVerticalSlider(juce::Rectangle<int> trackArea_, juce::Slider& slider_, float sliderPos_)
            : thumbTravelArea{ trackArea_ }, slider{ slider_ }, sliderPos{ sliderPos_ },
            expandedPaintArea{ thumbTravelArea.toFloat().expanded(0.0f, (float)(trackArea_.getY() - slider_.getY()) * 2.0f) },
            thumbSize{ (float)thumbTravelArea.getWidth() * 0.65f },
            trackImage{ juce::Image::ARGB, (int)expandedPaintArea.getWidth(), (int)expandedPaintArea.getHeight(), true },
            thumbImage{ juce::Image::ARGB, trackArea_.getWidth(), trackArea_.getWidth(), true },
            outputImage{ juce::Image::ARGB, (int)expandedPaintArea.getWidth(), (int)expandedPaintArea.getHeight(), true },
            thumbOutputImage{ juce::Image::ARGB, thumbImage.getHeight(), thumbImage.getHeight(), true }
        {
            paintImages();
            createGraph();
        }

        void paintImages();
        void createGraph();
        void paint(juce::Graphics& g);

        juce::Rectangle<int> thumbTravelArea;
        juce::Rectangle<float> expandedPaintArea;
        float thumbSize = 0.0f;
        float cornerProportion = ((expandedPaintArea.getHeight() - (float)thumbTravelArea.getHeight()) * 0.5f) / expandedPaintArea.getHeight();
        float outlineThickness = expandedPaintArea.getWidth() * 0.05f;
        juce::Slider& slider;
        float sliderPos = 0.0f;

        juce::Image trackImage;
        juce::Image outputImage;
        juce::Image thumbImage;
        juce::Image thumbOutputImage;
        mescal::Effect::Ptr effectGraph;
        mescal::Effect::Ptr thumbGraph;
    };

    void paint3DButtonImages(juce::Colour backgroundColor, bool buttonHighlighted, bool buttonDown);
    mescal::Effect::Ptr create3DButtonEffectGraph(bool buttonDown, bool buttonHighlighted);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MescalLookAndFeel)
};
