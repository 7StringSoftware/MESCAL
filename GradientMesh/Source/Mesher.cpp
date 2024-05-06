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

    std::shared_ptr<Vertex> subpathStart;
    while (it.next())
    {
        DBG(print(it));

        switch (it.elementType)
        {
        case Path::Iterator::startNewSubPath:
            subpathStart = std::make_shared<Vertex>(juce::Point<float>{ it.x1, it.y1 });
            iterateSubpath(it, subpathStart);
            break;

        case Path::Iterator::lineTo:
        case Path::Iterator::cubicTo:
        case Path::Iterator::quadraticTo:
            subpathStart = std::make_shared<Vertex>(juce::Point<float>{ it.x1, it.y1 });
            iterateSubpath(it, subpathStart);
            break;

        default:
            break;
        }
    }

    auto center = path.getBounds().getCentre();
    for (auto& subpath : subpaths)
    {
        for (int edgeIndex = 0; edgeIndex < subpath.edges.size(); edgeIndex += 2)
        {
            auto const& firstEdge = subpath.edges[edgeIndex];
            auto const& secondEdge = subpath.edges[(edgeIndex + 1) % subpath.edges.size()];
            subpath.quads.emplace_back(PatchBoundary{ { firstEdge->vertices[0]->point, firstEdge->vertices[1]->point, secondEdge->vertices[1]->point, center } });
        }
    }
}

void Mesher::iterateSubpath(juce::Path::Iterator& it, std::shared_ptr<Vertex> subpathStart)
{
    auto& subpath = subpaths.emplace_back(Subpath{});

    auto bounds = path.getBounds();
    auto center = bounds.getCentre();

    std::shared_ptr<Vertex> previousVertex = subpathStart;
    std::shared_ptr<Vertex> vertex;
    std::shared_ptr<Edge> edge;

    subpath.vertices.emplace_back(subpathStart);

    bool closed = false;
    while (it.next() || closed)
    {
        DBG("   " << print(it));

        switch (it.elementType)
        {
        case Path::Iterator::startNewSubPath:
        {
            subpathStart = std::make_shared<Vertex>(juce::Point<float>{ it.x1, it.y1 });
            iterateSubpath(it, subpathStart);
            return;
        }

        case Path::Iterator::lineTo:
        {
            vertex = std::make_shared<Vertex>(juce::Point<float>{ it.x1, it.y1 });
            edge = std::make_shared<Edge>(Edge{ Edge::Type::line, { previousVertex, vertex } });
            break;
        }

        case Path::Iterator::quadraticTo:
        {
            vertex = std::make_shared<Vertex>(juce::Point<float>{ it.x2, it.y2 });
            edge = std::make_shared<Edge>(Edge{ Edge::Type::line, { previousVertex, vertex } });
            edge->controlPoints = { juce::Point<float>{ it.x1, it.y1 }, juce::Point<float>{ it.x1, it.y1 } };
            break;
        }

        case Path::Iterator::cubicTo:
        {
            vertex = std::make_shared<Vertex>(juce::Point<float>{ it.x3, it.y3 });
            edge = std::make_shared<Edge>(Edge{ Edge::Type::line, { previousVertex, vertex } });
            edge->controlPoints = { juce::Point<float>{ it.x1, it.y1 }, juce::Point<float>{ it.x2, it.y2 } };
            break;
        }

        case Path::Iterator::closePath:
        {
            vertex = subpathStart;
            edge = std::make_shared<Edge>(Edge{Edge::Type::line, {previousVertex, vertex} });
            closed = true;
            break;
        }
        }

        if (approximatelyEqual(vertex->point.x, previousVertex->point.x) && approximatelyEqual(vertex->point.y, previousVertex->point.y))
        {
            return;
        }

        subpath.vertices.emplace_back(vertex);
        subpath.edges.emplace_back(edge);
        previousVertex = vertex;
        vertex = {};
        edge = {};
    }
}