#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <d2d1_3helper.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#include <JuceHeader.h>
#include <juce_graphics/native/juce_DirectX_windows.h>
#include <juce_graphics/native/juce_Direct2DImage_windows.h>
#include "mescal_GradientMesh_windows.h"
#define USINGZ 1
#include "../Clipper2Lib/include/clipper.h"

struct GradientMesh::Pimpl
{
    Pimpl(GradientMesh& owner_) : owner(owner_)
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

    static void pathToGeometrySink(const Path& path, ID2D1GeometrySink* sink, const AffineTransform& transform, D2D1_FIGURE_BEGIN figureMode)
    {
        // Every call to BeginFigure must have a matching call to EndFigure. But - the Path does not necessarily
        // have matching startNewSubPath and closePath markers. The figureStarted flag indicates if an extra call
        // to BeginFigure or EndFigure is needed during the iteration loop or when exiting this function.
        Path::Iterator it(path);
        bool figureStarted = false;

        while (it.next())
        {
            switch (it.elementType)
            {
            case Path::Iterator::cubicTo:
            {
                if (!figureStarted)
                    break;

                transform.transformPoint(it.x1, it.y1);
                transform.transformPoint(it.x2, it.y2);
                transform.transformPoint(it.x3, it.y3);

                sink->AddBezier({ { it.x1, it.y1 }, { it.x2, it.y2 }, { it.x3, it.y3 } });
                break;
            }

            case Path::Iterator::lineTo:
            {
                if (!figureStarted)
                    break;

                transform.transformPoint(it.x1, it.y1);
                sink->AddLine({ it.x1, it.y1 });
                break;
            }

            case Path::Iterator::quadraticTo:
            {
                if (!figureStarted)
                    break;

                transform.transformPoint(it.x1, it.y1);
                transform.transformPoint(it.x2, it.y2);
                sink->AddQuadraticBezier({ { it.x1, it.y1 }, { it.x2, it.y2 } });
                break;
            }

            case Path::Iterator::closePath:
            {
                if (figureStarted)
                {
                    sink->EndFigure(D2D1_FIGURE_END_CLOSED);
                    figureStarted = false;
                }
                break;
            }

            case Path::Iterator::startNewSubPath:
            {
                if (figureStarted)
                    sink->EndFigure(D2D1_FIGURE_END_OPEN);

                transform.transformPoint(it.x1, it.y1);
                sink->BeginFigure({ it.x1, it.y1 }, figureMode);

                figureStarted = true;
                break;
            }
            }
        }

        if (figureStarted)
        {
            sink->EndFigure(D2D1_FIGURE_END_OPEN);
        }
    }

    static winrt::com_ptr<ID2D1Geometry> pathToGeometry(juce::Path const& path, juce::AffineTransform const& transform)
    {
        winrt::com_ptr<ID2D1PathGeometry1> geometry;
        juce::SharedResourcePointer<juce::DirectX> directX;

        if (auto hr = directX->getD2DFactory()->CreatePathGeometry(geometry.put()); FAILED(hr))
        {
            return {};
        }

        winrt::com_ptr<ID2D1GeometrySink> sink;
        if (auto hr = geometry->Open(sink.put()); FAILED(hr))
        {
            return {};
        }

        pathToGeometrySink(path, sink.get(), transform, D2D1_FIGURE_BEGIN_FILLED);

        sink->Close();

        return geometry;
    }

    GradientMesh& owner;
    winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
    winrt::com_ptr<ID2D1GradientMesh> gradientMesh;
};

GradientMesh::GradientMesh() :
    pimpl(std::make_unique<Pimpl>(*this))
{
}

GradientMesh::~GradientMesh()
{
    for (auto& patch : patches)
    {
        patch->release();
    }

    for (auto& halfedge : halfedges)
    {
        halfedge->release();
    }

    for (auto& bezier : bezierControlPoints)
    {
        bezier.reset();
    }

    for (auto& vertex : vertices)
    {
        vertex.reset();
    }
}

