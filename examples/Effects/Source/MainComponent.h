#pragma once

#include <JuceHeader.h>
#include "PropertyComponents.h"

class MainComponent  : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    mescal::JSONObject effectProperties;
    std::unique_ptr<mescal::Effect> effect = std::make_unique<mescal::Effect>(mescal::Effect::Type::gaussianBlur);
    juce::PropertyPanel propertyPanel;

    std::vector<juce::Image> sourceImages;
    juce::Image outputImage;

    std::vector<juce::Component::SafePointer<EffectPropertyValueComponent>> propertyValueComponents;

    void buildPropertyPanel();
    void updateEffectType();
    void applyEffect();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
