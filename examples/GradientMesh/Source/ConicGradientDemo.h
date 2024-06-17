#pragma once

#include "Base.h"

class ConicGradientDemo : public juce::Component
{
public:
    ConicGradientDemo();
    ~ConicGradientDemo() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    mescal::ConicGradient conicGradient;
    juce::Image image;

    void updateConicGradient();
    void paintMesh(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConicGradientDemo)
};