void GradientMesh::addPatch(juce::Rectangle<float> bounds)
{
    std::array<std::shared_ptr<Halfedge>, 4> patchHalfedges;

    std::array<juce::Point<float>, 4> const boundsCorners
    {
        bounds.getTopLeft(),
        bounds.getTopRight(),
        bounds.getBottomRight(),
        bounds.getBottomLeft()
    };

    auto addBezierPointsAndHalfedges = [&](std::shared_ptr<Vertex> tail, std::shared_ptr<Vertex> head, Direction direction)
        {
            juce::Line<float> line{ tail->position, head->position };
            auto angle = line.getAngle();
            auto offset = line.getLength() * 0.1f;
            auto cp0 = line.getPointAlongLineProportionally(0.25f).getPointOnCircumference(offset, angle + juce::MathConstants<float>::halfPi);
            auto cp1 = line.getPointAlongLineProportionally(0.75f).getPointOnCircumference(offset, angle - juce::MathConstants<float>::halfPi);
            auto b0 = std::make_shared<BezierControlPoint>(cp0, *this);
            auto b1 = std::make_shared<BezierControlPoint>(cp1, *this);

            bezierControlPoints.push_back(b0);
            bezierControlPoints.push_back(b1);

            auto halfedge = addHalfedge(tail, head, b0, b1, direction);

            return halfedge;
        };

    auto topLeftVertex = addVertex(bounds.getTopLeft());
    auto topRightVertex = addVertex(bounds.getTopRight());
    auto bottomRightVertex = addVertex(bounds.getBottomRight());
    auto bottomLeftVertex = addVertex(bounds.getBottomLeft());

    patchHalfedges[(int)Direction::north] = addBezierPointsAndHalfedges(topLeftVertex, topRightVertex, { Direction::north });
    patchHalfedges[(int)Direction::east] = addBezierPointsAndHalfedges(topRightVertex, bottomRightVertex, { Direction::east });
    patchHalfedges[(int)Direction::south] = addBezierPointsAndHalfedges(bottomRightVertex, bottomLeftVertex, { Direction::south });
    patchHalfedges[(int)Direction::west] = addBezierPointsAndHalfedges(bottomLeftVertex, topLeftVertex, { Direction::west });

    auto patch = std::make_shared<Patch>(patchHalfedges);

    for (auto& halfedge : patchHalfedges)
    {
        halfedge->patch = patch;
    }

    patch->update();
    patches.push_back(patch);

    check();
}

void GradientMesh::addPatch(std::shared_ptr<Patch> patch)
{
    patch->update();
    patches.push_back(patch);
}

void GradientMesh::removePatch(Patch* patch)
{
    for (auto& halfedgeWeakPtr : patch->getHalfedges())
    {
        if (auto halfedge = halfedgeWeakPtr.lock())
        {
            if (auto twin = halfedge->twin.lock())
            {
                if (!twin->patch.lock())
                {
                    removeHalfedge(halfedge);
                    removeHalfedge(twin);
                }
            }
        }
    }

    auto it = std::find_if(patches.begin(), patches.end(), [patch](std::shared_ptr<Patch> p) { return p.get() == patch; });
    if (it != patches.end())
    {
        patches.erase(it);
    }
}

void GradientMesh::Vertex::removeHalfedge(std::shared_ptr<Halfedge> halfedge)
{
    for (auto& storedHalfedge : halfedges)
    {
        if (storedHalfedge.lock().get() == halfedge.get())
        {
            storedHalfedge = {};
        }
    }
}

void GradientMesh::removeHalfedge(std::shared_ptr<Halfedge> halfedge)
{
    if (auto v = halfedge->tail.lock())
    {
        if (v->getConnectionCount() <= 1)
        {
            removeVertex(v);
        }
    }

    if (auto v = halfedge->head.lock())
    {
        if (v->getConnectionCount() <= 1)
        {
            removeVertex(v);
        }
    }

    if (auto b = halfedge->b0.lock())
    {
        removeBezier(b);
    }

    if (auto b = halfedge->b1.lock())
    {
        removeBezier(b);
    }

    auto it = std::find_if(halfedges.begin(), halfedges.end(), [halfedge](std::shared_ptr<Halfedge> h) { return h.get() == halfedge.get(); });
    if (it != halfedges.end())
    {
        halfedges.erase(it);
    }
}

