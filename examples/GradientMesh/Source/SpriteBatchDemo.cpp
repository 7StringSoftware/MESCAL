#include "SpriteBatchDemo.h"
#include "Commander.h"

SpriteBatchDemo::SpriteBatchDemo()
{
    setOpaque(true);

    logoImage = juce::Image{ juce::Image::ARGB, 1536, 768, true };
    {
        juce::Graphics g{ logoImage };
        g.setColour(juce::Colours::black);
        g.setFont({ 400.0f, juce::Font::bold });
        g.drawText("MESCAL", logoImage.getBounds(), juce::Justification::centred);
    }

    updater.addAnimator(spriteAnimator);
    updater.addAnimator(fadeInAnimator);
    spriteAnimator.start();
    fadeInAnimator.start();
}

SpriteBatchDemo::~SpriteBatchDemo()
{
}

void SpriteBatchDemo::resized()
{
    if (spriteBatchImage.isNull() || spriteBatchImage.getWidth() != getWidth() || spriteBatchImage.getHeight() != getHeight())
    {
        spriteBatchImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
    }
}

void SpriteBatchDemo::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

//     juce::Path clipPath;
//     clipPath.addRoundedRectangle(getLocalBounds().reduced(100).toFloat(), 50.0f);
//     g.reduceClipRegion(clipPath);

    particles.draw(spriteBatchImage);
    g.drawImageAt(spriteBatchImage, 0, 0, false);

#if 0
    juce::Point<float> offset
    {
        ((float)getWidth() - logoImage.getWidth()) * 0.5f,
        ((float)getHeight() - logoImage.getHeight()) * 0.5f
    };
    g.addTransform(juce::AffineTransform::translation(offset));
    //glowEffect.setGlowProperties(50.0f, juce::Colours::white);
    //glowEffect.applyEffect(logoImage, g, 1.0f, opacity);
    shadowEffect.setShadowProperties(juce::DropShadow{ juce::Colours::white, 50, {} });
    shadowEffect.applyEffect(logoImage, g, 1.0f, opacity);
#endif
}

void SpriteBatchDemo::Particles::update(float timeSeconds, juce::Rectangle<float> area, juce::Point<float> mousePos)
{
    juce::Random random;
    float elapsedTime = timeSeconds - lastTimestamp;
    lastTimestamp = timeSeconds;

    const int spriteSize = 8;
    const int atlasSize = 1024;
    const int spritesPerRow = atlasSize / spriteSize;
    if (atlas.isNull())
    {
        atlas = juce::ImageFileFormat::loadFrom(BinaryData::VanGoghstarry_night_jpg, BinaryData::VanGoghstarry_night_jpgSize);
        sprites = std::vector<mescal::Sprite>(numSprites);
        velocities = std::vector<Velocity>(numSprites);

        auto center = area.getCentre();

        for (int y = 0; y < atlas.getHeight() - spriteSize; y += spriteSize)
        {
            for (int x = 0; x < atlas.getWidth() - spriteSize; x += spriteSize)
            {
                juce::Rectangle<int> source{ x, y, spriteSize, spriteSize };
                auto destination = juce::Rectangle<float>{ (float)x, 0.0f, (float)spriteSize, (float)spriteSize };
                sprites.emplace_back(mescal::Sprite{ destination, source });

                juce::Line<float> line{ destination.getCentre(), source.toFloat().getCentre() };

                velocities.emplace_back(Velocity{ 10.0f, line.getAngle()});
            }
        }
    }

    for (auto& sprite : sprites)
    {
        auto& velocity = velocities[&sprite - &sprites[0]];

        if (sprite.destination.getCentre().getDistanceFrom(sprite.source.toFloat().getCentre()) < 1.0f)
        {
            sprite.destination = sprite.source.toFloat();
        }
        else
        {
            auto center = sprite.destination.getCentre().getPointOnCircumference(elapsedTime * velocity.speed, velocity.angle);
            sprite.destination.setCentre(center);
        }
    }

#if 0
    int sourceX = 0, sourceY = 0;
    for (int i = 0; i < numSprites; ++i)
    {
        auto& sprite = sprites[i];
        auto& velocity = velocities[i];
        if (sprite.source.isEmpty() || !area.contains(sprite.destination.getCentre()))
        {
            sprite.source = juce::Rectangle<int>{ sourceX, sourceY, spriteSize, spriteSize };
            float x = (random.nextFloat() - 0.5f) * area.getWidth() * 0.1f + area.getCentreX();
            float y = (random.nextFloat() - 0.5f) * area.getHeight() * 0.1f + area.getCentreY();
            sprite.destination = juce::Rectangle<float>{ x, y, (float)spriteSize, (float)spriteSize };
            auto distance = mousePos.getDistanceFrom(sprite.destination.getCentre());
            distance = juce::jmax(1.0f, distance);
            velocity.speed = 10.0f;// random.nextFloat() * 100.0f;
            velocity.angle = (random.nextFloat() /** 0.1f - 0.05f*/) * juce::MathConstants<float>::twoPi + juce::MathConstants<float>::pi;

            sourceY += spriteSize;
            if (sourceY >= atlasSize)
            {
                sourceY = 0;
                sourceX += spriteSize;
                sourceX %= atlasSize;
            }
        }

        auto center = sprite.destination.getCentre().getPointOnCircumference(elapsedTime * velocity.speed, velocity.angle);
        sprite.destination.setCentre(center);
    }
#endif
}

void SpriteBatchDemo::Particles::draw(juce::Image& destinationImage)
{
    if (atlas.isNull())
        return;

    spriteBatch.setAtlas(atlas);
    spriteBatch.draw(destinationImage, sprites, true);
}
