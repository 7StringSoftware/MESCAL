#pragma once

#include "Base.h"
#include "ControlPointComponent.h"
#include "Document.h"

class GradientMeshEditor : public juce::Component, public juce::ApplicationCommandTarget
{
public:
    GradientMeshEditor(juce::ApplicationCommandManager& commandManager_);
    ~GradientMeshEditor() override;

    juce::Rectangle<int> getPreferredSize();
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    void mouseMove(const MouseEvent& event) override;

    void selectPatch(std::weak_ptr<GradientMesh::Patch> patch);

    juce::ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands(Array<CommandID>& commands) override;
    void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;

private:
    enum
    {
        addConnectedPatchCommand = 0x100
    };

    juce::ApplicationCommandManager& commandManager;
    float zoom = 1.0f;
    juce::AffineTransform zoomTransform;
    double lastMsec = juce::Time::getMillisecondCounterHiRes();
    double rotationAngle = 0.0;
    Document document;
    std::weak_ptr<GradientMesh::Patch> selectedPatch;

    struct DisplayComponent : public juce::Component
    {
        DisplayComponent(GradientMeshEditor& owner_);
        void paint(juce::Graphics& g) override;

        GradientMeshEditor& owner;
        juce::Image meshImage;
    } displayComponent;

    struct PatchComponent : public juce::Component
    {
        PatchComponent(GradientMeshEditor& owner_, std::weak_ptr<GradientMesh::Patch> patch_);
        ~PatchComponent() override {}

        bool hitTest(int x, int y) override;

        void mouseEnter(const juce::MouseEvent& event) override;
        void mouseExit(const MouseEvent& event) override;
        void mouseUp(const MouseEvent& event) override;

        void paint(juce::Graphics& g) override;

        GradientMeshEditor& owner;
        std::weak_ptr<GradientMesh::Patch> patch;
        bool selected = false;
    };

    struct PatchCornerComponent : public juce::Component
    {
        PatchCornerComponent(juce::AffineTransform& zoomTransform_, GradientMesh::CornerPosition cornerPosition_);
        ~PatchCornerComponent() override = default;

        bool hitTest(int x, int y) override;
        void mouseEnter(const juce::MouseEvent& event) override;
        void mouseExit(const MouseEvent& event) override;
        void mouseDown(const MouseEvent& event) override;
        void mouseDrag(const MouseEvent& event) override;
        void mouseUp(const MouseEvent& event) override;

        void paint(juce::Graphics& g) override;

        juce::AffineTransform& zoomTransform;
        GradientMesh::CornerPosition const cornerPosition;
        juce::Point<float> startPosition;
        bool highlighted = false;
        bool dragging = true;
        std::weak_ptr<GradientMesh::Vertex> controlPoint;

        std::function<void()> onDrag;
    };

    struct ControlPointComponent : public juce::Component
    {
        ControlPointComponent(juce::AffineTransform& zoomTransform_);
        ~ControlPointComponent() override = default;

        void setControlPoint(std::weak_ptr<GradientMesh::Vertex> controlPoint_);
        void updateTransform(juce::Point<float> position);

        bool hitTest(int x, int y) override;
        void mouseEnter(const juce::MouseEvent& event) override;
        void mouseExit(const MouseEvent& event) override;
        void mouseDown(const MouseEvent& event) override;
        void mouseDrag(const MouseEvent& event) override;
        void mouseUp(const MouseEvent& event) override;

        void paint(juce::Graphics& g) override;

        juce::AffineTransform& zoomTransform;
        juce::Point<float> startPosition;
        bool highlighted = false;
        bool dragging = true;
        std::weak_ptr<GradientMesh::Vertex> controlPoint;

        std::function<void(GradientMesh::Vertex*) > onDrag;
    };


#if 0
    struct ControlPointComponents
    {
        std::array<std::unique_ptr<ControlPointComponent>, 16> array;
        auto get(size_t row, size_t column) { return array[row * 4 + column].get(); }
    } controlPointComponents;
#endif

    struct AddPatchButton : public juce::Button
    {
        AddPatchButton();
        void paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

        //GradientMesh::Edge edge;
    };

    struct PathButton : public juce::Button
    {
        PathButton() :
            Button({})
        {
        }

        void paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

        juce::Path path;
    };

    struct EdgeControlComponent : public juce::Component
    {
        EdgeControlComponent(GradientMeshEditor& owner_, size_t edgePosition_);
        void setEdgeType(GradientMesh::EdgeType type);
        void resized() override;
        void paint(juce::Graphics& g) override;

        GradientMeshEditor& owner;
        size_t const edgePosition;
        PathButton addPatchButton;
        PathButton lineButton;
        PathButton quadraticButton;
        PathButton cubicButton;
    };

    std::vector<std::unique_ptr<PatchComponent>> patchComponents;

    std::array<std::unique_ptr<ControlPointComponent>, 4> cornerControlComponents;

    std::array<EdgeControlComponent, 4> edgeControlComponents
    {
        EdgeControlComponent{ *this, GradientMesh::EdgePosition::top },
        EdgeControlComponent{ *this, GradientMesh::EdgePosition::right },
        EdgeControlComponent{ *this, GradientMesh::EdgePosition::bottom },
        EdgeControlComponent{ *this, GradientMesh::EdgePosition::left }
    };

    juce::VBlankAttachment vblankAttachment{ this, [this]
        {
            double now = juce::Time::getMillisecondCounterHiRes();
            //double delta = now - lastMsec;
            lastMsec = now;
            //rotationAngle = delta * 0.001 * 0.1 * juce::MathConstants<float>::twoPi;

            while (rotationAngle > juce::MathConstants<double>::twoPi)
                rotationAngle -= juce::MathConstants<double>::twoPi;

            repaint();
        } };

    void positionControls();

    void addConnectedPatch(const InvocationInfo& info);
    void setEdgeType(size_t edgePosition, GradientMesh::EdgeType type);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GradientMeshEditor)
};
