#pragma once

#include <JuceHeader.h>
#include "PropertyComponents.h"

class MainComponent  : public juce::Component, public juce::Value::Listener
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void valueChanged(juce::Value& value) override;

private:
    mescal::JSONObject effectInfoCollection = []
        {
            auto jsonVar = juce::JSON::fromString(BinaryData::EffectParameters_json);
            return mescal::JSONObject{ jsonVar };
        }();
    juce::Value effectTypeValue{ (int)mescal::Effect::Type::gaussianBlur + 1 };
    std::unique_ptr<mescal::Effect> effect;
    juce::PropertyPanel propertyPanel;

    std::vector<juce::Image> sourceImages;
    juce::Image outputImage;

    std::vector<juce::Component::SafePointer<EffectPropertyValueComponent>> propertyValueComponents;

    void buildPropertyPanel();
    void updateEffectType();
    void applyEffect();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
