#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <d2d1_3helper.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#include <JuceHeader.h>
#include <juce_graphics/native/juce_DirectX_windows.h>
#include <juce_graphics/native/juce_Direct2DImage_windows.h>
#include "Mesher.h"

int Mesher::Edge::uniqueIDCounter = 0;

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

static std::array<int, 20> const stockColors
{
    0xe6194b, 0x3cb44b, 0xffe119, 0x4363d8, 0xf58231,
    0x911eb4, 0x46f0f0, 0xf032e6, 0xbcf60c, 0xfabebe,
    0x008080, 0xe6beff, 0x9a6324, 0xfffac8, 0x800000,
    0xaaffc3, 0x808000, 0xffd8b1, 0x000075, 0x808080
};

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
//    updateMesh();
}

Mesher::~Mesher()
{
}

void Mesher::updateMesh(int numPatchEdges)
{
    subpaths.clear();

    juce::Path::Iterator it{ path };

    //
    // Find perimeter vertices
    //
    std::shared_ptr<Vertex> subpathStart;
    while (it.next())
    {
        //DBG(print(it));

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
    // Default vertex colors
    //
    int colorIndex = 0;
    for (auto const& subpath : subpaths)
    {
        for (auto const& vertex : subpath.vertices)
        {
            vertex->color = juce::Colour(stockColors[colorIndex] | 0xff000000);
            colorIndex = (colorIndex + 1) % stockColors.size();
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

        for (auto const& vertex : subpath.vertices)
        {
            std::sort(vertex->vertexConnectedEdges.begin(), vertex->vertexConnectedEdges.end(), [&](auto const& a, auto const& b)
                {
                    auto angleA = a.lock()->getAngle(vertex.get());
                    auto angleB = b.lock()->getAngle(vertex.get());

                    return angleA < angleB;
                });

            int edgeIndex = 0;
            for (auto& edge : vertex->vertexConnectedEdges)
            {
                DBG("connecting edge " << edge.lock()->uniqueID << " to vertex " << vertex->point.toString() << " at index " << edgeIndex << " of " << vertex->vertexConnectedEdges.size());
                auto& endpoint = edge.lock()->getEndpoint(vertex.get());
                endpoint.edgeIndex = edgeIndex++;
            }

            for (auto const& edge : vertex->vertexConnectedEdges)
            {
                edge.lock()->dump();
            }
        }
    }

    //
    // Add patches
    //
    for (auto& subpath : subpaths)
    {
        subpath.addPatches(center, numPatchEdges);
    }
}

void Mesher::draw(juce::Image image, juce::AffineTransform transform)
{
    auto toPoint2F = [&](juce::Point<float> p)
        {
            return D2D1::Point2F(p.x, p.y);
        };

    auto setD2DPatchVertices = [&](std::weak_ptr<Patch> patch, int edgeIndex, D2D1_POINT_2F& p0, D2D1_POINT_2F& p1, D2D1_COLOR_F& c0, D2D1_COLOR_F& c1)
        {
            if (auto patchLock = patch.lock())
            {
                if (auto edge = patchLock->edges[edgeIndex].lock())
                {
                    auto v0 = edge->endpoints[0].vertex.lock();
                    auto v1= edge->endpoints[1].vertex.lock();

                    if (v0 && v1)
                    {
                        p0 = toPoint2F(v0->point.toFloat());
                        p1 = toPoint2F(v1->point.toFloat());
                        c0 = juce::D2DUtilities::toCOLOR_F(v0->color);
                        c1 = juce::D2DUtilities::toCOLOR_F(v1->color);
                    }
                }
            }
        };

    auto setEdgeControlPoints = [&](std::weak_ptr<Patch> patch, int edgeIndex, D2D1_POINT_2F& p0, D2D1_POINT_2F& p1)
        {
            if (auto patchLock = patch.lock())
            {
                if (auto edge = patchLock->edges[edgeIndex].lock())
                {
                    if (edge->controlPoints[0].has_value())
                    {
                        p0 = toPoint2F(edge->controlPoints[0].value());
                    }

                    if (edge->controlPoints[1].has_value())
                    {
                        p1 = toPoint2F(edge->controlPoints[1].value());
                    }
                }
            }
        };

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
        for (auto const& patch : subpath.patches)
        {
            auto& d2dPatch = d2dPatches.emplace_back(D2D1_GRADIENT_MESH_PATCH{});

            setD2DPatchVertices(patch, 0, d2dPatch.point00, d2dPatch.point03, d2dPatch.color00, d2dPatch.color03);
            setD2DPatchVertices(patch, 2, d2dPatch.point33, d2dPatch.point30, d2dPatch.color33, d2dPatch.color30);

            d2dPatch.point01 = d2dPatch.point00;
            d2dPatch.point02 = d2dPatch.point03;

            d2dPatch.point10 = d2dPatch.point00;
            d2dPatch.point13 = d2dPatch.point03;

            d2dPatch.point20 = d2dPatch.point30;
            d2dPatch.point31 = d2dPatch.point30;

            d2dPatch.point23 = d2dPatch.point33;
            d2dPatch.point32 = d2dPatch.point33;

            d2dPatch.point11 = d2dPatch.point00;
            d2dPatch.point12 = d2dPatch.point03;
            d2dPatch.point21 = d2dPatch.point30;
            d2dPatch.point22 = d2dPatch.point33;

            setEdgeControlPoints(patch, 0, d2dPatch.point01, d2dPatch.point02);
            setEdgeControlPoints(patch, 1, d2dPatch.point13, d2dPatch.point23);
            setEdgeControlPoints(patch, 2, d2dPatch.point32, d2dPatch.point31);
            setEdgeControlPoints(patch, 3, d2dPatch.point20, d2dPatch.point10);
        }
    }

    if (pimpl->deviceContext && image.isValid())
    {
        pimpl->deviceContext->CreateGradientMesh(d2dPatches.data(), d2dPatches.size(), pimpl->gradientMesh.put());

        if (pimpl->gradientMesh)
        {
            if (auto pixelData = dynamic_cast<juce::Direct2DPixelData*>(image.getPixelData()))
            {
                if (auto bitmap = pixelData->getAdapterD2D1Bitmap())
                {
                    pimpl->deviceContext->SetTarget(bitmap);
                    pimpl->deviceContext->BeginDraw();
                    pimpl->deviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
                    pimpl->deviceContext->Clear({ 0.0f, 0.0f, 0.0f, 0.0f });

                    if (pimpl->gradientMesh)
                    {
                        pimpl->deviceContext->DrawGradientMesh(pimpl->gradientMesh.get());
                    }

                    pimpl->deviceContext->EndDraw();
                    pimpl->deviceContext->SetTarget(nullptr);
                }
            }
        }
    }
}

void Mesher::iterateSubpath(juce::Path::Iterator& it, std::shared_ptr<Vertex> subpathStart)
{
    auto& subpath = subpaths.emplace_back(Subpath{});

    auto bounds = path.getBounds();
    auto center = bounds.getCentre();

    std::shared_ptr<Vertex> previousVertex = subpathStart;
    subpath.vertices.emplace_back(subpathStart);
    //DBG("   subpath start vertex " << subpathStart->point.toString());

    while (it.next())
    {
        std::shared_ptr<Vertex> vertex;
        std::shared_ptr<Edge> edge;

        DBG("   " << print(it));

        auto edgeType = Edge::Type::unknown;
        std::array<std::optional<juce::Point<float>>, 2> controlPoints;
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
            edgeType = Edge::Type::line;
            break;
        }

        case Path::Iterator::quadraticTo:
        {
            vertex = std::make_shared<Vertex>(juce::Point<float>{ it.x2, it.y2 });
            edgeType = Edge::Type::quadratic;
            controlPoints = { juce::Point<float>{ it.x1, it.y1 }, juce::Point<float>{ it.x1, it.y1 } };
            break;
        }

        case Path::Iterator::cubicTo:
        {
            vertex = std::make_shared<Vertex>(juce::Point<float>{ it.x3, it.y3 });
            edgeType = Edge::Type::cubic;
            controlPoints = { juce::Point<float>{ it.x1, it.y1 }, juce::Point<float>{ it.x2, it.y2 } };
            break;
        }

        case Path::Iterator::closePath:
        {
            vertex = subpathStart;
            edgeType = Edge::Type::line;
            break;
        }
        }

        if (vertex)
        {
            if (approximatelyEqual(vertex->point.x, previousVertex->point.x) && approximatelyEqual(vertex->point.y, previousVertex->point.y))
            {
                return;
            }

            if (approximatelyEqual(vertex->point.x, subpathStart->point.x) && approximatelyEqual(vertex->point.y, subpathStart->point.y))
            {
                edge = std::make_shared<Edge>(Edge{ edgeType, previousVertex, subpathStart });
            }
            else
            {
                edge = std::make_shared<Edge>(Edge{ edgeType, previousVertex, vertex });
                subpath.vertices.emplace_back(vertex);
            }

            edge->controlPoints = controlPoints;

            vertex->vertexConnectedEdges.emplace_back(edge);
            previousVertex->vertexConnectedEdges.emplace_back(edge);
            subpath.edges.emplace_back(edge);
        }

        previousVertex = vertex;
    }
}

