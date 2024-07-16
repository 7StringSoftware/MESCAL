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
    liquidFun.createWorld();

#if 0
    std::array<juce::Line<float>, 3> const lines
    {
        juce::Line<float>
        /*{ 40.0f, 0.0f, 60.0f, 0.0f},
        { 30.0f, 5.0f, 70.0f, 5.0f },
        { 30.0f, 115.0f, 70.0f, 115.0f },*/
        { 0.0f, 145.0f, 100.0f, 145.0f },
        { 0.0f, 400.0f, 100.0f, 400.0f },
        { 20.0f, 410.0f, 80.0f, 410.0f }
    };
    auto numRows = lines.size();
    int numColumns = 64;

    mesh = std::make_unique<mescal::GradientMesh>((int)numRows, numColumns);

    juce::Path p;
    auto addPatches = [&](juce::Line<float> topLine, juce::Line<float> bottomLine)
        {
            float height = bottomLine.getEndY() - topLine.getEndY();
            for (int i = 0; i < numColumns; ++i)
            {
                auto proportionalX = (float)i / (float)(numColumns - 1);
                auto x = std::cos(proportionalX * juce::MathConstants<float>::pi) * 0.5f + 0.5f;

                auto p1 = bottomLine.getPointAlongLineProportionally(x);
                auto p2 = topLine.getPointAlongLineProportionally(x);

                auto c1 = juce::Point<float>{ p1.getX(), p1.getY() - height * 0.55f };
                auto c2 = juce::Point<float>{ p2.getX(), p2.getY() };
                if (approximatelyEqual(c1.x, c2.x))
                {
                    p.startNewSubPath(p1);
                    p.lineTo(p2);
                    continue;
                }
                else if (c1.x < c2.x)
                {
                    c2.x -= (c2.x - c1.x) * 0.35f;
                }
                else
                {
                    c2.x += (c1.x - c2.x) * 0.35f;
                }

                p.startNewSubPath(p1);
                p.cubicTo(c1, c2, p2);
            }

            p.startNewSubPath(topLine.getStart());
            p.lineTo(topLine.getEnd());
            p.startNewSubPath(bottomLine.getStart());
            p.lineTo(bottomLine.getEnd());
        };

    float totalHeight = lines.back().getEndY() - lines.front().getEndY();

    {
        int row = 0;
        const auto& line = lines[0];

        for (auto column = 0; column < numColumns; ++column)
        {
            auto proportionalX = (float)column / (float)(numColumns - 1);
            auto x = std::cos(proportionalX * juce::MathConstants<float>::pi) * 0.5f + 0.5f;
            auto pos = line.getPointAlongLineProportionally(1.0f - x);
            auto vertex = mesh->getVertex(row, column);
            vertex->position = pos;
        }
    }

    juce::Line<float> topLine;
    for (int row = 0; row < numRows - 1; ++row)
    {
        auto const& bottomLine = lines[row + 1];

        for (auto column = 0; column < numColumns; ++column)
        {
            auto proportionalColumn = (float)column / (float)(numColumns - 1);
            auto x = std::cos(proportionalColumn * juce::MathConstants<float>::pi) * 0.5f + 0.5f;
            x = 1.0f - x;
            auto bottomPoint = bottomLine.getPointAlongLineProportionally(x);

            auto topVertex = mesh->getVertex(row, column);
            auto bottomVertex = mesh->getVertex(row + 1, column);
            bottomVertex->position = bottomPoint;

            //auto level = std::sin((x + 0.1f) * juce::MathConstants<float>::pi * 1.2f) * 0.8f;
            //level *= std::sin((proportionalY - 0.05f) * juce::MathConstants<float>::halfPi)
            //level = juce::jlimit(0.5f, 1.0f, level);
            //level = level * 0.25f + 0.5f;
            float proportionalTopY = (topVertex->position.y - lines.front().getEndY()) / totalHeight;
            float verticalColorScale = std::sin(proportionalTopY * juce::MathConstants<float>::pi) * 0.15f + 0.85f;
            auto levelPos = x;
            auto level = std::sin((levelPos + 0.25f) * juce::MathConstants<float>::pi * 0.8f);
            level = level * 0.25f * verticalColorScale + 0.75f;
            auto color = juce::Colour::fromFloatRGBA(level, level, level, 0.75f);
            topVertex->setColor(color);
            bottomVertex->setColor(color);

            if (approximatelyEqual(topVertex->position.x, bottomPoint.x))
            {
                continue;
            }

            auto delta = bottomPoint - topVertex->position;
            auto c1 = topVertex->position;
            auto c2 = bottomPoint;

            if (bottomLine.getLength() < topLine.getLength())
            {
                c1.y += delta.y * 0.55f;
                c2.x += delta.x * -0.35f;
            }
            else
            {
                c1.x += delta.x * 0.35f;
                c2.y += delta.y * -0.55f;
            }

            if (auto halfedge = mesh->getHalfedge(topVertex, bottomVertex))
            {
                halfedge->bezierControlPoints = { c1, c2 };
                halfedge->twin.lock()->bezierControlPoints = { c2, c1 };
                halfedge->antialiasing = true;
            }
        }

        topLine = bottomLine;
    }
