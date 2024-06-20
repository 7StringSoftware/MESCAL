#pragma once

#include <JuceHeader.h>

class MainComponent  : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Image input, output;
    mescal::Effect effect{ mescal::Effect::Type::gaussianBlur };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
