#pragma once

#include "Base.h"
#include "GradientMeshEditor.h"

class ContentComponent : public juce::Component
{
public:
    ContentComponent();
    ~ContentComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Viewport viewport;
    GradientMeshEditor meshEditor;
};
