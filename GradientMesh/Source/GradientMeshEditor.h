#pragma once

#include "Base.h"
#include "GradientMesh.h"
#include "ControlPointComponent.h"
#include "Triangulator.h"
#include "Mesher.h"
#include "HalfEdgeMesh.h"

class GradientMeshEditor  : public juce::Component
{
public:
    GradientMeshEditor();

    void createConic(float rotationAngle);
    void createSinglePatch();

    ~GradientMeshEditor() override;

    juce::Rectangle<int> getPreferredSize();
    void paint (juce::Graphics&) override;
    void paintSubpath(juce::Graphics& g, const HalfEdgeMesh::Subpath& subpath, juce::Rectangle<float> area);
    void resized() override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    void mouseMove(const MouseEvent& event) override;

private:
    struct HalfEdgeComponent;
    struct VertexComponent : public juce::Component
    {
        VertexComponent(const HalfEdgeMesh::Vertex* const vertex_);
        ~VertexComponent() override {}

        bool hitTest(int x, int y) override;

        void mouseEnter(const juce::MouseEvent& event) override;
        void mouseExit(const MouseEvent& event) override;

        void paint(juce::Graphics& g) override;

        const HalfEdgeMesh::Vertex* const vertex;
        bool highlighted = false;

        std::function<void()> onMouseOver;
    };

    struct HalfEdgeComponent : public juce::Component
    {
        HalfEdgeComponent(const HalfEdgeMesh::Halfedge* halfedge_);
        ~HalfEdgeComponent() override {}

        bool hitTest(int x, int y) override;

        void mouseEnter(const juce::MouseEvent& event) override;
        void mouseExit(const MouseEvent& event) override;

        void paint(juce::Graphics& g) override;

        const HalfEdgeMesh::Halfedge* const halfedge;
        bool highlighted = false;
        juce::Line<float> paintedLine;

        std::function<void()> onMouseOver;
        std::function<void()> onMouseExit;
    };

    struct FaceComponent : public juce::Component
    {
        FaceComponent(const HalfEdgeMesh::Face* const face_);
        ~FaceComponent() override {}

        bool hitTest(int x, int y) override;

        void mouseEnter(const juce::MouseEvent& event) override;
        void mouseExit(const MouseEvent& event) override;

        void paint(juce::Graphics& g) override;

        const HalfEdgeMesh::Face* const face;
        bool highlighted = false;
        juce::Path path;
    };

    void clearHighlights();
    void highlightVertex(VertexComponent* vertexComponent);
    void highlightEdge(HalfEdgeComponent* edgeComponent);

    GradientMesh mesh;
    juce::Image meshImage;
    std::vector<std::unique_ptr<VertexComponent>> vertexComponents;
    std::vector<std::unique_ptr<HalfEdgeComponent>> edgeComponents;
    std::vector<std::unique_ptr<ControlPointComponent>> controlPointComponents;
    std::vector<std::unique_ptr<FaceComponent>> faceComponents;
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
    HalfEdgeMesh halfedgeMesh;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GradientMeshEditor)
};