void Mesher::Subpath::addPatches(juce::Point<float> center, int numPatchEdges)
{
    auto centerVertex = std::make_shared<Vertex>(center);
    std::shared_ptr<Edge> newEdge;
    std::weak_ptr<Edge> previousEdge;

    auto perimeterVerticesEnd = vertices.begin() + vertices.size();
    auto perimeterEdgesEnd = edges.begin() + edges.size();

    for (auto vertexIndex = 0; vertexIndex < vertices.size(); vertexIndex += 1)
    {
        auto& perimeterVertex = vertices[vertexIndex];
        newEdge = std::make_shared<Edge>(Edge{ Edge::Type::line, perimeterVertex, centerVertex });
        perimeterVertex->vertexConnectedEdges.emplace_back(newEdge);
        centerVertex->vertexConnectedEdges.emplace_back(newEdge);
        edges.emplace_back(newEdge);
    }

    switch (numPatchEdges)
    {
    case 3:
    {
        for (size_t centerVertexEdgeIndex = 0; centerVertexEdgeIndex < centerVertex->vertexConnectedEdges.size(); ++centerVertexEdgeIndex)
        {
            auto centerVertexEdge0 = centerVertex->vertexConnectedEdges[centerVertexEdgeIndex];
            auto centerVertexEdge1 = centerVertex->vertexConnectedEdges[(centerVertexEdgeIndex + 1) % centerVertex->vertexConnectedEdges.size()];

            auto perimeterEdge = edges[centerVertexEdgeIndex];

            patches.emplace_back(std::make_shared<Patch>(Patch{ { perimeterEdge, perimeterEdge, centerVertexEdge1, centerVertexEdge0 } }));
        }
        break;
    }

    case 4:
    {
        for (size_t centerVertexEdgeIndex = 0; centerVertexEdgeIndex < centerVertex->vertexConnectedEdges.size(); ++centerVertexEdgeIndex)
        {
            auto centerVertexEdge0 = centerVertex->vertexConnectedEdges[centerVertexEdgeIndex];
            auto centerVertexEdge1 = centerVertex->vertexConnectedEdges[(centerVertexEdgeIndex + 1) % centerVertex->vertexConnectedEdges.size()];

            auto perimeterEdge0 = centerVertexEdge0.lock()->nextEdgeRightTurn(centerVertex.get());


            //auto& patch = patches.emplace_back(std::make_shared<Patch>(Patch{ { perimeterEdge0, perimeterEdge1, centerVertexEdge1, centerVertexEdge0 } }));
        }
        break;
    }

    default:
    {
        jassertfalse;
        break;
    }
    }

    vertices.emplace_back(centerVertex);
}

std::weak_ptr<Mesher::Edge> Mesher::Edge::nextEdgeRightTurn(Vertex const* const start)
{
    Endpoint& endpoint = (start == endpoints[0].vertex.lock().get()) ? endpoints[1] : endpoints[0];
    return endpoint.vertex.lock()->turnRight(endpoint.edgeIndex);
}

std::weak_ptr<Mesher::Edge> Mesher::Vertex::turnRight(int edgeIndex) const
{
    jassert(edgeIndex >= 0);
    edgeIndex = (edgeIndex - 1) % vertexConnectedEdges.size();
    return vertexConnectedEdges[edgeIndex];
}
