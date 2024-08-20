#pragma once

#include <JuceHeader.h>
#include "EffectChainComponent.h"

class MainComponent  : public juce::Component{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    double angle = 0.0;
    juce::Image sourceImage{ juce::Image::ARGB, 1000, 1000, true, juce::NativeImageType{} };

    mescal::Effect::Ptr outputEffect;
    EffectChainComponent effectChainComponent;
    double lastMsec = juce::Time::getMillisecondCounterHiRes();
    juce::VBlankAttachment vblank{ this, [this] { animate(); } };

    void animate();
    void paintSourceImage();
    void buildEffectChain();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