#endif



#if 0
    {
        juce::Range<int> columnRange{ juce::roundToInt(numColumns * 0.2f), juce::roundToInt(numColumns * 0.4f) };
        float alpha = 0.5f;
        for (int row = 2; row <= 3; ++row)
        {
            float red = 0.2f;
            float redStep = 0.1f;
            for (int column = columnRange.getStart(); column < columnRange.getEnd(); column += 1)
            {
                {
                    auto topLeft = mesh->getVertex(row, column);
                    topLeft->southeastColor = juce::Colour::fromFloatRGBA(red, 0.0f, 0.0f, alpha);

                    auto topRight = mesh->getVertex(row, column + 1);
                    topRight->southwestColor = juce::Colour::fromFloatRGBA(red + redStep, 0.0f, 0.0f, alpha);

                    auto bottomLeft = mesh->getVertex(row + 1, column);
                    bottomLeft->northeastColor = juce::Colour::fromFloatRGBA(red, 0.0f, 0.0f, alpha);

                    auto bottomRight = mesh->getVertex(row + 1, column + 1);
                    bottomRight->northwestColor = juce::Colour::fromFloatRGBA(red + redStep, 0.0f, 0.0f, alpha);

                    red += redStep;
                }

                //             {
                //                 auto vertex = mesh->getVertex(row + 1, column);
                //                 vertex->setColor(juce::Colours::lightgrey);
                //             }
            }
        }
    }
#endif


#if 0
    for (int row = 0; row < numRows - 1; ++row)
    {
        auto topLine = lines[row];
        auto bottomLine = lines[row + 1];

        for (int column = 0; column < numColumns; ++column)
        {
            auto proportionalX = (float)column / (float)(numColumns - 1);
            auto x = std::cos(proportionalX * juce::MathConstants<float>::pi) * 0.5f + 0.5f;
            auto bottomPoint = bottomLine.getPointAlongLineProportionally(x);
            auto topPoint = topLine.getPointAlongLineProportionally(x);

            auto grey = std::sin((x + 0.5f) * juce::MathConstants<float>::pi * 0.75f);
            grey = juce::jlimit(0.0f, 1.0f, grey);
            auto topColor = juce::Colour::greyLevel(grey * 0.5f);
            auto bottomColor = topColor;

#if 0
            if (row == 2 && column >= 32 && column <= 33)
            {
                topColor = juce::Colours::red;
                bottomColor = juce::Colours::blue;
            }
#endif
            if (row == 3 && column >= 24 && column <= 28)
            {
                topColor = juce::Colours::darkred;
                bottomColor = juce::Colours::lightgrey;
            }

            //mesh->configureVertex(row, column, topPoint, topColor);
            //mesh->configureVertex(row + 1, column, bottomPoint, bottomColor);

            auto topVertex = mesh->getVertex(row, column);
            topVertex->position = topPoint;
            auto bottomVertex = mesh->getVertex(row + 1, column);
            bottomVertex->position = bottomPoint;

            topVertex->setColor(topColor);
            bottomVertex->setColor(bottomColor);

            float height = bottomPoint.getY() - topPoint.getY();
            auto c1 = juce::Point<float>{ bottomPoint.getX(), bottomPoint.getY() - height * 0.55f };
            auto c2 = juce::Point<float>{ topPoint.getX(), topPoint.getY() };

            if (approximatelyEqual(topPoint.x, bottomPoint.x))
            {
                continue;
            }

            float xOffset = 0.0f;
            if (bottomPoint.x < topPoint.x)
            {
                xOffset = (c2.x - c1.x) * -0.35f;
            }
            else
            {
                xOffset = (c1.x - c2.x) * 0.35f;
            }
            c2.x += xOffset;

            auto head = mesh->getVertex(row, column);
            auto tail = mesh->getVertex(row + 1, column);
            if (auto halfedge = mesh->getHalfedge(tail, head))
            {
                halfedge->bezierControlPoints = { c1, c2 };
                halfedge->twin.lock()->bezierControlPoints = { c2, c1 };
                halfedge->antialiasing = true;
            }
        }
    }
