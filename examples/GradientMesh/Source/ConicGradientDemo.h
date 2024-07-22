#pragma once

#include "Base.h"

class ConicGradientDemo : public juce::Component
{
public:
    ConicGradientDemo();

    void createSliders();

    void paint(juce::Graphics&) override;
    void resized() override;


private:
    juce::Rectangle<int> conicGradientBounds;
    juce::Image image;
    mescal::ConicGradient conicGradient;
    juce::ComboBox presetCombo;

    enum PresetType
    {
        royGBiv = 1,
        hsv,
        grayscale
    };

    void setGradientStops(PresetType presetType);

    struct ArcSlider : public juce::Component
    {
        ArcSlider(ConicGradientDemo& owner_, size_t index_);

        void setAngle(float angle);
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;

        ConicGradientDemo& owner;
        size_t index;
        float dragRadius = 0.0f;
        float mouseDragStartAngle = 0.0f;
        float startAngle = 0.0f;
        juce::Range<float> arcRange;
        std::function<void(size_t, float)> onChange;
    };

    std::vector <std::unique_ptr<ArcSlider>> sliders;

    void updateSliders();
    void paintConicGradient(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConicGradientDemo)
};
