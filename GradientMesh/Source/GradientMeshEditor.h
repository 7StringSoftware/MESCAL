#pragma once

#include "Base.h"
#include "GradientMesh.h"
#include "ControlPointComponent.h"

class GradientMeshEditor  : public juce::Component
{
public:
    GradientMeshEditor();
    ~GradientMeshEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    GradientMesh mesh;
    std::vector<std::unique_ptr<ControlPointComponent>> controlPointComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GradientMeshEditor)
};