void GradientMesh::removeVertex(std::shared_ptr<Vertex> vertex)
{
    auto it = std::find_if(vertices.begin(), vertices.end(), [vertex](std::shared_ptr<Vertex> v) { return v.get() == vertex.get(); });
    if (it != vertices.end())
    {
        vertices.erase(it);
    }
}

void GradientMesh::removeBezier(std::shared_ptr<BezierControlPoint> bezier)
{
    auto it = std::find_if(bezierControlPoints.begin(), bezierControlPoints.end(), [bezier](std::shared_ptr<BezierControlPoint> b) { return b.get() == bezier.get(); });
    if (it != bezierControlPoints.end())
    {
        bezierControlPoints.erase(it);
    }
}

std::shared_ptr<GradientMesh::Patch> GradientMesh::addConnectedPatch(Patch* sourcePatch, Direction sourceEdgePlacement)
{
    jassert(!sourcePatch->isConnected(sourceEdgePlacement));

    std::array<std::shared_ptr<Halfedge>, 4> patchHalfedges;
    std::array<juce::Colour, 4> colors = sourcePatch->getColors();

    auto translationHalfedge = sourcePatch->getHalfedges()[(int)counterclockwiseFrom(sourceEdgePlacement)].lock();
    auto destinationStartEdgePlacement = opposite(sourceEdgePlacement);
    std::shared_ptr<Halfedge> destinationStartHalfedge;
    if (auto halfedge = sourcePatch->getHalfedges()[(int)sourceEdgePlacement].lock())
    {
        if (auto twin = halfedge->twin.lock())
        {
            destinationStartHalfedge = twin;
        }
    }

    if (!translationHalfedge || !destinationStartHalfedge)
    {
        return nullptr;
    }

    patchHalfedges[(int)destinationStartEdgePlacement] = destinationStartHalfedge;

    auto sourceStartEdgeCorners = toCorners(sourceEdgePlacement);
    auto destStartEdgeCorners = toCorners(destinationStartEdgePlacement);

    colors[(int)destStartEdgeCorners.second] = sourcePatch->getColors()[(int)sourceStartEdgeCorners.first];
    colors[(int)destStartEdgeCorners.first] = sourcePatch->getColors()[(int)sourceStartEdgeCorners.second];

    //
    // Figure out how far to translate the new patch
    //
    juce::Point<float> translation;
    {
        auto head = translationHalfedge->head.lock();
        auto tail = translationHalfedge->tail.lock();
        if (!head || !tail)
        {
            return nullptr;
        }

        translation = head->position - tail->position;
    }

    //
    // Find existing halfedges
    //
    // Go clockwise starting from the existing halfedge head
    //
    if (auto vertex = destinationStartHalfedge->head.lock())
    {
        auto destinationEdgePlacement = clockwiseFrom(destinationStartEdgePlacement);
        while (destinationEdgePlacement != destinationStartEdgePlacement)
        {
            auto vertexEdgeDirection = clockwiseFrom(destinationEdgePlacement);
            auto halfedge = vertex->getHalfedge(vertexEdgeDirection).lock();
            if (!halfedge)
            {
                break;
            }

            if (auto existingPatch = halfedge->patch.lock())
            {
                auto destPatchCorners = toCorners(destinationEdgePlacement);
                auto sourcePatchCorners = toCorners(opposite(destinationEdgePlacement));
                colors[(int)destPatchCorners.first] = existingPatch->getColors()[(int)sourcePatchCorners.first];
                colors[(int)destPatchCorners.second] = existingPatch->getColors()[(int)sourcePatchCorners.second];
            }

            patchHalfedges[(int)destinationEdgePlacement] = halfedge;
            vertex = halfedge->head.lock();
            if (!vertex)
                break;

            destinationEdgePlacement = clockwiseFrom(destinationEdgePlacement);
        }
    }

    //
    // Counterclockwise from the existing halfedge tail
    //
    if (auto vertex = destinationStartHalfedge->tail.lock())
    {
        auto destinationEdgePlacement = counterclockwiseFrom(destinationStartEdgePlacement);
        while (destinationEdgePlacement != destinationStartEdgePlacement)
        {
            auto vertexEdgeDirection = counterclockwiseFrom(destinationEdgePlacement);
            auto halfedge = vertex->getHalfedge(vertexEdgeDirection).lock();
            if (!halfedge)
            {
                break;
            }

            if (auto existingPatch = halfedge->patch.lock())
            {
                auto destPatchCorners = toCorners(destinationEdgePlacement);
                auto sourcePatchCorners = toCorners(opposite(destinationEdgePlacement));
                colors[(int)destPatchCorners.first] = existingPatch->getColors()[(int)sourcePatchCorners.first];
                colors[(int)destPatchCorners.second] = existingPatch->getColors()[(int)sourcePatchCorners.second];
            }

            patchHalfedges[(int)destinationEdgePlacement] = halfedge->twin.lock();
            vertex = halfedge->twin.lock()->tail.lock();
            if (!vertex)
                break;

            destinationEdgePlacement = counterclockwiseFrom(destinationEdgePlacement);
        }
    }

    {
        if (auto tail = destinationStartHalfedge->head.lock())
        {
            auto edgePlacement = clockwiseFrom(destinationStartEdgePlacement);
            while (edgePlacement != destinationStartEdgePlacement)
            {
                if (auto destHalfedge = patchHalfedges[(int)edgePlacement])
                {
                    DBG("already have halfedge for " << directionNames[(int)edgePlacement] << " " << destHalfedge->toString());
                }
                else
                {
                    auto nextEdgePlacement = clockwiseFrom(edgePlacement);
                    if (auto sourceHalfedge = sourcePatch->getHalfedges()[(int)edgePlacement].lock())
                    {
                        auto b0 = sourceHalfedge->b0.lock();
                        auto b1 = sourceHalfedge->b1.lock();
                        if (!b0 || !b1)
                        {
                            return nullptr;
                        }

                        auto bcp0 = std::make_shared<BezierControlPoint>(b0->position + translation, *this);
                        auto bcp1 = std::make_shared<BezierControlPoint>(b1->position + translation, *this);

                        bezierControlPoints.push_back(bcp0);
                        bezierControlPoints.push_back(bcp1);

                        if (auto nextHalfedge = patchHalfedges[(int)nextEdgePlacement])
                        {
                            if (auto head = nextHalfedge->tail.lock())
                                patchHalfedges[(int)edgePlacement] = addHalfedge(tail, head, bcp0, bcp1, edgePlacement);
                        }
                        else
                        {
                            if (auto head = sourceHalfedge->head.lock())
                            {
                                auto vertex = addVertex(head->position + translation);
                                patchHalfedges[(int)edgePlacement] = addHalfedge(tail, vertex, bcp0, bcp1, edgePlacement);
                            }
                        }
                    }
                }

                check();

                tail = patchHalfedges[(int)edgePlacement]->head.lock();
                if (!tail)
                    break;

                edgePlacement = clockwiseFrom(edgePlacement);
            }
        }
    }

    auto patch = std::make_shared<Patch>(patchHalfedges);

    for (auto& patchHalfedge : patchHalfedges)
    {
        patchHalfedge->patch = patch;
    }

    for (auto const& cornerPlacement : corners)
    {
        patch->setColor(cornerPlacement, colors[(int)cornerPlacement]);
    }

    patch->update();
    patches.push_back(patch);

    check();

    return patch;
}

