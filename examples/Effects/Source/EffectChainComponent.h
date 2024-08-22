#pragma once

#include <JuceHeader.h>
#include "EffectPropertyPanel.h"

class EffectChainComponent  : public juce::Component
{
public:
    EffectChainComponent();
    ~EffectChainComponent() override;

    void setOutputEffect(mescal::Effect::Ptr outputEffect, int imageWidth, int imageHeight);
    void paint(juce::Graphics&) override;
    void resized() override;
    juce::Rectangle<int> getPreferredSize();

private:
    struct NodeComponent;
    struct ConnectionComponent;
    struct OutputConnectorComponent : public juce::Component
    {
        OutputConnectorComponent(NodeComponent& nodeComponent_, int index_);
        void paint(juce::Graphics&) override;

        NodeComponent& nodeComponent;
        int const index;

        juce::Component::SafePointer<ConnectionComponent> connection;
    };

    struct InputConnectorComponent : public juce::Component
    {
        InputConnectorComponent(NodeComponent& nodeComponent_, int index_);
        void paint(juce::Graphics&) override;

        NodeComponent& nodeComponent;
        int const index;

        juce::Component::SafePointer<ConnectionComponent> connection;
    };

    struct NodeComponent : public juce::Component
    {
        NodeComponent(size_t numInputs_);

        static int constexpr textHeight = 44;
        size_t numInputs = 0;
        size_t numOutputs = 1;

        void paint(juce::Graphics&) override;
        void resized() override;

        juce::Image image;
        std::vector<std::unique_ptr<InputConnectorComponent>> inputConnectors;
        OutputConnectorComponent outputConnector;
    };

    struct InputImageComponent : public NodeComponent
    {
        InputImageComponent(juce::Image image_);

        void paint(juce::Graphics&) override;
    };

    struct EffectComponent : public NodeComponent
    {
        EffectComponent(mescal::Effect::Ptr effect_);

        void paint(juce::Graphics&) override;
        void resized() override;
        void mouseUp(juce::MouseEvent const& event) override;
        void showCallout();

        mescal::Effect::Ptr effect;
    };

    struct ConnectionComponent : public juce::Component
    {
        void paint(juce::Graphics&) override;
        bool hitTest(int x, int y) override;

        juce::Component::SafePointer<OutputConnectorComponent> outputConnector;
        juce::Component::SafePointer<InputConnectorComponent> inputConnector;
    };

    struct ViewportContent : public juce::Component
    {
        ViewportContent();

        static void buildEffectChainComponentsRecursive(ViewportContent* parent,
            NodeComponent* downstreamComponent, 
            int downstreamInputIndex,
            mescal::Effect::Ptr effect,
            int depth,
            int& maxDepth);
        static void positionEffectChainComponentsRecursive(NodeComponent* nodeComponent, int& y, int depth);

        void buildEffectChainComponents(mescal::Effect::Ptr newOutputEffect, int imageWidth, int imageHeight);
        void resized() override;
        void paint(juce::Graphics&) override;
        juce::Rectangle<int> getPreferredSize();

        mescal::Effect::Ptr outputEffect = nullptr;
        std::vector<std::unique_ptr<InputImageComponent>> inputImageComponents;
        std::vector<std::unique_ptr<EffectComponent>> effectComponents;
        std::vector<std::unique_ptr<ConnectionComponent>> connectionComponents;
    } viewportContent;
    juce::Viewport viewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EffectChainComponent)
};
