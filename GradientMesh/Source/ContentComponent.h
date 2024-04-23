#pragma once

#include "Base.h"
#include "GradientMeshEditor.h"

class ContentComponent : public juce::Component
{
public:
    ContentComponent();
    ~ContentComponent() override = default;

    void resized() override;

private:
    GradientMeshEditor meshEditor;
};
