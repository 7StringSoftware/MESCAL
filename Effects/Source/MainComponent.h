#pragma once

#include <JuceHeader.h>
#include "Direct2DEffect.h"

class MainComponent  : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Image input, output;
    Direct2DEffect effect{ Direct2DEffect::EffectType::spotDiffuseLighting };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
