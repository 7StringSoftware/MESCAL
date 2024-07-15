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
#if 0

    mesh->draw(meshImage, transform, juce::Colours::transparentBlack);


    //effect.setProperty(mescal::SpotSpecularLightingProperty::lightPosition, mescal::Point3D{ 100.0f, 100.0f, 100.0f });
    //effect.setProperty(mescal::SpotSpecularLightingProperty::focusPointPosition, mescal::Point3D{ 100.0f, 100.0f, 000.0f });

    //effect.setProperty(mescal::SpotSpecularLightingProperty::surfaceScale, 100.0f);
    //effect.setProperty(mescal::SpotSpecularLightingProperty::lightColor, mescal::RGBColor{ 0.0f, 1.0f, 0.5f });
    juce::Rectangle<int> clipR{ 0, 115, 40, 260 };
    clipR = clipR.transformedBy(transform);
    //g.drawImageAt(meshImage, 0, 0);

    g.setColour(juce::Colours::black);
        //g.strokePath(outline, juce::PathStrokeType{ 1.0f });

    auto center = outline.getBounds().getCentre();
    std::vector<juce::Point<float>> points;
    juce::PathFlatteningIterator it{ outline, {}, 20.0f };
    while (it.next())
    {
        points.emplace_back(juce::Point<float>{ it.x2, it.y2 });
    }

    std::sort(points.begin(), points.end(), [&](juce::Point<float> const& p1, juce::Point<float> const& p2)
        {
            //auto distance1 = p1.getDistanceFrom(center);
            //auto distance2 = p2.getDistanceFrom(center);
            auto angle1 = center.getAngleToPoint(p1);
            auto angle2 = center.getAngleToPoint(p2);
            return angle1 < angle2;
        });

    std::vector<juce::Point<float>> innerPoints;

    float startAngle = -juce::MathConstants<float>::twoPi;
    float angleStep = juce::MathConstants<float>::twoPi / 32;
    size_t index = 0;
    size_t segmentIndex = 0;
    while (startAngle <= juce::MathConstants<float>::twoPi && index < points.size())
    {
        auto angle = center.getAngleToPoint(points[index]);
        if (angle >= startAngle + angleStep)
        {
            float distance = 1000000.0f;
            juce::Point<float> p;
            for (size_t i = segmentIndex; i <= index; ++i)
            {
                auto d = center.getDistanceFrom(points[i]);
                if (d < distance)
                {
                    distance = d;
                    p = points[i];
                }
            }

            innerPoints.emplace_back(p);

            DBG((innerPoints.size() - 1) << " { " << p.x << ", " << p.y << "},");

            g.setColour(juce::Colours::black);
            //g.fillEllipse(juce::Rectangle<float>{ 5.0f, 5.0f }.withCentre(p));

            startAngle += angleStep;
            segmentIndex = index + 1;
        }

        index++;
    }
#endif
// 
    auto transform = juce::AffineTransform::scale(2.0f).translated(400.0f, 0.0f);
    snifter->draw(g, 1.0f, transform);
    liquidFun.paint(g, transform);;
    snifterForeground->draw(g, 0.4f, transform);

#if 0
    for (auto const& halfedge : mesh->getHalfedges())
    {
        juce::Line<float> line{ halfedge->tail.lock()->position, halfedge->head.lock()->position };
        line = { line.getStart().transformedBy(pathTransform), line.getEnd().transformedBy(pathTransform) };
        g.setColour(juce::Colours::purple);
        g.drawArrow(line.withShortenedStart(5.0f).withShortenedEnd(5.0f), 2.0f, 12.0f, 12.0f);
    }
#endif

