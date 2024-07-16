#include "BottleDemo.h"

/*

center rectangle
    100 w
    255 h

body top
115 to 145 y
    30 to 70 x
    30 h

*/

BottleDemo::BottleDemo()
{
    shadowEffectChain.addEffect(mescal::Effect::Type::shadow);
    shadowEffectChain.addEffect(mescal::Effect::Type::perspectiveTransform3D);

    auto& effect = shadowEffectChain.getEffect(1);

    liquidFun.createWorld();
}

BottleDemo::~BottleDemo()
{
}

void BottleDemo::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    {
        juce::Graphics ig{ liquidImage };
        ig.setColour(juce::Colours::transparentBlack);
        ig.getInternalContext().fillRect(liquidImage.getBounds(), true);
        liquidFun.paint(ig, liquidFunTransform);
        g.setColour(juce::Colours::black);
        snifterForeground->draw(ig, 0.8f, snifterForegroundTransform);
    }

    {
        juce::Graphics snifterGraphics{ snifterImage };
        snifterGraphics.setColour(juce::Colours::transparentBlack);
        snifterGraphics.getInternalContext().fillRect(liquidImage.getBounds(), true);

        snifterGraphics.setColour(juce::Colours::black);
        snifter->draw(snifterGraphics, 1.0f, snifterForegroundTransform);
        snifterGraphics.drawImageAt(liquidImage, 0, 0);
        snifterOutline->draw(g, 1.0f, snifterForegroundTransform);
    }

    {
        auto& shadow = shadowEffectChain.getEffect(0);
        shadow.setProperty(1, juce::Colours::black.withAlpha(0.5f));
    }

    auto& effect = shadowEffectChain.getEffect(1);
    effect.setProperty(mescal::PerspectiveTransform3DProperty::rotation, mescal::Point3D{ -40.0f, 0.0f, 0.0f });
    //effect.setProperty(mescal::PerspectiveTransform3DProperty::globalOffset, mescal::Point3D{ 0.0f, 20.0f, 30.0f });
    //effect.setProperty(mescal::PerspectiveTransform3DProperty::localOffset, mescal::Point3D{ 100.0f, 200.0f, 300.0f });
    effect.setProperty(mescal::PerspectiveTransform3DProperty::rotationOrigin, mescal::Point3D{ (float)snifterImage.getWidth(), (float)snifterImage.getHeight(), 0.0f});
    shadowEffectChain.applyEffects(snifterImage, effectOutputImage, 1.0f, 1.0f, true);

    g.drawImageAt(effectOutputImage, 0, 0);
    g.drawImageAt(snifterImage, 0, 0);

}

void BottleDemo::resized()
{
    auto drawableSize = snifter->getBounds();

    animationArea = getLocalBounds().removeFromRight(getWidth() / 3).withTrimmedBottom(50).translated(-50, 0);
    snifterArea = juce::Rectangle<int>{ animationArea }.removeFromBottom(getHeight() * 2 / 3);

    float yOffset = snifterArea.getY() - animationArea.getY();
    snifterBackgroundTransform = juce::RectanglePlacement{}.getTransformToFit(drawableSize.toFloat(), snifterArea.toFloat());
    snifterForegroundTransform = juce::RectanglePlacement{}.getTransformToFit(drawableSize.toFloat(), snifterArea.withX(0.0f).withY(yOffset).toFloat());
    liquidFunTransform = snifterForegroundTransform;

    snifterImage = juce::Image{ juce::Image::ARGB, animationArea.getWidth(), animationArea.getHeight(), true };
    liquidImage = juce::Image{ juce::Image::ARGB, animationArea.getWidth(), animationArea.getHeight(), true};
    effectOutputImage = juce::Image{ juce::Image::ARGB, getWidth(), animationArea.getHeight(), true };

    liquidFun.resize(liquidImage.getBounds());
}

void BottleDemo::LiquidFun::createWorld()
{
    float minY = 1000.0f, minX = 1000.0f, maxY = 0.0f, maxX = 0.0f;
    for (auto const& p : snifterBowlPoints)
    {
        minY = juce::jmin(minY, p.getY());
        minX = juce::jmin(minX, p.getX());
        maxY = juce::jmax(maxY, p.getY());
        maxX = juce::jmax(maxX, p.getX());
    }

    snifterBowlBoxWorldArea = juce::Rectangle<float>{ minX, minY, maxX - minX, maxY - minY };
    auto flip = juce::AffineTransform::verticalFlip(snifterBowlBoxWorldArea.getHeight());

    auto it = snifterBowlPoints.begin();
    auto lastPoint = it->transformedBy(flip);
    while (it != snifterBowlPoints.end())
    {
        auto p = it->transformedBy(flip);
        juce::Line<float> line{ lastPoint, p };

        b2BodyDef groundBodyDef;
        auto lineCenter = line.getPointAlongLineProportionally(0.5f);
        auto groundBody = world.CreateBody(&groundBodyDef);
        b2PolygonShape groundBox;
        groundBox.SetAsBox(line.getLength() * 0.5f, 0.5f, { lineCenter.x, lineCenter.y }, line.getAngle() + juce::MathConstants<float>::halfPi);
        groundBody->CreateFixture(&groundBox, 0.0f);

        lastPoint = p;
        ++it;
    }

    const b2ParticleSystemDef particleSystemDef;
    particleSystem = world.CreateParticleSystem(&particleSystemDef);
    particleSystem->SetRadius(3.0f);

    juce::Random random{};
    for (int i = 0; i < 2000; ++i)
    {
        b2ParticleDef pd;
        pd.flags = b2_waterParticle;
        pd.position.Set(150.0f + random.nextFloat() * 5.0f - 2.5f, 150.0f + (float)i * 3.0f);
        pd.velocity.Set(random.nextFloat() * 10.0f - 5.0f, -40.0f);
        particleSystem->CreateParticle(pd);
    }
}

void BottleDemo::LiquidFun::step(double msec)
{
    world.Step((float)(msec * 0.001f),
        8,
        3,
        3);
}

void BottleDemo::LiquidFun::paint(juce::Graphics& g, juce::AffineTransform const &transform)
{
    renderer.render(g,
        world,
        snifterBowlBoxWorldArea,
        snifterBowlBoxWorldArea.transformedBy(transform));
}

void BottleDemo::LiquidFun::resize(juce::Rectangle<int> size)
{
    paintArea = size;

    renderer.resize(size);
}
