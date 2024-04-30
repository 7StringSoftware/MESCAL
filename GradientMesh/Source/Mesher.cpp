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

    juce::Point<float> previousPoint;
    juce::Point<float> point;
    juce::Point<float> subpathStart;
    Edge edge{ Edge::Type::unknown };
    auto storeVertices = [&]()
        {
            DBG(print(it));

            if (it.elementType == juce::Path::Iterator::startNewSubPath)
            {
                point = { it.x1, it.y1 };
                subpathStart = point;
                previousPoint = point;
                return;
            }

            if (it.elementType == Path::Iterator::lineTo)
            {
                point = { it.x1, it.y1 };
                edge = Edge{ Edge::Type::line };
                edge.line = { previousPoint, point };
            }
            else if (it.elementType == Path::Iterator::quadraticTo)
            {
                point = { it.x2, it.y2 };
                edge = Edge{ Edge::Type::quadratic };
                edge.line = { previousPoint, point };
            }
            else if (it.elementType == Path::Iterator::cubicTo)
            {
                point = { it.x3, it.y3 };
                edge = Edge{ Edge::Type::cubic };
                edge.line = { previousPoint, point };
                edge.controlPoints = { juce::Point<float>{ it.x1, it.y1 }, juce::Point<float>{ it.x2, it.y2 } };
            }
            else if (it.elementType == Path::Iterator::closePath)
            {
                point = subpathStart;
                edge = Edge{ Edge::Type::line };
                edge.line = { previousPoint, point };
            }
            else
            {
                return;
            }


            if (approximatelyEqual(point.x, previousPoint.x) && approximatelyEqual(point.y, previousPoint.y))
            {
                return;
            }

            DBG("     adding " << point.toString());
            vertices.add(Vertex{ point });
            edges.add(edge);

            previousPoint = point;
        };

    while (it.next())
    {
        storeVertices();
    }

    for (auto vertex : vertices)
    {
        edges.add({ Edge::Type::line, juce::Line<float>{ vertex.point, bounds.getCentre() } });
    }

    if (vertices.size() < 4)
    {
        return;
    }

    int numQuads = vertices.size() / 2;
    int vertexIndex = 0;
    for (int index = 0; index < numQuads; ++index)
    {
        quads.add(Quadrilateral{ { vertices[vertexIndex].point, vertices[vertexIndex + 1].point, vertices[vertexIndex + 2].point, bounds.getCentre()} });
        vertexIndex += 2;
    }

    if (vertices.size() & 1)
    {
        quads.add(Quadrilateral{ { vertices[vertices.size() - 1].point, vertices[0].point, bounds.getCentre(), bounds.getCentre()} });
    }

    vertices.add(Vertex{ bounds.getCentre() });
}

