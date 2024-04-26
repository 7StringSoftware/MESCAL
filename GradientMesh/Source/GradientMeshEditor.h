#pragma once

#include "Base.h"
#include "GradientMesh.h"
#include "ControlPointComponent.h"

class GradientMeshEditor  : public juce::Component
{
public:
    GradientMeshEditor();

    void createConic();
    void createSinglePatch();

    ~GradientMeshEditor() override;

    juce::Rectangle<int> getPreferredSize();
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

private:
    struct PatchComponent : public juce::Component
    {
        PatchComponent(GradientMesh::Patch::Ptr patch_);
        ~PatchComponent() override {}

        bool hitTest(int x, int y) override;

        void mouseEnter(const juce::MouseEvent& event) override;
        void mouseExit(const MouseEvent& event) override;

        void paint(juce::Graphics& g) override;

        GradientMesh::Patch::Ptr patch;
    };

    GradientMesh mesh;
    juce::Image meshImage;
    std::vector<std::unique_ptr<ControlPointComponent>> controlPointComponents;
    std::vector<std::unique_ptr<PatchComponent>> patchComponents;
    juce::VBlankAttachment vblankAttachment{ this, [this] { repaint();  } };

    float zoom = 1.0f;
    double phase = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GradientMeshEditor)
};
