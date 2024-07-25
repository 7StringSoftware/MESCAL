#pragma once

#include "Base.h"

class GradientMeshEditor : public juce::Component
{
public:
    GradientMeshEditor();

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct VertexComponent : public juce::Component
    {
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void paint(juce::Graphics& g) override;

        juce::ComponentDragger dragger;
        std::weak_ptr<mescal::GradientMesh::Vertex> vertex;
        std::function<void()> onChange;
    };

    struct PatchComponent : public juce::Component
    {
        PatchComponent(std::weak_ptr<mescal::GradientMesh::Vertex> northwestCorner_);

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseMove(const juce::MouseEvent& e) override;
        void updateOutline();
        void paint(juce::Graphics& g) override;
        bool hitTest(int x, int y) override;

        std::weak_ptr<mescal::GradientMesh::Vertex> northwestCorner;
        juce::Path outline;
    };

    std::unique_ptr<mescal::GradientMesh> mesh;
    juce::Image meshImage;
    juce::Rectangle<float> meshImageBounds;
    std::array<VertexComponent, 4> vertexComponents;
    std::vector<std::unique_ptr<PatchComponent>> patchComponents;

    void createMesh();
    void paintMesh(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GradientMeshEditor)
};
