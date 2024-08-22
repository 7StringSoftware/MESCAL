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
    setOpaque(true);

    liquidFun.createWorld();
}

BottleDemo::~BottleDemo()
{
}

void BottleDemo::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    liquidFun.paint(g, {});

#if 0
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
        snifterOutline->draw(snifterGraphics, 1.0f, snifterForegroundTransform);
    }

    {
        auto& shadow = shadowEffectChain.getEffect(0);
        shadow.setProperty(1, juce::Colours::black.withAlpha(0.5f));
    }

    auto& effect = shadowEffectChain.getEffect(1);
    effect.setProperty(mescal::PerspectiveTransform3DProperty::rotation, mescal::Point3D{ -40.0f, 30.0f, 0.0f });
    //effect.setProperty(mescal::PerspectiveTransform3DProperty::globalOffset, mescal::Point3D{ 0.0f, 20.0f, 30.0f });
    //effect.setProperty(mescal::PerspectiveTransform3DProperty::localOffset, mescal::Point3D{ 100.0f, 200.0f, 300.0f });
    effect.setProperty(mescal::PerspectiveTransform3DProperty::perspectiveOrigin, juce::Point<float>{ (float)snifterImage.getWidth() * 0.02f, (float)snifterImage.getHeight() * 0.1f });
    effect.setProperty(mescal::PerspectiveTransform3DProperty::rotationOrigin, mescal::Point3D{ (float)snifterImage.getWidth(), (float)snifterImage.getHeight(), 50.0f });
    shadowEffectChain.applyEffects(snifterImage, effectOutputImage, 1.0f, 1.0f, true);

    int x = getWidth() - snifterImage.getWidth();
    g.drawImageAt(effectOutputImage, x, 0);
    g.drawImageAt(snifterImage, x, 0);
#endif

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
    liquidImage = juce::Image{ juce::Image::ARGB, animationArea.getWidth(), animationArea.getHeight(), true };
    effectOutputImage = juce::Image{ juce::Image::ARGB, getWidth(), animationArea.getHeight(), true };

    liquidFun.resize(getLocalBounds());
}

void BottleDemo::LiquidFun::createWorld()
{
#if 0
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
#endif

    float boxEdgeWidth = 10.0f;
    boxWorldArea = juce::Rectangle<float>{ -boxEdgeWidth, -boxEdgeWidth, 1000.0f + 2.0f * boxEdgeWidth, 1000.0f + 2.0f * boxEdgeWidth};
    std::array<juce::Line<float>, 4> lines
    {
        juce::Line<float>{ boxWorldArea.getTopLeft(), boxWorldArea.getTopRight() },
        { boxWorldArea.getTopRight(), boxWorldArea.getBottomRight() },
        { boxWorldArea.getBottomRight(), boxWorldArea.getBottomLeft() },
        { boxWorldArea.getBottomLeft(), boxWorldArea.getTopLeft() }
    };

    juce::AffineTransform flip = juce::AffineTransform::verticalFlip(boxWorldArea.getHeight());

    for (auto const& line : lines)
    {
        auto p1 = line.getStart().transformedBy(flip);
        auto p2 = line.getEnd().transformedBy(flip);

        b2BodyDef groundBodyDef;
        auto lineCenter = line.getPointAlongLineProportionally(0.5f);
        auto groundBody = world.CreateBody(&groundBodyDef);
        b2PolygonShape groundBox;
        groundBox.SetAsBox(line.getLength() * 0.5f, 0.5f, { lineCenter.x, lineCenter.y }, line.getAngle() + juce::MathConstants<float>::halfPi);
        groundBody->CreateFixture(&groundBox, 0.0f);
    }

    const b2ParticleSystemDef particleSystemDef;
    particleSystem = world.CreateParticleSystem(&particleSystemDef);
    particleSystem->SetRadius(2.0f);

    juce::Random random{};
    auto center = boxWorldArea.getCentre().transformedBy(flip);
    for (int i = 0; i < 100; ++i)
    {
        b2ParticleDef pd;
        pd.flags = b2_waterParticle;
        pd.position.Set(center.x, center.y);
        pd.velocity.Set(random.nextFloat() * 10.0f, random.nextFloat() * 10.0f);
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

void BottleDemo::LiquidFun::paint(juce::Graphics& g, juce::AffineTransform const& transform)
{
    renderer.render(g,
        world,
        boxWorldArea.expanded(200.0f),
        g.getClipBounds().toFloat());
}

void BottleDemo::LiquidFun::resize(juce::Rectangle<int> size)
{
    paintArea = size;

    renderer.resize(size);
}

