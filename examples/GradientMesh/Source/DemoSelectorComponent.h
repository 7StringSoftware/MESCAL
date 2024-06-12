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
    mescal::ScatterEffect scatterEffect;
    juce::Rectangle<int> hitBox;

    juce::VBlankAnimatorUpdater updater{ this };
    juce::Animator scatterAnimator = juce::ValueAnimatorBuilder{}
        .withEasing(juce::Easings::createEaseIn())
        .withDurationMs(3000)
        .withValueChangedCallback([this](auto value)
            {
                scatterEffect.setScatterDistanceMultiplier(1.0f + (1.0f - value));
                repaint();
            })
        .build();
};
