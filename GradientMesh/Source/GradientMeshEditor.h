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
    void paintOverChildren(juce::Graphics& g) override;
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
        addNorthConnectedPatchCommand = 0x100,
        addEastConnectedPatchCommand,
        addSouthConnectedPatchCommand,
        addWestConnectedPatchCommand,
        removePatchCommand
    };

    juce::ApplicationCommandManager& commandManager;
    float zoom = 1.0f;
    juce::AffineTransform patchToZoomedDisplayTransform;
    double lastMsec = juce::Time::getMillisecondCounterHiRes();
    double rotationAngle = 0.0;
    Document document;
    std::weak_ptr<GradientMesh::Patch> selectedPatch;

    struct Drawables
    {
        std::unique_ptr<Drawable> trashCan = juce::Drawable::createFromImageData(BinaryData::TrashcanButtonTransparent_svg, BinaryData::TrashcanButtonTransparent_svgSize);
    } drawables;

    struct DisplayComponent : public juce::Component
    {
        DisplayComponent(GradientMeshEditor& owner_);
        void paint(juce::Graphics& g) override;

        GradientMeshEditor& owner;
        juce::Image meshImage;

        int frameCount = 0;
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

    struct ControlPointComponent : public juce::Component
    {
        ControlPointComponent(juce::AffineTransform& zoomTransform_);
        ~ControlPointComponent() override = default;

        void updateTransform(juce::Point<float> position);

        bool hitTest(int x, int y) override;
        void mouseEnter(const juce::MouseEvent& event) override;
        void mouseExit(const MouseEvent& event) override;
        void mouseDown(const MouseEvent& event) override;
        void mouseDrag(const MouseEvent& event) override;
        void mouseUp(const MouseEvent& event) override;
        void moved() override;

        void paint(juce::Graphics& g) override;

        virtual juce::Point<float> getControlPointPosition() const noexcept = 0;
        virtual void setControlPointPosition(juce::Point<float> position) noexcept = 0;
        void updatePosition();

        juce::AffineTransform& patchToZoomedDisplayTransform;
        juce::Colour color = juce::Colours::white;
        juce::Point<float> startPosition;
        bool highlighted = false;
        bool dragging = true;
        std::function<void() > onMoved;
    };

    struct PatchCornerComponent : public ControlPointComponent
    {
        PatchCornerComponent(GradientMesh::CornerPlacement corner_, juce::AffineTransform& zoomTransform_);

        juce::Point<float> getControlPointPosition() const noexcept override
        {
            if (auto cp = vertex.lock())
            {
                return cp->position;
            }

            return {};
        }
        void setControlPointPosition(juce::Point<float> position) noexcept override
        {
            if (auto cp = vertex.lock())
            {
                cp->position = position;
            }
        }
        void paint(juce::Graphics& g) override;

        const GradientMesh::CornerPlacement corner;
        std::weak_ptr<GradientMesh::Vertex> vertex;
    };

    struct BezierControlComponent : public ControlPointComponent
    {
        BezierControlComponent(juce::AffineTransform& zoomTransform_);

        juce::Point<float> getControlPointPosition() const noexcept override
        {
            if (auto cp = bezier.lock())
            {
                return cp->position;
            }

            return {};
        }
        void setControlPointPosition(juce::Point<float> position) noexcept override
        {
            if (auto cp = bezier.lock())
            {
                cp->position = position;
            }
        }
        void paint(juce::Graphics& g) override;
        std::weak_ptr<GradientMesh::BezierControlPoint> bezier;
    };

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
        explicit EdgeControlComponent(GradientMeshEditor& owner_, GradientMesh::Direction direction_);
        void resized() override;
        void paint(juce::Graphics& g) override;

        GradientMeshEditor& owner;
        const GradientMesh::Direction direction;
        PathButton addPatchButton;
        PathButton lineButton;
        PathButton quadraticButton;
        PathButton cubicButton;
    };

    std::vector<std::unique_ptr<PatchComponent>> patchComponents;
    std::array<std::unique_ptr<PatchCornerComponent>, 4> cornerControlComponents;

    struct EdgeControlGroup
    {
        explicit EdgeControlGroup(GradientMeshEditor& owner_, GradientMesh::Direction direction_, juce::AffineTransform& zoomTransform_);

        EdgeControlComponent edgeControl;
        std::pair<std::unique_ptr<BezierControlComponent>, std::unique_ptr<BezierControlComponent>> bezierControlPair;
    };

    std::array<std::unique_ptr<EdgeControlGroup>, 4> edgeControlGroups;

    struct SVGButton : public juce::Button
    {
        SVGButton(StringRef name, const Drawable * const drawable_) :
            Button(name),
            drawable(drawable_)
        {
            setRepaintsOnMouseActivity(true);
        }

        void paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            float scale = 1.0f;

            if (shouldDrawButtonAsHighlighted)
                scale = 1.2f;

            if (shouldDrawButtonAsDown)
                scale = 0.8f;

            Rectangle<float> r = getLocalBounds().toFloat().withSizeKeepingCentre(getWidth() * 0.8f * scale, getHeight() * 0.8f * scale);
            drawable->drawWithin(g, r, RectanglePlacement::centred, 1.);
        }

        const Drawable* const drawable;
    };

    SVGButton trashButton{ "Remove", drawables.trashCan.get() };

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
    void removeSelectedPatch();
    void setEdgeType(GradientMesh::Direction direction, GradientMesh::EdgeType type);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GradientMeshEditor)
};
