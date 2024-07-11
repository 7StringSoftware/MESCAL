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
    auto drawable = juce::Drawable::createFromImageData(BinaryData::Wine_bottle_svg, BinaryData::Wine_bottle_svgSize);
    originalBottlePath = drawable->getOutlineAsPath();

    juce::Range<float> yRange = { 115.0f, 145.0f };
    juce::Range<float> topEdgeXRange = { 30.0f, 70.0f };
    juce::Range<float> bottomEdgeXRange = { 0.0f, 100.0f };

    int numRows = 6;
    int numColumns = 8;

    mesh = std::make_unique<mescal::GradientMesh>(numRows, numColumns);

    //
    //
    //
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

    std::array<juce::Line<float>, 6> const lines
    {
        juce::Line<float>{ 40.0f, 0.0f, 60.0f, 0.0f},
        { 30.0f, 5.0f, 70.0f, 5.0f },
        { 30.0f, 115.0f, 70.0f, 115.0f },
        { 0.0f, 145.0f, 100.0f, 145.0f },
        { 0.0f, 400.0f, 100.0f, 400.0f },
        { 20.0f, 410.0f, 80.0f, 410.0f }
    };

    for (int row = 0; row < numRows; row += 2)
    {
        auto const& topLine = lines[row];
        auto const& bottomLine = lines[row + 1];

        for (int column = 0; column < numColumns; ++column)
        {
            auto proportionalX = (float)column / (float)(numColumns - 1);
            auto x = std::cos(proportionalX * juce::MathConstants<float>::pi) * 0.5f + 0.5f;
            auto bottomPoint = bottomLine.getPointAlongLineProportionally(x);
            auto topPoint = topLine.getPointAlongLineProportionally(x);

            auto grey = std::sin(x * juce::MathConstants<float>::pi);
            auto topColor = juce::Colour::greyLevel(grey * 0.5f);
            auto bottomColor = topColor;

            if (row == 2 && column >= 32 && column <= 48)
            {
                topColor = juce::Colours::red;
                bottomColor = juce::Colours::darkred;
            }

            mesh->configureVertex(row, column, topPoint, topColor);
            mesh->configureVertex(row + 1, column, bottomPoint, bottomColor);

            float height = bottomPoint.getY() - topPoint.getY();
            auto c1 = juce::Point<float>{ bottomPoint.getX(), bottomPoint.getY() - height * 0.55f };
            auto c2 = juce::Point<float>{ topPoint.getX(), topPoint.getY() };
            if (approximatelyEqual(c1.x, c2.x))
            {
                c1 = bottomPoint;
                c2 = topPoint;
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
        
    bottleBodyTopPath = p;


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

    g.setColour(juce::Colours::black);
    g.strokePath(bottleBodyTopPath, juce::PathStrokeType{ 1.0f });

    mesh->draw(meshImage, pathTransform, juce::Colours::white);
    g.drawImageAt(meshImage, 0, 0);

#if 0
    for (auto const& halfedge : mesh->getHalfedges())
    {
        juce::Line<float> line{ halfedge->tail.lock()->position, halfedge->head.lock()->position };
        line = { line.getStart().transformedBy(pathTransform), line.getEnd().transformedBy(pathTransform) };
        g.setColour(juce::Colours::purple);
        g.drawArrow(line.withShortenedStart(5.0f).withShortenedEnd(5.0f), 2.0f, 12.0f, 12.0f);
    }
#endif

#if 1
    for (auto const& vertex : mesh->getVertices())
    {
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
                line = { line.getStart().transformedBy(pathTransform), line.getEnd().transformedBy(pathTransform) };

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
    float scale = juce::jmin((float)getWidth() / originalBottlePath.getBounds().getWidth(), (float)getHeight() / originalBottlePath.getBounds().getHeight());
    pathTransform = juce::AffineTransform::scale(scale * 0.9f).translated(0.0f, 10.0f);
    transformedBottlePath = originalBottlePath;
    transformedBottlePath.applyTransform(pathTransform);

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

    meshImage = juce::Image{ juce::Image::ARGB, (int)transformedBottlePath.getBounds().getWidth(), (int)transformedBottlePath.getBounds().getHeight(), true};
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
