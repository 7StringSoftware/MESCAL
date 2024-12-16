#include "SpriteBatchDemo.h"
#include "Commander.h"

SpriteBatchDemo::SpriteBatchDemo()
{
    setOpaque(true);

    logoImage = juce::Image{ juce::Image::ARGB, 1536, 768, true };
    {
        juce::Graphics g{ logoImage };
        g.setColour(juce::Colours::black);
        g.setFont(juce::FontOptions{ 400.0f, juce::Font::bold });
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

    particles.create(getLocalBounds().toFloat());
}

void SpriteBatchDemo::mouseDown(juce::MouseEvent const& event)
{
}

void SpriteBatchDemo::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::black);
    particles.draw(spriteBatchImage);
    g.drawImageAt(spriteBatchImage, 0, 0);
}

SpriteBatchDemo::Particles::Particles()
{
}

void SpriteBatchDemo::Particles::create(juce::Rectangle<float> area)
{
    juce::Random random;
    const int spriteSize = 8;

    auto numSpritesAcross = sourceImage.getWidth() / spriteSize;
    auto numSpritesDown = sourceImage.getHeight() / spriteSize;
    numSprites = numSpritesAcross * numSpritesDown;
    sprites.reserve(numSprites);
    velocities.reserve(numSprites);
    auto center = sourceImage.getBounds().toFloat().getCentre();

    for (int y = 0; y < sourceImage.getHeight() - spriteSize; y += spriteSize)
    {
        for (int x = 0; x < sourceImage.getWidth() - spriteSize; x += spriteSize)
        {
            juce::Rectangle<int> source{ x, y, spriteSize, spriteSize };

            auto line = juce::Line<float>{ center, source.toFloat().getCentre() };
            auto p = line.getPointAlongLine(line.getLength() * 20.0f);

            juce::Rectangle<float> drawArea{ p.x, p.y, spriteSize, spriteSize };
            sprites.emplace_back(mescal::Sprite{ source, drawArea });

            targets.emplace_back(source.toFloat().getCentre());
            velocities.emplace_back(Velocity{ line.getAngle(), -1.0f });
        }
    }
}

void SpriteBatchDemo::Particles::update(float timeMsec)
{
#if 0
    auto center = sourceImage.getBounds().toFloat().getCentre();
    auto distance = timeMsec * -0.05;

    for (auto& sprite : sprites)
    {
        auto line = juce::Line<float>{ center, sprite.atlasSourceArea.toFloat().getCentre() };
        auto p = line.getPointAlongLine(line.getLength() + distance);
        sprite.drawArea.setCentre(p);
    }
#endif

#if 1
    phase = juce::MathConstants<double>::twoPi * timeMsec * 0.001 * 0.25;
    float amplitude = 100.0f;

    auto xScale = juce::MathConstants<float>::twoPi / (float)sourceImage.getWidth();
    for (auto& sprite : sprites)
    {
        auto yOffset = amplitude * std::sin((float)phase + (float)sprite.atlasSourceArea.getCentreX() * xScale);
        sprite.drawArea.setCentre(sprite.atlasSourceArea.toFloat().getCentre().translated(0.0f, yOffset));
    }
#endif

#if 0
    auto center = sourceImage.getBounds().toFloat().getCentre();

    for (size_t index = 0; index < sprites.size(); ++index)
    {
        auto& velocity = velocities[index];
        if (velocity.speed == 0.0f)
            continue;

        auto& sprite = sprites[index];
        auto line = juce::Line<float>::fromStartAndAngle(sprite.drawArea.getCentre(), velocity.speed * timeMsec * 0.001f, velocity.angle);
        auto distanceToTarget = sprite.drawArea.getCentre().getDistanceFrom(targets[index]);
        if (distanceToTarget <= line.getLength() * 2.0f)
        {
            sprite.drawArea.setCentre(targets[index]);
            velocity.speed = 0.0f;
        }
        else
        {
            sprite.drawArea.setCentre(line.getEnd());
        }
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
