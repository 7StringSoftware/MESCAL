#pragma once

#include <JuceHeader.h>
#include "MescalLookAndFeel.h"

class WidgetsComponent : public juce::Component
{
public:
    WidgetsComponent();
    ~WidgetsComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MescalLookAndFeel mescalLookAndFeel;
    juce::Slider rotarySlider{ juce::Slider::Rotary, juce::Slider::NoTextBox };
    juce::TextButton textButton{ "TextButton" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WidgetsComponent)
};
