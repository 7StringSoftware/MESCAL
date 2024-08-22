#pragma once

#include "EffectGraph.h"
#include "EffectGraphComponent.h"

class MainComponent  : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    double angle = 0.0;

    EffectGraph effectGraph;
    EffectGraphComponent effectGraphComponent;
    double lastMsec = juce::Time::getMillisecondCounterHiRes();
    juce::VBlankAttachment vblank{ this, [this] { animate(); } };

    void animate();
    void paintSourceImage();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
