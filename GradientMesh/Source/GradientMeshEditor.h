#pragma once

#include "Base.h"
#include "GradientMesh.h"
#include "ControlPointComponent.h"
#include "Triangulator.h"
#include "Mesher.h"

class GradientMeshEditor  : public juce::Component
{
public:
    GradientMeshEditor();

    void createConic(float rotationAngle);
    void createSinglePatch();

    ~GradientMeshEditor() override;

    juce::Rectangle<int> getPreferredSize();
    void paint (juce::Graphics&) override;
    void paintSubpath(juce::Graphics& g, const Mesher::Subpath& subpath, juce::Rectangle<float> area);
    void resized() override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    void mouseMove(const MouseEvent& event) override;

private:
    struct EdgeComponent;
    struct VertexComponent : public juce::Component
    {
        VertexComponent(std::weak_ptr<Mesher::Vertex> vertex_);
        ~VertexComponent() override {}

        bool hitTest(int x, int y) override;

        void mouseEnter(const juce::MouseEvent& event) override;
        void mouseExit(const MouseEvent& event) override;

        void paint(juce::Graphics& g) override;

        std::weak_ptr<Mesher::Vertex> vertex;

        std::function<void()> onMouseOver;
    };

    struct EdgeComponent : public juce::Component
    {
        EdgeComponent(std::weak_ptr<Mesher::Edge> edge_);
        ~EdgeComponent() override {}

        bool hitTest(int x, int y) override;

        void mouseEnter(const juce::MouseEvent& event) override;
        void mouseExit(const MouseEvent& event) override;

        void paint(juce::Graphics& g) override;

        std::weak_ptr<Mesher::Edge> edge;
        bool highlighted = false;

        std::function<void()> onMouseOver;
    };

    struct PatchComponent : public juce::Component
    {
        PatchComponent(std::weak_ptr<Mesher::Patch> patch_);
        ~PatchComponent() override {}

        bool hitTest(int x, int y) override;

        void mouseEnter(const juce::MouseEvent& event) override;
        void mouseExit(const MouseEvent& event) override;

        void paint(juce::Graphics& g) override;

        std::weak_ptr<Mesher::Patch> patch;
        bool highlighted = false;
        juce::Path path;
    };
    
    void clearHighlights();
    void highlightVertex(VertexComponent* vertexComponent);
    void highlightEdge(EdgeComponent* edgeComponent);

    GradientMesh mesh;
    juce::Image meshImage;
    std::vector<std::unique_ptr<VertexComponent>> vertexComponents;
    std::vector<std::unique_ptr<EdgeComponent>> edgeComponents;
    std::vector<std::unique_ptr<ControlPointComponent>> controlPointComponents;
    std::vector<std::unique_ptr<PatchComponent>> patchComponents;
    juce::VBlankAttachment vblankAttachment{ this, [this] 
        { 
            double now = juce::Time::getMillisecondCounterHiRes();
            double delta = now - lastMsec;
            lastMsec = now;
            //rotationAngle = delta * 0.001 * 0.1 * juce::MathConstants<float>::twoPi;

            while (rotationAngle > juce::MathConstants<double>::twoPi)
                rotationAngle -= juce::MathConstants<double>::twoPi;

            repaint(); 
        } };

    float zoom = 1.0f;
    double lastMsec = juce::Time::getMillisecondCounterHiRes();
    double rotationAngle = 0.0;

    Mesher mesher;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GradientMeshEditor)
};
