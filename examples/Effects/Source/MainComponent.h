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
    std::unique_ptr<mescal::Effect> effect = std::make_unique<mescal::Effect>(mescal::Effect::Type::gaussianBlur);
    juce::PropertyPanel propertyPanel;

    std::vector<juce::Image> sourceImages;
    juce::Image outputImage;

    std::map<int, juce::Value> valueMap;

    void buildPropertyPanel();
    void updateEffect();
    void applyEffect();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
