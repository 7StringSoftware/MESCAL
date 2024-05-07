#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#include <JuceHeader.h>
#include <juce_graphics/native/juce_DirectX_windows.h>
#include <juce_graphics/native/juce_Direct2DImage_windows.h>
#include <d2d1_3helper.h>
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

struct Mesher::Pimpl
{
    Pimpl(Mesher& owner_) : owner(owner_)
    {
    }

    void createResources(juce::Image image)
    {
        if (!deviceContext)
        {
            if (auto pixelData = dynamic_cast<juce::Direct2DPixelData*>(image.getPixelData()))
            {
                if (auto adapter = pixelData->getAdapter())
                {
                    winrt::com_ptr<ID2D1DeviceContext1> deviceContext1;
                    if (const auto hr = adapter->direct2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
                        deviceContext1.put());
                        FAILED(hr))
                    {
                        jassertfalse;
                        return;
                    }

                    deviceContext = deviceContext1.as<ID2D1DeviceContext2>();
                }
            }
        }
    }

    Mesher& owner;
    winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
    winrt::com_ptr<ID2D1GradientMesh> gradientMesh;
};

Mesher::Mesher(Path&& p) :
    pimpl(std::make_unique<Pimpl>(*this)),
    path(p)
{
    juce::Path::Iterator it{ p };

    //
    // Find perimeter vertices
    //
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

    //
    // Sort perimeter vertices by clockwise angle from center
    //
    auto center = path.getBounds().getCentre();
    for (auto& subpath : subpaths)
    {
        std::sort(subpath.vertices.begin(), subpath.vertices.end(), [&](auto const& a, auto const& b)
            {
                auto angleA = center.getAngleToPoint(a->point);
                auto angleB = center.getAngleToPoint(b->point);
                return angleA < angleB;
            });

        std::sort(subpath.edges.begin(), subpath.edges.end(), [&](auto const& a, auto const& b)
            {
                auto angleA = center.getAngleToPoint(a->vertices[0].lock()->point);
                auto angleB = center.getAngleToPoint(b->vertices[0].lock()->point);
                return angleA < angleB;
            });
    }

    //
    // Add patches
    //
    for (auto& subpath : subpaths)
    {
        for (auto const& v : subpath.vertices)
        {
            DBG("vertex " << v->point.toString());
        }
        subpath.addPatches(center);
    }
}

Mesher::~Mesher()
{
}

void Mesher::draw(juce::Image image, juce::AffineTransform transform)
{
    pimpl->createResources(image);

    size_t numPatches = 0;
    for (auto const& subpath : subpaths)
    {
        numPatches += subpath.patches.size();
    }

    std::vector<D2D1_GRADIENT_MESH_PATCH> d2dPatches;
    d2dPatches.reserve(numPatches);
    winrt::com_ptr<ID2D1GradientMesh> gradientMesh;

    for (auto const& subpath : subpaths)
    {
#if 0
        for (auto const& quad : subpath.quads)
        {
            D2D1_GRADIENT_MESH_PATCH d2dPatch;
            d2dPatches.emplace_back(d2dPatch);
        }
#endif
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
    while (it.next() || !closed)
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
            edge = std::make_shared<Edge>(Edge{Edge::Type::line, {previousVertex, subpathStart} });
            closed = true;
            break;
        }
        }

        if (vertex)
        {
            if (approximatelyEqual(vertex->point.x, previousVertex->point.x) && approximatelyEqual(vertex->point.y, previousVertex->point.y))
            {
                return;
            }

            vertex->edges.emplace_back(edge);
            subpath.vertices.emplace_back(vertex);
        }

        if (edge)
        {
            previousVertex->edges.emplace_back(edge);
            subpath.edges.emplace_back(edge);
        }

        previousVertex = vertex;
        vertex = {};
        edge = {};
    }
}

void Mesher::Subpath::addPatches(juce::Point<float> center)
{
    auto centerVertex = std::make_shared<Vertex>(center);
    std::shared_ptr<Edge> newEdge;
    std::weak_ptr<Edge> previousEdge;

    auto addPatch = [&](std::weak_ptr<Vertex> perimeterVertex1, std::weak_ptr<Vertex> perimeterVertex2)
        {
            if (auto previousEdgeLock = previousEdge.lock())
            {
                DBG("addPatch " << perimeterVertex1.lock()->point.toString() << " -> " << perimeterVertex2.lock()->point.toString());

                auto newPatch = std::make_shared<Patch>(Patch{ { perimeterVertex1.lock()->edges.front(), perimeterVertex2.lock()->edges.front(), newEdge, previousEdgeLock } });
                patches.emplace_back(newPatch);
            }
        };

    auto lastPerimeterVertex = vertices.begin() + vertices.size();
    auto lastPerimeterEdge = edges.begin() + edges.size();
    for (auto it = vertices.begin(); it != lastPerimeterVertex; it += 2)
    {
        auto perimeterVertex = *it;
        newEdge = std::make_shared<Edge>(Edge{ Edge::Type::line, { perimeterVertex, centerVertex } });
        perimeterVertex->edges.emplace_back(newEdge);
        centerVertex->edges.emplace_back(newEdge);
        edges.emplace_back(newEdge);
    }

    for (size_t centerVertexEdgeIndex = 0; centerVertexEdgeIndex < centerVertex->edges.size() - 1; ++centerVertexEdgeIndex)
    {
        auto centerVertexEdge0 = centerVertex->edges[centerVertexEdgeIndex];
        auto centerVertexEdge1 = centerVertex->edges[centerVertexEdgeIndex + 1];


    }

    vertices.emplace_back(centerVertex);
}
