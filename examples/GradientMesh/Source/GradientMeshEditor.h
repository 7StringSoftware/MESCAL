#pragma once

#include "Base.h"

class GradientMeshEditor : public juce::Component
{
public:
    GradientMeshEditor();

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct BezierControlComponent : public juce::Component
    {
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void paint(juce::Graphics& g) override;

        juce::ComponentDragger dragger;
        std::weak_ptr<mescal::GradientMesh::Vertex> vertex;
        mescal::GradientMesh::Placement placement;
        std::function<void(BezierControlComponent&)> onChange;
    };

    struct VertexComponent : public juce::Component
    {
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void paint(juce::Graphics& g) override;

        juce::ComponentDragger dragger;
        std::weak_ptr<mescal::GradientMesh::Vertex> vertex;
        std::function<void(VertexComponent&)> onChange;
    };

    struct PatchComponent : public juce::Component
    {
        PatchComponent(int row_, int column_);
        bool hitTest(int x, int y) override;
        void paint(juce::Graphics& g) override;
        void build(std::shared_ptr<mescal::GradientMesh::Vertex> topLeftVertex, GradientMeshEditor& owner);
        void mouseUp(const juce::MouseEvent& e) override;

        int const row, column;
        juce::Path path;
        bool selected = false;
        std::function<void(PatchComponent*)> onSelect;
    };

    struct ColorCalloutContent : public juce::ChangeListener, public juce::Component
    {
        ColorCalloutContent(juce::Colour color) :
            colorSelector(juce::ColourSelector::showColourAtTop | juce::ColourSelector::showSliders | juce::ColourSelector::showColourspace)
        {
            colorSelector.setCurrentColour(color);
            colorSelector.addChangeListener(this);
            addAndMakeVisible(colorSelector);
        }

        void resized() override
        {
            colorSelector.setBounds(getLocalBounds());
        }

        void changeListenerCallback(juce::ChangeBroadcaster*) override
        {
            if (onChange)
                onChange(colorSelector.getCurrentColour());
        }

        juce::ColourSelector colorSelector;
        std::function<void(juce::Colour)> onChange;
    };

    int numRows = 4;
    int numColumns = 4;
    std::unique_ptr<mescal::GradientMesh> mesh;
    juce::Image meshImage;
    juce::Rectangle<float> meshBounds;
    std::array<VertexComponent, 4> vertexComponents;
    std::array<BezierControlComponent, 8> bezierControlComponents;
    std::vector<std::unique_ptr<PatchComponent>> patchComponents;

    void createMesh();
    void createComponents();
    void buildPatches();
    void paintMesh(juce::Graphics& g);
    void selectPatch(PatchComponent* patch);
    VertexComponent& getVertexComponent(int row, int column);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GradientMeshEditor)
};
