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

static bool isInterioPoint(juce::Path const& path, juce::Point<float> const& point)
{
    auto bounds = path.getBounds();
    auto intersection = juce::Point<float>{ bounds.getCentre().x, bounds.getCentre().y - 1000.0f };
    auto line = juce::Line<float>{ point, intersection };

    int intersections = 0;
    juce::Path::Iterator it{ path };
    while (it.next())
    {
        if (it.elementType == Path::Iterator::lineTo)
        {
            auto edge = juce::Line<float>{ it.x1, it.y1, it.x2, it.y2 };
            juce::Point<float> intersection;
            if (edge.intersects(line, intersection))
            {
                ++intersections;
            }
        }
        else if (it.elementType == Path::Iterator::quadraticTo)
        {
            auto edge = juce::Line<float>{ it.x1, it.y1, it.x2, it.y2 };
            juce::Point<float> intersection;
            if (edge.intersects(line, intersection))
            {
                ++intersections;
            }
        }
        else if (it.elementType == Path::Iterator::cubicTo)
        {
            auto edge = juce::Line<float>{ it.x1, it.y1, it.x3, it.y3 };
            juce::Point<float> intersection;
            if (edge.intersects(line, intersection))
            {
                ++intersections;
            }
        }
    }

    return intersections & 1;
}

Mesher::Mesher(Path&& p) :
    path(p)
{
    juce::Path::Iterator it{ p };

    auto bounds = p.getBounds();
    auto center = bounds.getCentre();

    juce::Point<float> previousPoint;
    juce::Point<float> point;
    juce::Point<float> subpathStart;
    Edge edge{ Edge::Type::unknown };
    juce::Array<Vertex> vertices;
    juce::Array<Edge> edges;

    auto isPerimeterPoint = [&](juce::Point<float> const point) -> bool
        {
            juce::Line<float> line{ center, point };
            bool contained = path.contains(point);
            bool fartherPointContained = path.contains(line.getPointAlongLine(line.getLength() + 1.0f));
            return ! contained || (contained && !fartherPointContained);
        };

    auto storeEdge = [&](Edge const& edge)
        {
            for (auto const& storedEdge : edges)
            {
                juce::Point<float> intersection;
                if (storedEdge.line.intersects(edge.line, intersection))
                {
                    vertices.add(Vertex{ intersection });
                    edges.add({ Edge::Type::line, juce::Line<float>{ edge.line.getStart(), intersection} });
                    edges.add({ Edge::Type::line, juce::Line<float>{ intersection, edge.line.getEnd() } });
                    return;
                }
            }

            edges.add(edge);
        };

    auto storeVertex = [&]()
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

            vertices.add(Vertex{ point });
            storeEdge(edge);

            previousPoint = point;
        };

    while (it.next())
    {
        storeVertex();
    }

    if (vertices.size() < 4)
    {
        return;
    }

    {
	    for (auto const& vertex : vertices)
	    {
	        if (isPerimeterPoint(vertex.point))
	        {
                perimeterVertices.addIfNotAlreadyThere(vertex);
	        }
	    }

        std::sort(perimeterVertices.begin(), perimeterVertices.end(), [&](Vertex const& a, Vertex const& b) -> bool
            {
                return center.getAngleToPoint(a.point) < center.getAngleToPoint(b.point);
            });
    }

    {
        auto it = perimeterVertices.begin();
        auto previousPoint = it->point;
        it++;

        while (it != perimeterVertices.end())
        {
            perimeterEdges.add({ Edge::Type::line, juce::Line<float>{ previousPoint, it->point } });
            previousPoint = it->point;
            it++;
        }
    }

#if 0
    int numQuads = vertices.size() / 2;
    int vertexIndex = 0;
    for (int index = 0; index < numQuads; ++index)
    {
        quads.add(Quadrilateral{ { vertices[vertexIndex].point, vertices[vertexIndex + 1].point, vertices[vertexIndex + 2].point, center } });
        vertexIndex += 2;
    }

    if (vertices.size() & 1)
    {
        quads.add(Quadrilateral{ { vertices[vertices.size() - 1].point, vertices[0].point, center, center } });
    }

    vertices.add(Vertex{ bounds.getCentre() });
#endif
}

