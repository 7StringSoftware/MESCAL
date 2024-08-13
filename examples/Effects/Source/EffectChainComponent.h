#pragma once

#include <JuceHeader.h>

class EffectChainComponent  : public juce::Component
{
public:
    EffectChainComponent();
    ~EffectChainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:

    struct NodeComponent : public juce::Component
    {
        int numInputs = 0;
        int numOutputs = 0;

        std::vector<juce::Component::SafePointer<NodeComponent>> inputs;
        juce::Component::SafePointer<NodeComponent> output;
    };

    std::vector<std::unique_ptr<NodeComponent>> nodeComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EffectChainComponent)
};
