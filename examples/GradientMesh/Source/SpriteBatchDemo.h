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
        Particles();

        void update(float timeMsec);
        void draw(juce::Image& destinationImage);

        struct Velocity
        {
            float speed = 0.0f;
            float angle = 0.0f;
        };

        juce::Image sourceImage = juce::ImageFileFormat::loadFrom(BinaryData::VanGoghstarry_night_jpg, BinaryData::VanGoghstarry_night_jpgSize);
        mescal::SpriteBatch spriteBatch;
        size_t numSprites = 0;
        std::vector<mescal::Sprite> sprites;
        std::vector<float> speeds;
    } particles;

    double startTime = juce::Time::getMillisecondCounterHiRes();
    juce::VBlankAttachment vblankAttachment
    { this, [this]
        {
            auto now = juce::Time::getMillisecondCounterHiRes();
            particles.update(now - startTime);

            repaint();
        }
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpriteBatchDemo)
};
