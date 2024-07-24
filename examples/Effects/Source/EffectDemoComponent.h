#pragma once

#include <JuceHeader.h>

class EffectDemoComponent : public juce::Component
{
public:
    EffectDemoComponent();

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    std::unique_ptr<mescal::Effect> effect = std::make_unique<mescal::Effect>(mescal::Effect::Type::gaussianBlur);
    juce::PropertyPanel propertyPanel;

    std::vector<juce::Image> sourceImages;
    juce::Image outputImage;
};