#endif


#if 0
    int numPatches = 5;
    juce::Line<float> topLine{ topEdgeXRange.getStart(), yRange.getStart(), topEdgeXRange.getEnd(), yRange.getStart() };
    juce::Line<float> bottomLine{ bottomEdgeXRange.getStart(), yRange.getEnd(), bottomEdgeXRange.getEnd(), yRange.getEnd() };

    juce::Path p;

    float xStep = 1.0f / (float)numPatches;
    float height = bottomLine.getEndY() - topLine.getEndY();
    float curveWidth = bottomLine.getEndX() - topLine.getEndX();
    float width = bottomEdgeXRange.getLength();
    int numEdges = numPatches + 1;
    for (int i = 0; i < numEdges; ++i)
    {
        float proportionalX = (float)i / (float)(numEdges - 1);
        auto p1 = bottomLine.getPointAlongLineProportionally(proportionalX);
        auto p2 = topLine.getPointAlongLineProportionally(proportionalX);

        auto c1 = juce::Point<float>{ p1.getX(), p1.getY() - height * 0.55f };
        auto c2 = juce::Point<float>{ p2.getX(), p2.getY() };
        if (approximatelyEqual(c1.x, c2.x))
        {
            p.startNewSubPath(p1);
            p.lineTo(p2);
            continue;
        }
        else if (c1.x < c2.x)
        {
            c2.x -= (c2.x - c1.x) * 0.35f;
        }
        else
        {
            c2.x += (c1.x - c2.x) * 0.35f;
        }

        p.startNewSubPath(p1);
        p.cubicTo(c1, c2, p2);
    }

    p.startNewSubPath(bottomEdgeXRange.getStart(), yRange.getEnd());
    p.lineTo(bottomEdgeXRange.getEnd(), yRange.getEnd());
    p.startNewSubPath(topEdgeXRange.getStart(), yRange.getStart());
    p.lineTo(topEdgeXRange.getEnd(), yRange.getStart());
    p.closeSubPath();
    bottleBodyTopPath = p;
#endif
}

BottleDemo::~BottleDemo()
{
}

void BottleDemo::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    snifter->draw(g, 1.0f, snifterBackgroundTransform);
    {
        juce::Graphics ig{ effectInputImage };
        liquidFun.paint(ig, liquidFunTransform);
        g.setColour(juce::Colours::black);
        snifterForeground->draw(ig, 0.8f, snifterForegroundTransform);
    }
    effect.applyEffect(effectInputImage, effectOutputImage, 1.0f, 1.0f);
    g.drawImageAt(effectOutputImage, animationArea.getX(), animationArea.getY());
    snifterOutline->draw(g, 1.0f, snifterBackgroundTransform);
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

    effectInputImage = juce::Image{ juce::Image::ARGB, animationArea.getWidth(), animationArea.getHeight(), true};
    effectOutputImage = juce::Image{ juce::Image::ARGB, animationArea.getWidth(), animationArea.getHeight(), true };

    liquidFun.resize(effectInputImage.getBounds());
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
