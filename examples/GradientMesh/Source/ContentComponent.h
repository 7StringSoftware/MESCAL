#pragma once

#include "Base.h"
#include "GradientMeshEditor.h"

class ContentComponent : public juce::Component
{
public:
    ContentComponent(juce::ApplicationCommandManager& commandManager_, Settings& settings_);
    ~ContentComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Viewport viewport;
    GradientMeshEditor meshEditor;
};
