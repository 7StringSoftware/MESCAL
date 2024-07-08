#pragma once

#include <JuceHeader.h>
#include "SpotSpecularLightingControlComponent.h"

class MainComponent  : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void initEffect();

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Image input, output;
    std::unique_ptr<mescal::Effect> effect = std::make_unique<mescal::Effect>(mescal::Effect::Type::spotSpecularLighting);

    void updateEffect();
    void applyEffect();

    struct DisplayComponent : public juce::Component
    {
        DisplayComponent(MainComponent& owner_) : owner(owner_)
        {
            setOpaque(false);
        }
        void paint(juce::Graphics&) override;

        MainComponent& owner;
    } displayComponent;

    SpotSpecularLightingControlComponent spotSpecularLightingControlComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
