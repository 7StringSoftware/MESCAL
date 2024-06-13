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

    juce::Path clipPath;
    clipPath.addRoundedRectangle(getLocalBounds().reduced(100).toFloat(), 50.0f);
    g.reduceClipRegion(clipPath);

    particles.draw(spriteBatchImage);
    g.drawImageAt(spriteBatchImage, 0, 0);

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
}

void SpriteBatchDemo::Particles::update(float timeSeconds, juce::Rectangle<float> area, juce::Point<float> mousePos)
{
    juce::Random random;
    float elapsedTime = timeSeconds - lastTimestamp;
    lastTimestamp = timeSeconds;

    const int spriteSize = 8;
    const int atlasSize = 256;
    const int spritesPerRow = atlasSize / spriteSize;
    if (atlas.isNull())
    {
        juce::Image source{ juce::Image::ARGB, atlasSize, atlasSize, true };
        atlas = juce::Image{ juce::Image::ARGB, atlasSize, atlasSize, true };
        {
            juce::Graphics g{ source };

            for (int x = 0; x < atlas.getWidth(); x += spriteSize)
            {
                for (int y = 0; y < atlas.getHeight(); y += spriteSize)
                {
                    juce::Path p;
                    juce::Rectangle<int> r{ x, y, spriteSize, spriteSize };
                    p.addStar(r.getCentre().toFloat(), random.nextInt({ 4, 16 }), spriteSize * 0.4f, spriteSize * 0.45f, random.nextFloat() * juce::MathConstants<float>::twoPi);

                    g.setColour(juce::Colours::cyan.withAlpha(random.nextFloat() * 0.5f + 0.5f).contrasting(random.nextFloat() * 0.2f));
                    g.fillPath(p);
                }
            }
        }

        source.getPixelData()->applyGaussianBlurEffect(spriteSize * 0.25f, atlas);
    }

    if (sprites.size() == 0)
    {
        sprites = std::vector<mescal::Sprite>(numSprites);
        velocities = std::vector<Velocity>(numSprites);
    }

    for (int i = 0; i < numSprites; ++i)
    {
        auto& sprite = sprites[i];
        auto& velocity = velocities[i];
        if (sprite.source.isEmpty() || !area.contains(sprite.destination.getCentre()))
        {
            auto sourceX = i % spritesPerRow;
            auto sourceY = i / spritesPerRow;

            sprite.source = juce::Rectangle<int>{ sourceX, sourceY, spriteSize, spriteSize };
            float x = random.nextFloat() * area.getWidth();
            sprite.destination = juce::Rectangle<float>{ x, 0.0f, (float)spriteSize, (float)spriteSize };
            auto distance = mousePos.getDistanceFrom(sprite.destination.getCentre());
            distance = juce::jmax(1.0f, distance);
            velocity.speed = random.nextFloat() * 100.0f;
            velocity.angle = (random.nextFloat() * 0.1f - 0.05f) * juce::MathConstants<float>::twoPi + juce::MathConstants<float>::pi;
        }

        auto center = sprite.destination.getCentre().getPointOnCircumference(elapsedTime * velocity.speed, velocity.angle);
        sprite.destination.setCentre(center);
    }
}

void SpriteBatchDemo::Particles::draw(juce::Image& destinationImage)
{
    spriteBatch.setAtlas(atlas);
    spriteBatch.draw(destinationImage, sprites, true);
}
