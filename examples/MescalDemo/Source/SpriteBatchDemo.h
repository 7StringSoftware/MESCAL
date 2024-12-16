#pragma once

#include "Base.h"

class SpriteBatchDemo : public juce::Component
{
public:
    SpriteBatchDemo();
    ~SpriteBatchDemo() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(juce::MouseEvent const& event) override;

private:
    juce::Image logoImage;
    juce::Image spriteBatchImage;

    float opacity = 0.0f;

    struct Particles
    {
        Particles();

        void create(juce::Rectangle<float> area);
        void update(float timeMsec);
        void draw(juce::Image& destinationImage);

        struct Velocity
        {
            float angle = 0.0f;
            float speed = 0.0f;
        };

        juce::Image sourceImage = juce::ImageFileFormat::loadFrom(BinaryData::VanGoghstarry_night_jpg, BinaryData::VanGoghstarry_night_jpgSize);
        mescal::SpriteBatch spriteBatch;
        size_t numSprites = 0;
        std::vector<mescal::Sprite> sprites;
        std::vector<juce::Point<float>> targets;
        std::vector<Velocity> velocities;

        double phase = 0.0;
    } particles;

    double startTime = juce::Time::getMillisecondCounterHiRes();
    juce::VBlankAttachment vblankAttachment
    { this, [this]
        {
            auto now = juce::Time::getMillisecondCounterHiRes();
            particles.update((float)(now - startTime));

            repaint();
        }
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpriteBatchDemo)
};
