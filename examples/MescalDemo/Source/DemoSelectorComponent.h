#pragma once

#include "Base.h"

class DemoSelectorComponent : public juce::Button
{
public:
    DemoSelectorComponent();

    void setHitBox(juce::Rectangle<int> hitBox_);
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    bool hitTest(int x, int y) override;
    void resized() override;

protected:
    juce::Rectangle<int> hitBox;
};
