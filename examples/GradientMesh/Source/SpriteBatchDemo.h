#pragma once

#include "Base.h"

class SpriteBatchDemo : public juce::Component
{
public:
    SpriteBatchDemo();
    ~SpriteBatchDemo() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::Image logoImage;
    juce::Image spriteBatchImage;

    float opacity = 0.0f;

    struct Particles
    {
        void update(float timeSeconds, juce::Rectangle<float> area, juce::Point<float> mousePos);
        void draw(juce::Image& destinationImage);

        struct Velocity
        {
            float speed = 0.0f;
            float angle = 0.0f;
        };

        juce::Image atlas;
        mescal::SpriteBatch spriteBatch;
        size_t numSprites = 0;
        std::vector<mescal::Sprite> sprites;
        std::vector<Velocity> velocities;
        std::vector<juce::Point<float>> finalPositions;
        float lastTimestamp = 0.0f;
    } particles;

    juce::Animator spriteAnimator = juce::ValueAnimatorBuilder{}
        .withEasing(juce::Easings::createLinear())
        .runningInfinitely()
        .withValueChangedCallback([this](auto value)
            {
                particles.update(value, getLocalBounds().toFloat(), getMouseXYRelative().toFloat());
                repaint();
            })
        .build();

    juce::VBlankAnimatorUpdater updater{ this };
    juce::Animator fadeInAnimator = juce::ValueAnimatorBuilder{}
        .withEasing(juce::Easings::createLinear())
        .withDurationMs(4000)
        .withValueChangedCallback([this](auto value)
            {
                if (value > 0.5f)
                {
                    opacity = (value - 0.5f) * 2.0f;
                }
                repaint();
            })
        .build();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpriteBatchDemo)
};
