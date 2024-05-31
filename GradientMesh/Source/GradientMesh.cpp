#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <d2d1_3helper.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#include <JuceHeader.h>
#include <juce_graphics/native/juce_DirectX_windows.h>
#include <juce_graphics/native/juce_Direct2DImage_windows.h>
#include "GradientMesh.h"

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
}

void GradientMesh::addPatch(juce::Rectangle<float> bounds)
{
    std::array<std::shared_ptr<Halfedge>, 4> patchHalfedges;

    std::array<juce::Point<float>, 4> const corners
    {
        bounds.getTopLeft(),
        bounds.getTopRight(),
        bounds.getBottomRight(),
        bounds.getBottomLeft()
    };

    auto addBezierPointsAndHalfedges = [&](std::shared_ptr<Vertex> tail, std::shared_ptr<Vertex> head)
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

            auto halfedge = addHalfedge(tail, head, b0, b1);
            tail->halfedge = halfedge;

            return halfedge;
        };

    auto topLeftVertex = addVertex(bounds.getTopLeft());
    auto topRightVertex = addVertex(bounds.getTopRight());
    auto bottomRightVertex = addVertex(bounds.getBottomRight());
    auto bottomLeftVertex = addVertex(bounds.getBottomLeft());

    patchHalfedges[EdgePlacement::top] = addBezierPointsAndHalfedges(topLeftVertex, topRightVertex);
    patchHalfedges[EdgePlacement::right] = addBezierPointsAndHalfedges(topRightVertex, bottomRightVertex);
    patchHalfedges[EdgePlacement::bottom] = addBezierPointsAndHalfedges(bottomRightVertex, bottomLeftVertex);
    patchHalfedges[EdgePlacement::left] = addBezierPointsAndHalfedges(bottomLeftVertex, topLeftVertex);

    auto patch = std::make_shared<Patch>(patchHalfedges);
    patch->update();
    patches.push_back(patch);
}

void GradientMesh::addPatch(std::shared_ptr<Patch> patch)
{
    patch->update();
    patches.push_back(patch);
}

std::shared_ptr<GradientMesh::Patch> GradientMesh::createConnectedPatch(Patch* sourcePatch, EdgePlacement edge) const
{
    std::array<std::shared_ptr<Halfedge>, 4> patchHalfedges;

    auto oppositeEdge = edge.opposite();
    patchHalfedges[oppositeEdge.placement] = sourcePatch->getHalfedges()[edge.placement]->twin;



    return std::shared_ptr<Patch>();
}

std::shared_ptr<GradientMesh::Vertex> GradientMesh::addVertex(juce::Point<float> point)
{
    vertices.emplace_back(std::make_shared<Vertex>(point, *this, vertices.size()));
    return vertices.back();
}

std::shared_ptr<GradientMesh::Halfedge> GradientMesh::addHalfedge(std::shared_ptr<Vertex> tail, std::shared_ptr<Vertex> head, std::shared_ptr<BezierControlPoint> b0, std::shared_ptr<BezierControlPoint> b1)
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

    halfedges.push_back(halfedge);
    halfedges.push_back(twin);

    return halfedge;
}

void GradientMesh::applyTransform(const AffineTransform& transform) noexcept
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
    auto convertVertex = [&](Vertex* vertex)
    {
        auto p = vertex->position.transformedBy(transform);
        return D2D1_POINT_2F{ p.x, p.y };
    };

    auto convertBezier= [&](BezierControlPoint* bezier)
        {
            auto p = bezier->position.transformedBy(transform);
            return D2D1_POINT_2F{ p.x, p.y };
        };

    std::vector<D2D1_GRADIENT_MESH_PATCH> d2dPatches;
    for (auto const& patch : patches)
    {
        d2dPatches.emplace_back(D2D1_GRADIENT_MESH_PATCH{});
        auto& d2dPatch = d2dPatches.back();

        const auto& patchHalfedges = patch->getHalfedges();
        auto halfedge = patchHalfedges[EdgePlacement::top];
        d2dPatch.point00 = convertVertex(halfedge->tail.get());

        d2dPatch.point01 = convertBezier(halfedge->b0.get());
        d2dPatch.point02 = convertBezier(halfedge->b1.get());
        d2dPatch.point03 = convertVertex(halfedge->head.get());

        halfedge = patchHalfedges[EdgePlacement::right];
        d2dPatch.point13 = convertBezier(halfedge->b0.get());
        d2dPatch.point23 = convertBezier(halfedge->b1.get());
        d2dPatch.point33 = convertVertex(halfedge->head.get());

        halfedge = patchHalfedges[EdgePlacement::bottom];
        d2dPatch.point32 = convertBezier(halfedge->b0.get());
        d2dPatch.point31 = convertBezier(halfedge->b1.get());
        d2dPatch.point30 = convertVertex(halfedge->head.get());

        halfedge = patchHalfedges[EdgePlacement::left];
        d2dPatch.point20 = convertBezier(halfedge->b0.get());
        d2dPatch.point10 = convertBezier(halfedge->b1.get());

        const auto& colors = patch->getColors();
        d2dPatch.color00 = D2DUtilities::toCOLOR_F(colors[CornerPlacement::topLeft]);
        d2dPatch.color03 = D2DUtilities::toCOLOR_F(colors[CornerPlacement::topRight]);
        d2dPatch.color33 = D2DUtilities::toCOLOR_F(colors[CornerPlacement::bottomRight]);
        d2dPatch.color30 = D2DUtilities::toCOLOR_F(colors[CornerPlacement::bottomLeft]);

        d2dPatch.point11 = d2dPatch.point00;
        d2dPatch.point12 = d2dPatch.point03;
        d2dPatch.point21 = d2dPatch.point30;
        d2dPatch.point22 = d2dPatch.point33;
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
        for (auto const& halfedge : patch->getHalfedges())
        {
            if (halfedge->tail.get() == vertex || halfedge->head.get() == vertex)
            {
                patch->update();
            }
        }
    }
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

GradientMesh::Patch::Patch(std::array<std::shared_ptr<Halfedge>, 4>& halfedges_) :
    halfedges(halfedges_)
{
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
    path.clear();
    path.startNewSubPath(halfedges[EdgePlacement::top]->tail->position);

    for (auto const& halfedge : halfedges)
    {
        switch (halfedge->edgeType)
        {
        case EdgeType::straight:
            path.lineTo(halfedge->head->position);
            break;

        case EdgeType::quadratic:
        case EdgeType::cubic:
            path.cubicTo(halfedge->b0->position,
                halfedge->b1->position,
                halfedge->head->position);
            break;
        }
    }

    path.closeSubPath();
}

juce::String GradientMesh::toString() const
{
    String text = "\nPatch";

    int index = 0;
    for (auto const &vertex : vertices)
    {
        DBG("   " << index++ << "  " << vertex->position.toString());
    }

    return text;
}