std::shared_ptr<GradientMesh::Vertex> GradientMesh::addVertex(juce::Point<float> point)
{
    vertices.emplace_back(std::make_shared<Vertex>(point, *this, vertices.size()));
    return vertices.back();
}

std::shared_ptr<GradientMesh::Halfedge> GradientMesh::addHalfedge(std::shared_ptr<Vertex> tail,
    std::shared_ptr<Vertex> head,
    std::shared_ptr<BezierControlPoint> b0,
    std::shared_ptr<BezierControlPoint> b1,
    Direction edgePlacement)
{
    juce::Line<float> line{ tail->position, head->position };

    auto halfedge = std::make_shared<Halfedge>();
    halfedge->tail = tail;
    halfedge->head = head;
    halfedge->b0 = b0;
    halfedge->b1 = b1;

    auto twin = std::make_shared<Halfedge>();
    twin->head = halfedge->tail;
    twin->tail = halfedge->head;
    twin->b0 = b1;
    twin->b1 = b0;

    halfedge->twin = twin;
    twin->twin = halfedge;

    jassert(!tail->getHalfedge(clockwiseFrom(edgePlacement)).lock());
    jassert(!head->getHalfedge(counterclockwiseFrom(edgePlacement)).lock());

    tail->halfedges[(int)clockwiseFrom(edgePlacement)] = halfedge;
    head->halfedges[(int)counterclockwiseFrom(edgePlacement)] = halfedge->twin.lock();

    halfedges.push_back(halfedge);
    halfedges.push_back(twin);

    return halfedge;
}

