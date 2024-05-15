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

void HalfEdgeMesh::Vertex::dump() const
{
    DBG("Vertex " << point.toString() << "  halfedge:" << (halfedge ? halfedge->print() : "null"));
}

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

    auto addHalfedge = [&](Vertex* tailVertex, Vertex* headVertex, Halfedge::Type type) -> Halfedge*
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

            subpath.halfedges.push_back(std::move(twin));
            subpath.halfedges.push_back(std::move(halfedge));

            return subpath.halfedges.back().get();
        };

    for (const auto& pathPoint : pathPoints)
    {
        subpath.vertices.emplace_back(std::make_unique<Vertex>(pathPoint.point));
    }

    std::vector<Halfedge*> perimeterHalfedges;
    for (auto index = 0; index < pathPoints.size(); ++index)
    {
        auto& vertex = subpath.vertices[index];
        auto& nextVertex = subpath.vertices[(index + 1) % pathPoints.size()];
        auto perimeterHalfedge = addHalfedge(vertex.get(), nextVertex.get(), pathPoints[index].type);
        vertex->halfedge = perimeterHalfedge;
        perimeterHalfedges.push_back(perimeterHalfedge);
    }

    auto centerVertex = std::make_unique<Vertex>(center);
    std::vector<Halfedge*> centerHalfedges;
    for (size_t index = 0; index < perimeterHalfedges.size(); ++index)
    {
        auto centerHalfedge = addHalfedge(perimeterHalfedges[index]->tailVertex, centerVertex.get(), Halfedge::Type::line);
        centerHalfedges.push_back(centerHalfedge);

        auto& perimeterHalfedge = perimeterHalfedges[index];
        auto& previousPerimeterHalfedge = perimeterHalfedges[(index + perimeterHalfedges.size() - 1) % perimeterHalfedges.size()];
        perimeterHalfedge->next = centerHalfedge;
        perimeterHalfedge->previous = previousPerimeterHalfedge->twin;

        centerHalfedge->next = perimeterHalfedge;
        centerHalfedge->previous = previousPerimeterHalfedge->twin;

        previousPerimeterHalfedge->twin->previous = centerHalfedge;
        previousPerimeterHalfedge->twin->next = perimeterHalfedge;
    }

    for (size_t index = 0; index < centerHalfedges.size(); ++index)
    {
        size_t nextIndex = (index + 1) % centerHalfedges.size();
        size_t previousIndex = (index + centerHalfedges.size() - 1) % centerHalfedges.size();
        auto& centerHalfedge = centerHalfedges[index]->twin;
        centerHalfedge->next = centerHalfedges[nextIndex]->twin;
        centerHalfedge->previous = centerHalfedges[previousIndex]->twin;
    }

    centerVertex->halfedge = subpath.halfedges.back()->twin;
    subpath.vertices.push_back(std::move(centerVertex));
}
