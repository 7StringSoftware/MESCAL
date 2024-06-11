#pragma once

#include "Base.h"

class GradientMeshDemo : public juce::Component
{
public:
    GradientMeshDemo();
    ~GradientMeshDemo() override;

    void paint(juce::Graphics&) override;

private:
    mescal::GradientMesh mesh;
    juce::Image meshImage;
    juce::Image logoImage;
    float gradientOpacity = 0.0f;
    float timestamp = 0.0f;
    juce::GlowEffect glowEffect;

    juce::VBlankAnimatorUpdater updater{ this };
    juce::Animator fadeInAnimator = juce::ValueAnimatorBuilder{}
        .withEasing(juce::Easings::createEaseIn())
        .withDurationMs(4000)
        .withValueChangedCallback([this](auto value)
            {
                gradientOpacity = value;
                repaint();
            })
        .withOnCompleteCallback([this]
            {
                updater.addAnimator(colorAnimator);
                colorAnimator.start();
            })
        .build();

    juce::Animator colorAnimator = juce::ValueAnimatorBuilder{}
        .runningInfinitely()
        .withEasing([this](auto t)
            {
                return t;
            })
        .withValueChangedCallback([this](auto value)
            {
                timestamp = value;
                repaint();
            })
        .build();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GradientMeshDemo)
};
