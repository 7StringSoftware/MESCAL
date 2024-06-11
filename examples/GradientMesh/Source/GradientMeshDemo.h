#pragma once

#include "Base.h"
#include "Document.h"

class GradientMeshDemo : public juce::Component
{
public:
    GradientMeshDemo();
    ~GradientMeshDemo() override;

    void paint(juce::Graphics&) override;
    void paintOverChildren(Graphics& g) override;
    void resized() override;

    void createGradientMesh();

private:
    mescal::GradientMesh mesh;
    mescal::SpriteBatch spriteBatch;
    juce::AffineTransform gradientMeshTransform;

    juce::VBlankAnimatorUpdater updater{ this };
    juce::Animator fadeInAnimator = juce::ValueAnimatorBuilder{}
        .withEasing(juce::Easings::createEaseIn())
        .withDurationMs(4000)
        .withValueChangedCallback([this](auto value)
            {
                displayComponent.gradientOpacity = value;
                createGradientMesh();
                displayComponent.repaint();
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
                displayComponent.timestamp = value;
                createGradientMesh();
                displayComponent.repaint();
            })
        .build();

    struct DisplayComponent : public juce::Component
    {
        DisplayComponent(GradientMeshDemo& owner_);
        void paint(juce::Graphics& g) override;

        GradientMeshDemo& owner;
        juce::Image meshImage;
        juce::Image spriteAtlas;

        float gradientOpacity = 0.0f;
        float timestamp = 0.0f;
        int frameCount = 0;
    } displayComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GradientMeshDemo)
};
