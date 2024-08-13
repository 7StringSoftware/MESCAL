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
    enum
    {
        m31Galaxy = 1,
        pietModriaan,
        vanGogh,
        checkerboard
    };

    mescal::JSONObject effectInfoCollection = []
        {
            auto jsonVar = juce::JSON::fromString(BinaryData::EffectParameters_json);
            return mescal::JSONObject{ jsonVar };
        }();
    juce::Value effectTypeValue{ (int)mescal::Effect::Type::blend + 1 };
    juce::Value sourceImageValue{ (int)m31Galaxy };
    juce::Value showSourceImageValue{ true };
    std::unique_ptr<mescal::Effect> effect;
    juce::PropertyPanel propertyPanel;

    std::vector<juce::Image> sourceImages;
    juce::Image outputImage;

    std::vector<juce::Component::SafePointer<EffectPropertyValueComponent>> propertyValueComponents;

    void buildPropertyPanel();
    void updateSourceImages();
    void updateEffectType();
    void applyEffect();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