void GradientMesh::check()
{
#if 0
    for (auto i = 0; i < vertices.size(); ++i)
    {
        for (auto j = i + 1; j < vertices.size(); ++j)
        {
            if (vertices[i]->position == vertices[j]->position)
            {
                DBG("Duplicate vertex found at " << i << " and " << j);
                jassertfalse;
            }
        }
    }

    for (auto i = 0; i < halfedges.size(); ++i)
    {
        for (auto j = i + 1; j < halfedges.size(); ++j)
        {
            if (halfedges[i]->tail->position == halfedges[j]->tail->position &&
                halfedges[i]->head->position == halfedges[j]->head->position)
            {
                DBG("Duplicate halfedge found at " << i << " and " << j);
                jassertfalse;
            }
        }
    }
#endif
}

void GradientMesh::applyTransform(const juce::AffineTransform& transform) noexcept
{
    for (auto& vertex : vertices)
    {
        vertex->position.applyTransform(transform);
    }

    for (auto& bezier : bezierControlPoints)
    {
        bezier->position.applyTransform(transform);
    }

    for (auto& patch : patches)
    {
        patch->update();
    }
}

void GradientMesh::draw(juce::Image image, juce::AffineTransform transform)
{
    auto convertVertex = [&](std::weak_ptr<Vertex> vertex)
        {
            if (auto v = vertex.lock())
            {
                auto p = v->position.transformedBy(transform);
                return D2D1_POINT_2F{ p.x, p.y };
            }

            throw std::runtime_error("Invalid vertex");
        };

    auto convertBezier = [&](std::weak_ptr<BezierControlPoint> bezier)
        {
            if (auto b = bezier.lock())
            {
                auto p = b->position.transformedBy(transform);
                return D2D1_POINT_2F{ p.x, p.y };
            }

            throw std::runtime_error("Invalid bezier control point");
        };

    auto convertHalfedge = [&](std::weak_ptr<Halfedge> halfedge,
        D2D1_POINT_2F& d2dBezeir0,
        D2D1_POINT_2F& d2dBezeir1,
        D2D1_POINT_2F& head,
        D2D1_PATCH_EDGE_MODE& d2dEdgeMode
        )
        {
            if (auto h = halfedge.lock())
            {
                head = convertVertex(h->head);
                d2dEdgeMode = h->twin.lock()->patch.lock() ? D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ALIASED : D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ANTIALIASED;

                if (h->edgeType == EdgeType::straight)
                {
                    if (auto tail = h->tail.lock())
                    {
                        juce::Line<float> line{ tail->position.x, tail->position.y, head.x, head.y };
                        auto mid = line.getPointAlongLineProportionally(0.5f);
                        d2dBezeir0 = { mid.x, mid.y };
                        d2dBezeir1 = { mid.x, mid.y };
                    }
                    return;
                }

                d2dBezeir0 = convertBezier(h->b0);
                d2dBezeir1 = convertBezier(h->b1);

                return;
            }

            throw std::runtime_error("Invalid halfedge");
        };

    std::vector<D2D1_GRADIENT_MESH_PATCH> d2dPatches;
    for (auto const& patch : patches)
    {
        d2dPatches.emplace_back(D2D1_GRADIENT_MESH_PATCH{});
        auto& d2dPatch = d2dPatches.back();

        try
        {
            const auto& patchHalfedges = patch->getHalfedges();
            if (auto halfedge = patchHalfedges[(int)Direction::north].lock())
            {
                d2dPatch.point00 = convertVertex(halfedge->tail);
            }

            convertHalfedge(patchHalfedges[(int)Direction::north], d2dPatch.point01, d2dPatch.point02, d2dPatch.point03, d2dPatch.topEdgeMode);
            convertHalfedge(patchHalfedges[(int)Direction::east], d2dPatch.point13, d2dPatch.point23, d2dPatch.point33, d2dPatch.rightEdgeMode);
            convertHalfedge(patchHalfedges[(int)Direction::south], d2dPatch.point32, d2dPatch.point31, d2dPatch.point30, d2dPatch.bottomEdgeMode);
            convertHalfedge(patchHalfedges[(int)Direction::west], d2dPatch.point20, d2dPatch.point10, d2dPatch.point00, d2dPatch.leftEdgeMode);

            const auto& colors = patch->getColors();
            d2dPatch.color00 = D2DUtilities::toCOLOR_F(colors[(int)CornerPlacement::topLeft]);
            d2dPatch.color03 = D2DUtilities::toCOLOR_F(colors[(int)CornerPlacement::topRight]);
            d2dPatch.color33 = D2DUtilities::toCOLOR_F(colors[(int)CornerPlacement::bottomRight]);
            d2dPatch.color30 = D2DUtilities::toCOLOR_F(colors[(int)CornerPlacement::bottomLeft]);

            d2dPatch.point11 = d2dPatch.point00;
            d2dPatch.point12 = d2dPatch.point03;
            d2dPatch.point21 = d2dPatch.point30;
            d2dPatch.point22 = d2dPatch.point33;
        }
        catch (...)
        {
            d2dPatches.pop_back();
        }
    }

    pimpl->createResources(image);

    if (pimpl->deviceContext && image.isValid())
    {
        pimpl->gradientMesh = {};
        pimpl->deviceContext->CreateGradientMesh(d2dPatches.data(), (uint32_t)d2dPatches.size(), pimpl->gradientMesh.put());

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

void GradientMesh::setVertexPosition(Vertex* vertex, juce::Point<float> position)
{
    vertex->position = position;

    for (auto& patch : patches)
    {
        for (auto const& halfedgeWeakPtr : patch->getHalfedges())
        {
            if (auto halfedge = halfedgeWeakPtr.lock())
            {
                auto tail = halfedge->tail.lock();
                auto head = halfedge->head.lock();

                if (tail.get() == vertex || head.get() == vertex)
                {
                    patch->update();
                }
            }
        }
    }
}

void GradientMesh::setEdgeType(Halfedge* edge, EdgeType edgeType)
{
    edge->edgeType = edgeType;
    edge->twin.lock()->edgeType = edgeType;
}

juce::Rectangle<float> GradientMesh::getBounds() const noexcept
{
    juce::Rectangle<float> bounds;

    for (auto const& patch : patches)
    {
        bounds = bounds.getUnion(patch->getPath().getBounds());
    }

    return bounds;
}

GradientMesh::Patch::Patch(std::array<std::shared_ptr<Halfedge>, 4>& halfedges_)
{
    for (size_t index = 0; index < 4; ++index)
    {
        halfedges[index] = halfedges_[index];
    }
}

GradientMesh::Patch::~Patch()
{
    for (auto& halfedge : halfedges)
    {
        halfedge.reset();
    }
}

void GradientMesh::Patch::update()
{
    createPath();

    for (auto const& halfedgeWeakPtr : halfedges)
    {
        if (auto halfedge = halfedgeWeakPtr.lock())
        {
            if (auto twinPatch = halfedge->twin.lock()->patch.lock())
            {
                twinPatch->path.clear();
            }
        }
    }
}

void GradientMesh::Patch::createPath()
{
    path.clear();

    {
        auto halfedge = halfedges[(int)Direction::north].lock();
        if (!halfedge)
            return;

        auto tail = halfedge->tail.lock();
        if (!tail)
            return;

        path.startNewSubPath(tail->position);
    }

    for (auto const& halfedgeWeakPtr : halfedges)
    {
        if (auto halfedge = halfedgeWeakPtr.lock())
        {
            auto head = halfedge->head.lock();
            auto b0 = halfedge->b0.lock();
            auto b1 = halfedge->b1.lock();

            switch (halfedge->edgeType)
            {
            case EdgeType::straight:
                if (head)
                    path.lineTo(head->position);
                break;

            case EdgeType::approximateQuadratic:
            case EdgeType::cubic:
            {
                if (head && b0 && b1)
                    path.cubicTo(b0->position,
                        b1->position,
                        head->position);
                break;
            }
            }
        }

    }

    path.closeSubPath();
}

juce::String GradientMesh::toString() const
{
    juce::String text = "\nPatch";

    DBG("# vertices: " << (int)vertices.size());
    for (auto const& vertex : vertices)
    {
        DBG("      Vertex " << vertex->toString("         "));
    }

    return text;
}

std::unique_ptr<GradientMesh> GradientMesh::pathToGrid(Path const& path, AffineTransform const& transform, float tolerance)
{
    auto mesh = std::make_unique<GradientMesh>();

#if 0
    auto geometry = Pimpl::pathToGeometry(path, transform);
    if (!geometry)
    {
        return {};
    }

    struct TessellationSink : public juce::ComBaseClassHelper<ID2D1TessellationSink>
    {
        STDOVERRIDEMETHODIMP_(void) AddTriangles(_In_reads_(trianglesCount) CONST D2D1_TRIANGLE* triangles, UINT32 trianglesCount) override
        {
            for (UINT32 i = 0; i < trianglesCount; ++i)
            {
                DBG(triangles[i].point1.x << ", " << triangles[i].point1.y << " -> " << triangles[i].point2.x << ", " << triangles[i].point2.y << " -> " << triangles[i].point3.x << ", " << triangles[i].point3.y);
                this->triangleArray.push_back(triangles[i]);
            }
        }

        STDOVERRIDEMETHODIMP Close() override { return S_OK; }

        std::vector<D2D1_TRIANGLE> triangleArray;
    } sink;

    geometry->Tessellate(D2DUtilities::transformToMatrix(transform), tolerance, &sink);
#endif

    juce::PathFlatteningIterator it{ path, transform, tolerance };
    juce::SortedSet<float> xValues, yValues;
    std::vector<juce::Point<float>> perimeterPoints;
    while (it.next())
    {
        auto perimeterIndex = perimeterPoints.size();
        perimeterPoints.emplace_back(it.x1, it.y1);
        xValues.add(it.x1);
        yValues.add(it.y1);
    }

    DBG("\n");

    for (auto y : yValues)
    {
        DBG("y:" << y);
    }

    Clipper2Lib::PathD subjectPath;

    for (auto it = perimeterPoints.begin(); it != perimeterPoints.end(); ++it)
    {
        auto const& point = *it;
        int64_t z = int64_t(it - perimeterPoints.begin()) << 48;
        z |= 0x8000000000000000LL;
        subjectPath.emplace_back(point.x, point.y, z);
    }

    Clipper2Lib::PathsD subjectPaths{ subjectPath };
    Clipper2Lib::PathsD gridPaths;

    struct Grid
    {
        const size_t numColumns = 0;
        const size_t numRows = 0;

        void set(size_t x, size_t y, std::shared_ptr<GradientMesh::Vertex> vertex)
        {
            vertices[x + y * numColumns] = vertex;
        }

        auto& get(size_t x, size_t y)
        {
            return vertices[x + y * numColumns];
        }

        struct Intersection
        {
            size_t perimeterIndex = (size_t)-1;
        };

        std::vector<std::shared_ptr<GradientMesh::Vertex>> vertices{ numColumns * numRows };
    } grid{ (size_t)xValues.size(), (size_t)yValues.size() };


    for (auto itx = xValues.begin(); itx != xValues.end() - 1; ++itx)
    {
        for (auto ity = yValues.begin(); ity != yValues.end() - 1; ++ity)
        {
            int64_t gridColumn = itx - xValues.begin();
            int64_t gridRow = ity - yValues.begin();

            auto encode = [&](int64_t column, int64_t row)
                {
                    return (column << 24) | row;
                };

            auto decodeColumn = [](int64_t z)
                {
                    return (z >> 24) & 0xFFFFFF;
                };

            auto decodeRow = [](int64_t z)
                {
                    return z & 0xFFFFFF;
                };

            auto decodePerimeterIndex = [](int64_t z)
                {
                    return (z >> 48) && 0xffffff;
                };

            gridPaths.clear();
            gridPaths.emplace_back(Clipper2Lib::PathD
                {
                    { *(itx + 1), *(ity + 1), encode(gridColumn + 1, gridRow + 1) },
                    { *itx, *(ity + 1), encode(gridColumn, gridRow + 1)},
                    { *itx, *ity, encode(gridColumn, gridRow)},
                    { *(itx + 1), *ity, encode(gridColumn + 1, gridRow)}
                });
            auto intersectionPaths = Clipper2Lib::Intersect(subjectPaths, gridPaths, Clipper2Lib::FillRule::Positive);

            if (intersectionPaths.size())
            {
                for (auto const& intersectionPath : intersectionPaths)
                {
                    for (auto const& point : intersectionPath)
                    {
                        auto z = point.z;
                        if (z & 0x8000000000000000LL)
                        {
                            for (auto const& gridPathPoint : gridPaths.front())
                            {
                                if (approximatelyEqual(gridPathPoint.x, point.x) && approximatelyEqual(gridPathPoint.y, point.y))
                                {
                                    z = gridPathPoint.z;
                                    break;
                                }
                            }
                        }

                        auto column = decodeColumn(point.z);
                        auto row = decodeRow(point.z);

                        if (grid.get(column, row))
                            continue;

                        auto vertex = mesh->addVertex({ (float)point.x, (float)point.y });
                        grid.set(column, row, vertex);
                    }
                }
            }
        }
    }

    for (auto const& perimeterPoint : perimeterPoints)
    {
        mesh->addVertex(perimeterPoint);
    }

    for (size_t x = 0; x < grid.numColumns; ++x)
    {
        for (size_t y = 0; y < grid.numRows - 1; ++y)
        {
            auto vertex = grid.get(x, y);
            if (vertex)
            {
                if (auto vertexBelow = grid.get(x, y + 1))
                {
                    mesh->addHalfedge(vertex, vertexBelow, nullptr, nullptr, Direction::south);
                }
            }
        }
    }

    for (size_t x = 0; x < grid.numColumns - 1; ++x)
    {
        for (size_t y = 0; y < grid.numRows; ++y)
        {
            auto vertex = grid.get(x, y);
            if (vertex)
            {
                if (auto vertexRight = grid.get(x + 1, y))
                {
                    mesh->addHalfedge(vertex, vertexRight, nullptr, nullptr, Direction::east);
                }
            }
        }
    }

    return mesh;
}

int GradientMesh::Vertex::getConnectionCount() const
{
    int count = 0;

    for (auto const& halfedgeWeakPtr : halfedges)
    {
        if (auto halfedge = halfedgeWeakPtr.lock())
        {
            ++count;
        }
    }

    return count;
}
