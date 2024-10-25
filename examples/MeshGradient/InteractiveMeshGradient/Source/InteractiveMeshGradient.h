#pragma once

#include <JuceHeader.h>

class InteractiveMeshGradient : public juce::Component
{
public:
    InteractiveMeshGradient();

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct BezierControlComponent : public juce::Component
    {
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void paint(juce::Graphics& g) override;

        juce::ComponentDragger dragger;
        std::weak_ptr<mescal::MeshGradient::Patch> patch;
        mescal::MeshGradient::EdgePlacement edgePlacement = mescal::MeshGradient::EdgePlacement::unknown;
        mescal::MeshGradient::BezierControlPointPlacement controlPointPlacement = mescal::MeshGradient::BezierControlPointPlacement::first;
        std::function<void(BezierControlComponent&)> onChange;
    };

    struct InteriorControlComponent : public juce::Component
    {
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void paint(juce::Graphics& g) override;

        juce::ComponentDragger dragger;
        std::weak_ptr<mescal::MeshGradient::Patch> patch;
        mescal::MeshGradient::CornerPlacement placement = mescal::MeshGradient::CornerPlacement::unknown;
        std::function<void(InteriorControlComponent&)> onChange;
    };

    struct VertexComponent : public juce::Component
    {
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void paint(juce::Graphics& g) override;

        juce::ComponentDragger dragger;
        std::weak_ptr<mescal::MeshGradient::Patch> patch;
        mescal::MeshGradient::CornerPlacement placement = mescal::MeshGradient::CornerPlacement::unknown;
        std::function<void(VertexComponent&)> onChange;
    };

    struct PatchComponent : public juce::Component
    {
        PatchComponent(std::shared_ptr<mescal::MeshGradient::Patch> patch_);
        bool hitTest(int x, int y) override;
        void paint(juce::Graphics& g) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void updateOutlinePath();

        juce::Path path;
        std::shared_ptr<mescal::MeshGradient::Patch> patch;
        bool selected = false;
        std::function<void(PatchComponent*)> onSelect;
    };

    struct ColorCalloutContent : public juce::ChangeListener, public juce::Component
    {
        ColorCalloutContent(juce::Colour color) :
            colorSelector(juce::ColourSelector::showColourAtTop |
                juce::ColourSelector::showSliders |
                juce::ColourSelector::showColourspace |
                juce::ColourSelector::showAlphaChannel |
                juce::ColourSelector::editableColour)
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

    std::unique_ptr<mescal::MeshGradient> mesh;
    juce::Image meshImage;
    std::array<VertexComponent, 4> vertexComponents;
    std::array<BezierControlComponent, 8> bezierControlComponents;
    std::array<InteriorControlComponent, 4> interiorControlComponents;
    std::vector<std::unique_ptr<PatchComponent>> patchComponents;
    juce::Label rowCountLabel{ "Rows", "Rows" }, columnCountLabel{ "Columns", "Columns" };
    juce::Slider rowCountSlider{ juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft };
    juce::Slider columnCountSlider{ juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft };
    juce::ToggleButton showControlsToggle{ "Show control points" };

    void createMesh();
    void createComponents();
    void updatePatchComponents();
    void paintMesh(juce::Graphics& g);
    void selectPatch(PatchComponent* patch);
    VertexComponent& getVertexComponent(int row, int column);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InteractiveMeshGradient)
};