#if 0
    for (auto const& vertex : mesh->getVertices())
    {
        g.setColour(juce::Colours::hotpink);
        g.fillEllipse(juce::Rectangle<float>{ 5.0f, 5.0f }.withCentre(vertex->position));
        continue;

        g.setColour(juce::Colours::black);

        std::array<juce::Colour, 4> colors
        {
            juce::Colours::red, juce::Colours::blue, juce::Colours::yellow, juce::Colours::green
        };

        std::array<std::weak_ptr<mescal::GradientMesh::Halfedge>, 4> halfedges
        {
            vertex->northHalfedge, vertex->eastHalfedge, vertex->southHalfedge, vertex->westHalfedge
        };

        for (int i = 0; i < 4; ++i)
        {
            if (auto halfedge = halfedges[i].lock())
            {
                juce::Line<float> line{ halfedge->tail.lock()->position, halfedge->head.lock()->position };
                line = { line.getStart().transformedBy({}), line.getEnd().transformedBy({}) };

                auto angle = line.getAngle();
                angle += juce::MathConstants<float>::halfPi;

                line.withShortenedStart(5.0f).withShortenedEnd(5.0f);
                line = juce::Line<float>{ line.getStart().getPointOnCircumference(5.0f, angle), line.getEnd().getPointOnCircumference(5.0f, angle) };

                g.setColour(colors[i]);
                g.drawArrow(line, 2.0f, 12.0f, 12.0f);
            }
        }
    }
#endif
}

void BottleDemo::resized()
{
#if 0
    mesh = std::make_unique<mescal::GradientMesh>(8, 8);
    std::array<juce::Colour, 8> colors
    {
        juce::Colours::darkgreen,
        juce::Colours::green,
        juce::Colours::limegreen,
        juce::Colours::green,
        juce::Colours::green,
        juce::Colours::darkgreen,
        juce::Colours::darkgreen,
        juce::Colours::darkgrey
    };
    std::array<float, 8> shade
    {
        0.0f, 0.0f, 0.2f, 0.4f, 1.0f, 1.0f, 1.0f, 0.0f
    };
    auto bounds = transformedBottlePath.getBounds();
    float xStep = bounds.getWidth() / (float)(mesh->getNumColumns() - 1);
    float yStep = bounds.getHeight() / (float)(mesh->getNumRows() - 1);
    mesh->configureVertices([=](int row, int column, std::shared_ptr<mescal::GradientMesh::Vertex> vertex)
        {
            juce::Point<float> p{ (float)column * xStep, (float)row * yStep };
            vertex->position = p;

            vertex->setColor(colors[column].withAlpha(shade[row]));
        });
#endif

    float scale = (float)juce::jmin(getWidth(), getHeight()) / 400.0f;
    transform = juce::AffineTransform::scale(scale * 0.8f).translated(50.0f, 50.0f);

    meshImage = juce::Image{ juce::Image::ARGB, getHeight() / 4, getHeight(), true };
    effectImage = juce::Image{ juce::Image::ARGB, getHeight() / 4, getHeight(), true };
}

juce::Path BottleDemo::splitPath(juce::Path const& p)
{
    juce::Path::Iterator it{ p };

    while (it.next())
    {
        switch (it.elementType)
        {
        case Path::Iterator::startNewSubPath:
        {
            break;
        }

        case Path::Iterator::lineTo:
        {
            DBG("lineTo " << it.x1 << ", " << it.y1);
            break;
        }

        case Path::Iterator::quadraticTo:
        {
            DBG("quadraticTo " << it.x1 << ", " << it.y1);
            break;
        }

        case Path::Iterator::cubicTo:
        {
            DBG("cubicTo " << it.x1 << ", " << it.y1 << "   " << it.x2 << ", " << it.y2 << "  " << it.x3 << ", " << it.y3);
            break;
        }

        case Path::Iterator::closePath:
        {
            break;
        }
        }
    }

    return juce::Path{ p };
}

void BottleDemo::LiquidFun::createWorld()
{
    auto snifterBowlCenter = juce::Point<float>{};
    for (auto const& p : snifterBowlPoints)
    {
        snifterBowlCenter += p;
    }

    snifterBowlCenter /= (float)snifterBowlPoints.size();
    auto flip = juce::AffineTransform::verticalFlip(snifterBowlCenter.getY() * 2.0f).translated(0.0f, -40.0f);

    auto it = snifterBowlPoints.begin();
    auto lastPoint = it->transformedBy(flip);;
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
        pd.flags = b2_viscousParticle;
        pd.position.Set(150.0f + random.nextFloat() * 10.0f - 5.0f, 150.0f + (float)i * 3.0f);
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
    Rectangle<float> boxWorldArea{ 0.0f, 0.0f, 300, 300.0f };
    renderer.render(g,
        world,
        boxWorldArea,
        juce::Rectangle<float>{ 0.0f, 0.0f, 300.0f, 300.0f }.transformedBy(transform));
}