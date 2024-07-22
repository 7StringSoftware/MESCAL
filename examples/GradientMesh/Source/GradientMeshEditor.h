#pragma once

#include "Base.h"

class GradientMeshEditor : public juce::Component
{
public:
    GradientMeshEditor();

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    std::unique_ptr<mescal::GradientMesh> mesh;
    juce::Image meshImage;
    juce::Rectangle<float> meshImageBounds;

    void createMesh();
    void paintMesh(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GradientMeshEditor)
};
