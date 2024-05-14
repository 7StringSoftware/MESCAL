#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <d2d1_3helper.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#include <JuceHeader.h>
#include <juce_graphics/native/juce_DirectX_windows.h>
#include <juce_graphics/native/juce_Direct2DImage_windows.h>
#include "HalfEdgeMesh.h"

HalfEdgeMesh::HalfEdgeMesh(Path&& p) :
    path(p)
{

}

HalfEdgeMesh::~HalfEdgeMesh()
{

}

void HalfEdgeMesh::updateMesh(int numPatchEdges /*= 4*/)
{
    subpaths.clear();

    juce::Path::Iterator it{ path };

    //
    // Find perimeter vertices
    //
    Vertex subpathStart;
    while (it.next())
    {
        //DBG(print(it));

        switch (it.elementType)
        {
        case Path::Iterator::startNewSubPath:
            iterateSubpath(it, juce::Point<float>{ it.x1, it.y1 });
            break;

        case Path::Iterator::lineTo:
        case Path::Iterator::cubicTo:
        case Path::Iterator::quadraticTo:
            iterateSubpath(it, juce::Point<float>{ it.x1, it.y1 });
            break;

        default:
            break;
        }
    }
}

void HalfEdgeMesh::iterateSubpath(juce::Path::Iterator& it, Point<float> subpathStart)
{
    auto& subpath = subpaths.emplace_back(Subpath{});

    auto bounds = path.getBounds();
    auto center = bounds.getCentre();

    struct PathPoint
    {
        juce::Point<float> point;
        Halfedge::Type type = Halfedge::Type::unknown;
        std::pair<juce::Point<float>, juce::Point<float>> controlPoints;
    };
    std::vector<PathPoint> pathPoints;

    while (it.next())
    {
        auto edgeType = Halfedge::Type::unknown;
        switch (it.elementType)
        {
        case Path::Iterator::startNewSubPath:
        {
            iterateSubpath(it, juce::Point<float>{ it.x1, it.y1 });
            return;
        }

        case Path::Iterator::lineTo:
        {
            pathPoints.emplace_back(PathPoint{ juce::Point<float>{ it.x1, it.y1 }, Halfedge::Type::line });
            break;
        }

        case Path::Iterator::quadraticTo:
        {
            pathPoints.emplace_back(PathPoint{ juce::Point<float>{ it.x2, it.y2 },
                Halfedge::Type::quadratic,
                { juce::Point<float>{ it.x1, it.y1 }, juce::Point<float>{ it.x1, it.y1 } } });
            break;
        }

        case Path::Iterator::cubicTo:
        {
            pathPoints.emplace_back(PathPoint{ juce::Point<float>{ it.x3, it.y3 },
                Halfedge::Type::cubic,
                { juce::Point<float>{ it.x1, it.y1 }, juce::Point<float>{ it.x2, it.y2 } } });
            break;
        }

        case Path::Iterator::closePath:
        {
            pathPoints.emplace_back(PathPoint{ subpathStart, Halfedge::Type::line });
            break;
        }
        }
    }

    auto addHalfedge = [&](Vertex* tailVertex, Vertex* headVertex, Halfedge::Type type)
    {
        auto halfedge = std::make_unique<Halfedge>();
        halfedge->type = type;
        halfedge->tailVertex = tailVertex;
        halfedge->headVertex = headVertex;

        auto twin = std::make_unique<Halfedge>();
        twin->type = halfedge->type;
        twin->tailVertex = halfedge->headVertex;
        twin->headVertex = halfedge->tailVertex;

        halfedge->twin = twin.get();
        twin->twin = halfedge.get();

        subpath.halfedges.push_back(std::move(halfedge));
        subpath.halfedges.push_back(std::move(twin));
    };

    Vertex* previousVertex = subpath.vertices.emplace_back(std::make_unique<Vertex>(pathPoints.front().point)).get();
    for (auto pathPointIterator = pathPoints.begin() + 1; pathPointIterator != pathPoints.end(); ++pathPointIterator)
    {
        auto const& pathPoint = *pathPointIterator;
        auto& vertex = subpath.vertices.emplace_back(std::make_unique<Vertex>(pathPoint.point));

        addHalfedge(previousVertex, vertex.get(), pathPoint.type);
        vertex->halfedge = subpath.halfedges.back().get();

        previousVertex = vertex.get();
    }

    addHalfedge(previousVertex, subpath.vertices.front().get(), pathPoints.back().type);

    auto centerVertex = std::make_unique<Vertex>(center);
    for (auto& vertex : subpath.vertices)
    {
        addHalfedge(centerVertex.get(), vertex.get(), Halfedge::Type::line);
    }
    centerVertex->halfedge = subpath.halfedges.back().get();
    subpath.vertices.push_back(std::move(centerVertex));

    subpath.vertices.front()->halfedge = subpath.halfedges.back().get();

    for (auto& halfedge : subpath.halfedges)
    {
        halfedge->dump();
    }

    for (auto const& vertex : subpath.vertices)
    {
        vertex->dump();
    }
}
