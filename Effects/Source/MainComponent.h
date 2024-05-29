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
    juce::VBlankAttachment vblank{ this, [this] { repaint(); } };
    juce::Image input, blurred, output;
    std::unique_ptr<Direct2DEffect> blur = Direct2DEffect::create(Direct2DEffect::EffectType::gaussianBlur);
    std::unique_ptr<Direct2DEffect> effect = Direct2DEffect::create(Direct2DEffect::EffectType::spotDiffuseLighting);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
