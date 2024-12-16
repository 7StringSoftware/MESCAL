#pragma once

#include "Base.h"
#include "DemoSelectorComponent.h"

class ContentComponent : public juce::Component
{
public:
    ContentComponent();
    ~ContentComponent() override = default;

    void resized() override;
    void paint(juce::Graphics&) override {}

private:
    std::unique_ptr<juce::Component> demoComponent;
    DemoSelectorComponent demoSelector, demoSelector2, demoSelector3;
};
