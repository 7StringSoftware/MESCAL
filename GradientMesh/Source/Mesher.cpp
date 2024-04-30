#include <JuceHeader.h>
#include "Mesher.h"

static juce::String print(juce::Path::Iterator const& it)
{
    juce::String line;

    if (it.elementType == Path::Iterator::startNewSubPath)
    {
        line << "startNewSubPath " << it.x1 << ", " << it.y1;
    }
    else if (it.elementType == Path::Iterator::lineTo)
    {
        line << "lineTo " << it.x1 << ", " << it.y1;
    }
    else if (it.elementType == Path::Iterator::quadraticTo)
    {
        line << "quadraticTo " << it.x1 << ", " << it.y1 << " " << it.x2 << ", " << it.y2;
    }
    else if (it.elementType == Path::Iterator::cubicTo)
    {
        line << "cubicTo " << it.x1 << ", " << it.y1 << " " << it.x2 << ", " << it.y2 << " " << it.x3 << ", " << it.y3;
    }
    else if (it.elementType == Path::Iterator::closePath)
    {
        line << "closePath";
    }   

    return line;
}

Mesher::Mesher(Path&& p) :
    path(p)
{
    juce::Path::Iterator it{ p };

    auto bounds = p.getBounds();

    juce::Point<float> point;
    juce::Point<float> subpathStart;
    auto storeVertices = [&]()
        {
            DBG(print(it));

            Vertex::Type type = Vertex::Type::unknown;
            if (it.elementType == juce::Path::Iterator::startNewSubPath)
            {
                point = { it.x1, it.y1 };
                type = Vertex::Type::start;
                subpathStart = point;
            }
            else if (it.elementType == Path::Iterator::lineTo)
            {
                point = { it.x1, it.y1 };
                type = Vertex::Type::line;
            }
            else if (it.elementType == Path::Iterator::quadraticTo)
            {
                point = { it.x2, it.y2 };
                type = Vertex::Type::quadratic;
            }
            else if (it.elementType == Path::Iterator::cubicTo)
            {
                point = { it.x3, it.y3 };
                type = Vertex::Type::cubic;
            }
            else if (it.elementType == Path::Iterator::closePath)
            {
                point = subpathStart;
                type = Vertex::Type::close;

                if (approximatelyEqual(point.x, subpathStart.x) && approximatelyEqual(point.y, subpathStart.y))
                {
                    return;
                }
            }
            else
            {
                return;
            }

            perimeterVertices.add(Vertex(type, point, bounds));
            xySortedVertices.add(Vertex(type, point, bounds));
            yPositions.add(point.y);
            xPositions.add(point.x);
        };

    while (it.next())
    {
        storeVertices();
    }

    auto lastPoint = perimeterVertices.getReference(perimeterVertices.size() - 1).point;
    auto firstPoint = perimeterVertices.getReference(0).point;
    float lastAngle = lastPoint.getAngleToPoint(firstPoint);
    lastPoint = firstPoint;

    for (auto index = 1; index < perimeterVertices.size(); ++index)
    {
        DBG("#" << index << " " << perimeterVertices.getReference(index).point.toString());

        auto const& vertex = perimeterVertices.getReference(index);
        auto angle = lastPoint.getAngleToPoint(vertex.point);
        auto deltaRadians = std::abs(angle - lastAngle);
        DBG("angle " << angle / juce::MathConstants<float>::twoPi << " lastAngle " << lastAngle / juce::MathConstants<float>::twoPi << " delta " << deltaRadians);
        if (deltaRadians > juce::MathConstants<float>::halfPi * 0.99f)
        {
            DBG("new patch for " << lastPoint.toString());
        }

        DBG("\n");

        lastPoint = vertex.point;
        lastAngle = angle;
    }
}

