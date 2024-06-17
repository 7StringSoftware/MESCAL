#pragma once

#include "Base.h"

class GradientMeshDemo : public juce::Component
{
public:
    GradientMeshDemo();
    ~GradientMeshDemo() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    enum
    {
        gradientMeshBackground = 1,
        snowfallBackground
    };

    mescal::GradientMesh mesh;
    juce::Image meshImage;
    juce::Image logoImage;
    float gradientOpacity = 0.0f;
    float maskBoundarySize = 0.0f;
    float timestamp = 0.0f;
    juce::GlowEffect glowEffect;
    juce::ToggleButton showMeshToggle{ "Show mesh grid" };
    juce::ComboBox backgroundCombo;

    juce::VBlankAnimatorUpdater updater{ this };
    juce::Animator fadeInAnimator = juce::ValueAnimatorBuilder{}
        .withEasing(juce::Easings::createEaseIn())
        .withDurationMs(4000)
        .withValueChangedCallback([this](auto value)
            {
                gradientOpacity = value;
                repaint();
            })
        .build();

    juce::Animator maskAnimator = juce::ValueAnimatorBuilder{}
        .withEasing(juce::Easings::createEaseIn())
        .withDurationMs(1000)
        .withValueChangedCallback([this](auto value)
            {
                maskBoundarySize = value * 100.0f;
                repaint();
            })
        .withOnCompleteCallback([this]()
            {
                showMeshToggle.setVisible(true);
                backgroundCombo.setVisible(true);
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

    juce::Animator animator = juce::AnimatorSetBuilder{ fadeInAnimator }
        .followedBy(colorAnimator)
        .togetherWith(1000.0).followedBy(maskAnimator)
        .build();

    void paintMesh(juce::Graphics& g);
    void paintMeshWireframe(juce::Graphics& g);
    void paintSnowfall(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GradientMeshDemo)
};
