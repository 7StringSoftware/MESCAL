#pragma once

#include "Base.h"

class ContentComponent : public juce::Component
{
public:
    ContentComponent();
    ~ContentComponent() override = default;

    void resized() override;

private:
    std::unique_ptr<juce::Component> demoComponent;
};
