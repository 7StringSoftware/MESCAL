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

SpriteBatchDemo::Particles::Particles()
{
    juce::Random random;
    const int spriteSize = 4;

    numSprites = (sourceImage.getWidth() / spriteSize) * (sourceImage.getHeight() / spriteSize);
    sprites.reserve(numSprites);
    speeds.reserve(numSprites);
    auto center = sourceImage.getBounds().toFloat().getCentre();

    for (int y = 0; y < sourceImage.getHeight() - spriteSize; y += spriteSize)
    {
        for (int x = 0; x < sourceImage.getWidth() - spriteSize; x += spriteSize)
        {
            juce::Rectangle<int> source{ x, y, spriteSize, spriteSize };
            juce::Rectangle<float> destination{ center.x, center.y, (float)spriteSize, (float)spriteSize};
            sprites.emplace_back(mescal::Sprite{ destination, source });

            speeds.emplace_back(random.nextFloat() * 5.0f);
        }
    }
}

void SpriteBatchDemo::Particles::update(float timeMsec)
{
    auto center = sourceImage.getBounds().toFloat().getCentre();

    for (size_t index = 0; index < sprites.size(); ++index)
    {
        auto& sprite = sprites[index];
        auto speed = speeds[index];
        speed = 10.0f;

        juce::Line<float> line{ juce::Point<float>{ (float)sprite.source.getCentreX(), 0.0f }, sprite.source.toFloat().getCentre() };
        auto distance = timeMsec * 0.001f * speed;
        if (distance >= line.getLength())
        {
            sprite.destination = sprite.source.toFloat();
        }
        else
        {
            auto position = line.getPointAlongLine(distance);
            sprite.destination = sprite.source.toFloat().withCentre(position);
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
    if (sourceImage.isNull())
        return;

    spriteBatch.setAtlas(sourceImage);
    spriteBatch.draw(destinationImage, sprites, true);
}
