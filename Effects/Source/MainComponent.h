#pragma once

#include <JuceHeader.h>
#include "Direct2DEdgeDetectionEffect.h"
#include "Direct2DEmbossEffect.h"

class MainComponent  : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Image image;
    Direct2DEdgeDetectionEffect edgeDetectionEffect;
    Direct2DEmbossEffect embossEffect;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
