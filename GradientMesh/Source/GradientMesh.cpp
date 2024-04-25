/*
  ==============================================================================

    GradientMesh.cpp
    Created: 22 Apr 2024 12:03:36am
    Author:  Matt Gonzalez

  ==============================================================================
*/

#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#include <JuceHeader.h>
#include <juce_graphics/native/juce_DirectX_windows.h>
#include <juce_graphics/native/juce_Direct2DImage_windows.h>
#include "GradientMesh.h"
#include <d2d1_3helper.h>

const std::array<juce::Colour, 4> GradientMesh::Patch::defaultColors =
{
    juce::Colours::red,
    juce::Colours::yellow,
    juce::Colours::blue,
    juce::Colours::violet
};

struct GradientMesh::Patch::PatchPimpl
{
    PatchPimpl(Patch& owner_) :
        owner(owner_)
    {
    }

    ~PatchPimpl()
    {
    }

    Patch& owner;
};

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

juce::Rectangle<float> GradientMesh::getBounds() const noexcept
{
    juce::Rectangle<float> bounds;
    for (auto patch : patches)
    {
        bounds = bounds.getUnion(patch->getBounds());
    }

    return bounds;
}

void GradientMesh::updateMesh()
{
    auto toPoint2F = [&](juce::Point<float> p)
        {
            return D2D1::Point2F(p.x, p.y);
        };

    if (pimpl->deviceContext && !pimpl->gradientMesh)
    {
        std::vector<D2D1_GRADIENT_MESH_PATCH> d2dPatches;
        d2dPatches.reserve(patches.size());

        auto d2dPatch = d2dPatches.data();
        for (auto const& patch : patches)
        {
            *d2dPatch = D2D1::GradientMeshPatchFromCoonsPatch(toPoint2F(patch->options.upperLeftCorner.position),
                toPoint2F(patch->options.topEdge.leftControlPoint),
                toPoint2F(patch->options.topEdge.rightControlPoint),
                toPoint2F(patch->options.upperRightCorner.position),

                toPoint2F(patch->options.rightEdge.upperControlPoint),
                toPoint2F(patch->options.rightEdge.lowerControlPoint),
                toPoint2F(patch->options.lowerRightCorner.position),

                toPoint2F(patch->options.bottomEdge.rightControlPoint),
                toPoint2F(patch->options.bottomEdge.leftControlPoint),
                toPoint2F(patch->options.lowerLeftCorner.position),

                toPoint2F(patch->options.leftEdge.lowerControlPoint),
                toPoint2F(patch->options.leftEdge.upperControlPoint),

                juce::D2DUtilities::toCOLOR_F(patch->options.upperLeftCorner.color),
                juce::D2DUtilities::toCOLOR_F(patch->options.upperRightCorner.color),
                juce::D2DUtilities::toCOLOR_F(patch->options.lowerRightCorner.color),
                juce::D2DUtilities::toCOLOR_F(patch->options.lowerLeftCorner.color),

                D2D1_PATCH_EDGE_MODE_ALIASED_INFLATED, D2D1_PATCH_EDGE_MODE_ALIASED, D2D1_PATCH_EDGE_MODE_ALIASED, D2D1_PATCH_EDGE_MODE_ALIASED);
                //D2D1_PATCH_EDGE_MODE_ANTIALIASED, D2D1_PATCH_EDGE_MODE_ANTIALIASED, D2D1_PATCH_EDGE_MODE_ANTIALIASED, D2D1_PATCH_EDGE_MODE_ANTIALIASED);
            d2dPatch++;
        }

        pimpl->deviceContext->CreateGradientMesh(d2dPatches.data(), patches.size(), pimpl->gradientMesh.put());
    }
}

void GradientMesh::draw(juce::Image image, juce::AffineTransform transform)
{
    pimpl->createResources(image);

    updateMesh();

    if (pimpl->deviceContext && image.isValid() && pimpl->gradientMesh)
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

#if 0
    auto drawDot = [&](D2D1_POINT_2F p)
        {
            juce::Graphics g{ image };
            g.setColour(juce::Colours::white);
            g.fillEllipse(p.x - 16.0f, p.y - 16.0f, 32.0f, 32.0f);
        };

    for (auto patch : patches)
    {
        auto toPoint2F = [&](juce::Point<float> p)
            {
                return D2D1::Point2F(p.x, p.y);
            };

        D2D1_GRADIENT_MESH_PATCH d2dPatch = D2D1::GradientMeshPatchFromCoonsPatch(toPoint2F(patch->options.upperLeftCorner.position),
            toPoint2F(patch->options.topEdge.leftControlPoint),
            toPoint2F(patch->options.topEdge.rightControlPoint),
            toPoint2F(patch->options.upperRightCorner.position),

            toPoint2F(patch->options.rightEdge.upperControlPoint),
            toPoint2F(patch->options.rightEdge.lowerControlPoint),
            toPoint2F(patch->options.lowerRightCorner.position),

            toPoint2F(patch->options.bottomEdge.rightControlPoint),
            toPoint2F(patch->options.bottomEdge.leftControlPoint),
            toPoint2F(patch->options.lowerLeftCorner.position),

            toPoint2F(patch->options.leftEdge.lowerControlPoint),
            toPoint2F(patch->options.leftEdge.upperControlPoint),

            juce::D2DUtilities::toCOLOR_F(patch->options.upperLeftCorner.color),
            juce::D2DUtilities::toCOLOR_F(patch->options.upperRightCorner.color),
            juce::D2DUtilities::toCOLOR_F(patch->options.lowerRightCorner.color),
            juce::D2DUtilities::toCOLOR_F(patch->options.lowerLeftCorner.color),

            D2D1_PATCH_EDGE_MODE_ANTIALIASED, D2D1_PATCH_EDGE_MODE_ANTIALIASED, D2D1_PATCH_EDGE_MODE_ANTIALIASED, D2D1_PATCH_EDGE_MODE_ANTIALIASED);

        drawDot(d2dPatch.point00);
        drawDot(d2dPatch.point01);
        drawDot(d2dPatch.point02);
        drawDot(d2dPatch.point03);
        drawDot(d2dPatch.point10);
        drawDot(d2dPatch.point11);
        drawDot(d2dPatch.point12);
        drawDot(d2dPatch.point13);
        drawDot(d2dPatch.point20);
        drawDot(d2dPatch.point21);
        drawDot(d2dPatch.point22);
        drawDot(d2dPatch.point23);
        drawDot(d2dPatch.point30);
        drawDot(d2dPatch.point31);
        drawDot(d2dPatch.point32);
        drawDot(d2dPatch.point33);
    }
#endif
}

#if 0
GradientMesh::Patch::Ptr GradientMesh::addConnectedPatch(Patch::Ptr existingPatch, Direction direction, std::array<juce::Colour, 4> colors)
{
    if (!existingPatch)
        existingPatch = patches.getLast();

    auto existingFace = existingPatch->face;

    switch (direction)
    {
    case GradientMesh::Direction::east:
    {
        auto topLeft = existingFace->vertices[(size_t)Corner::topRight];
        auto bottomLeft = existingFace->vertices[(size_t)Corner::bottomRight];

        auto existingTopEdge = existingFace->edges[(size_t)Direction::north]->toLine();
        auto topRightPosition = existingTopEdge.getPointAlongLineProportionally(2.0f);

        auto existingBottomEdge = existingFace->edges[(size_t)Direction::south]->toLine();
        auto bottomRightPosition = existingBottomEdge.getPointAlongLineProportionally(2.0f);

        auto topRight = pimpl->addVertex(topRightPosition);
        auto bottomRight = pimpl->addVertex(bottomRightPosition);

        auto topEdge = pimpl->addEdge(Orientation::horizontal, topLeft, topRight);
        auto rightEdge = pimpl->addEdge(Orientation::vertical, topRight, bottomRight);
        auto bottomEdge = pimpl->addEdge(Orientation::horizontal, bottomLeft, bottomRight);
        auto leftEdge = existingFace->edges[(size_t)Direction::east];

        auto face = pimpl->addFace(topLeft, topRight, bottomRight, bottomLeft,
            topEdge, rightEdge, bottomEdge, leftEdge);
        face->colors = colors;
        face->setInnerControlPoints();

        return patches.add(new Patch{ face });
    }

    case GradientMesh::Direction::south:
    {
        auto topLeft = existingFace->vertices[(size_t)Corner::bottomLeft];
        auto topRight = existingFace->vertices[(size_t)Corner::bottomRight];

        auto existingLeftEdgeLine = existingFace->edges[(size_t)Direction::west]->toLine();
        auto bottomLeftPosition = existingLeftEdgeLine.getPointAlongLineProportionally(2.0f);

        auto existingRightEdgeLine = existingFace->edges[(size_t)Direction::east]->toLine();
        auto bottomRightPosition = existingRightEdgeLine.getPointAlongLineProportionally(2.0f);

        auto bottomRight = pimpl->addVertex(bottomRightPosition);
        auto bottomLeft = pimpl->addVertex(bottomLeftPosition);

        auto topEdge = existingFace->edges[(size_t)Direction::south];
        auto rightEdge = pimpl->addEdge(Orientation::vertical, topRight, bottomRight);
        auto bottomEdge = pimpl->addEdge(Orientation::horizontal, bottomLeft, bottomRight);
        auto leftEdge = pimpl->addEdge(Orientation::vertical, topLeft, bottomLeft);

        auto face = pimpl->addFace(topLeft, topRight, bottomRight, bottomLeft,
            topEdge, rightEdge, bottomEdge, leftEdge);
        face->colors = colors;
        face->setInnerControlPoints();

        return patches.add(new Patch{ face });
    }

    case GradientMesh::Direction::west:
    {
        auto topRight = existingFace->vertices[(size_t)Corner::topLeft];
        auto bottomRight = existingFace->vertices[(size_t)Corner::bottomLeft];

        auto existingTopEdge = existingFace->edges[(size_t)Direction::north]->toLine();
        auto topLeftPosition = existingTopEdge.getPointAlongLineProportionally(-1.0f);

        auto existingBottomEdge = existingFace->edges[(size_t)Direction::south]->toLine();
        auto bottomLeftPosition = existingBottomEdge.getPointAlongLineProportionally(-1.0f);

        auto topLeft = pimpl->addVertex(topLeftPosition);
        auto bottomLeft = pimpl->addVertex(bottomLeftPosition);

        auto topEdge = pimpl->addEdge(Orientation::horizontal, topLeft, topRight);
        auto rightEdge = existingFace->edges[(size_t)Direction::west];
        auto bottomEdge = pimpl->addEdge(Orientation::horizontal, bottomLeft, bottomRight);
        auto leftEdge = pimpl->addEdge(Orientation::vertical, topLeft, bottomLeft);

        auto face = pimpl->addFace(topLeft, topRight, bottomRight, bottomLeft,
            topEdge, rightEdge, bottomEdge, leftEdge);
        face->colors = colors;
        face->setInnerControlPoints();

        return patches.add(new Patch{ face });
    }
    }

    return nullptr;
}
#endif

GradientMesh::Patch::Ptr GradientMesh::addPatch(PatchOptions& options)
{
    auto toPoint2F = [&](juce::Point<float> p)
        {
            return D2D1::Point2F(p.x, p.y);
        };

    auto patch = patches.add(new Patch{ options, *this });

#if 0
    auto& d2dPatch = patch->pimpl->d2dPatch;

    d2dPatch.point00 = toPoint2F(options.upperLeftCorner.position);
    d2dPatch.point03 = toPoint2F(options.upperRightCorner.position);
    d2dPatch.point30 = toPoint2F(options.lowerLeftCorner.position);
    d2dPatch.point33 = toPoint2F(options.lowerRightCorner.position);

    d2dPatch.point01 = toPoint2F(options.topEdge.leftControlPoint);
    d2dPatch.point02 = toPoint2F(options.topEdge.rightControlPoint);

    d2dPatch.point10 = toPoint2F(options.leftEdge.upperControlPoint);
    d2dPatch.point20 = toPoint2F(options.leftEdge.lowerControlPoint);

    d2dPatch.point13 = toPoint2F(options.rightEdge.upperControlPoint);
    d2dPatch.point23 = toPoint2F(options.rightEdge.lowerControlPoint);

    d2dPatch.point31 = toPoint2F(options.bottomEdge.leftControlPoint);
    d2dPatch.point32 = toPoint2F(options.bottomEdge.rightControlPoint);

    juce::Line<float> l1{ options.upperLeftCorner.position, options.lowerRightCorner.position };
    d2dPatch.point11 = toPoint2F(l1.getPointAlongLineProportionally(0.5f));
    d2dPatch.point12 = toPoint2F(l1.getPointAlongLineProportionally(0.5f));
    d2dPatch.point21 = toPoint2F(l1.getPointAlongLineProportionally(0.5f));
    d2dPatch.point22 = toPoint2F(l1.getPointAlongLineProportionally(0.5f));

    d2dPatch.color00 = juce::D2DUtilities::toCOLOR_F(options.upperLeftCorner.color);
    d2dPatch.color03 = juce::D2DUtilities::toCOLOR_F(options.upperRightCorner.color);
    d2dPatch.color30 = juce::D2DUtilities::toCOLOR_F(options.lowerLeftCorner.color);
    d2dPatch.color33 = juce::D2DUtilities::toCOLOR_F(options.lowerRightCorner.color);
#endif

    return patch;
}

GradientMesh::Patch::Ptr GradientMesh::clonePatch(Patch::Ptr originalPatch, Direction direction)
{
    auto toPoint = [&](D2D1_POINT_2F p)
        {
            return juce::Point<float>{ p.x, p.y };
        };

    auto clone = patches.add(new Patch{ *originalPatch });

    clone->options.gridCoordinates = originalPatch->options.gridCoordinates.translated(direction);

    return clone;
}

GradientMesh::Patch::Patch(PatchOptions& options_, GradientMesh& mesh_) :
    pimpl(std::make_unique<PatchPimpl>(*this)),
    mesh(mesh_),
    options(options_)
{
}

GradientMesh::Patch::Patch(Patch const& other) :
    pimpl(std::make_unique<PatchPimpl>(*this)),
    mesh(other.mesh),
    options(other.options)
{
}

GradientMesh::Patch::~Patch()
{
}

juce::Rectangle<float> GradientMesh::Patch::getBounds() const noexcept
{
    //     juce::Rectangle<float> r1{ pimpl->d2dPatch.point00.x, pimpl->d2dPatch.point00.y, pimpl->d2dPatch.point33.x, pimpl->d2dPatch.point33.y };
    //     juce::Rectangle<float> r2{ pimpl->d2dPatch.point03.x, pimpl->d2dPatch.point03.y, pimpl->d2dPatch.point30.x, pimpl->d2dPatch.point30.y };
    //
    //     return r1.getUnion(r2);

    return {};

#if 0
    float x = std::numeric_limits<float>::max();
    float y = std::numeric_limits<float>::max();
    float right = 0.0f;
    float bottom = 0.0f;
    for (auto& vertex : face->vertices)
    {
        x = juce::jmin(x, vertex->position.x);
        y = juce::jmin(y, vertex->position.y);
        right = juce::jmax(right, vertex->position.x);
        bottom = juce::jmax(bottom, vertex->position.y);
    }

    return juce::Rectangle<float>::leftTopRightBottom(x, y, right, bottom);
#endif

#if 0
    auto iter = pimpl->controlPoints.begin();
    auto first = iter->position;
    iter++;
    auto second = iter->position;
    iter++;

    juce::Rectangle<float> r{ first, second };
    while (iter != pimpl->controlPoints.end())
    {
        auto p = iter->position;
        iter++;

        r = r.getUnion(juce::Rectangle<float>{ first, p });
    }

    return r;
#endif

    return {};
}

void GradientMesh::Patch::translate(float x, float y)
{
    auto translation = juce::Point<float>{ x, y };

    options.upperLeftCorner.position += translation;
    options.upperRightCorner.position += translation;
    options.lowerLeftCorner.position += translation;
    options.lowerRightCorner.position += translation;
    options.leftEdge.upperControlPoint += translation;
    options.leftEdge.lowerControlPoint += translation;
    options.rightEdge.upperControlPoint += translation;
    options.rightEdge.lowerControlPoint += translation;
    options.topEdge.leftControlPoint += translation;
    options.topEdge.rightControlPoint += translation;
    options.bottomEdge.leftControlPoint += translation;
    options.bottomEdge.rightControlPoint += translation;
}

void GradientMesh::Patch::applyTransform(juce::AffineTransform transform)
{
    options.upperLeftCorner.position.applyTransform(transform);
    options.upperRightCorner.position.applyTransform(transform);
    options.lowerLeftCorner.position.applyTransform(transform);
    options.lowerRightCorner.position.applyTransform(transform);
    options.leftEdge.upperControlPoint.applyTransform(transform);
    options.leftEdge.lowerControlPoint.applyTransform(transform);
    options.rightEdge.upperControlPoint.applyTransform(transform);
    options.rightEdge.lowerControlPoint.applyTransform(transform);
    options.topEdge.leftControlPoint.applyTransform(transform);
    options.topEdge.rightControlPoint.applyTransform(transform);
    options.bottomEdge.leftControlPoint.applyTransform(transform);
    options.bottomEdge.rightControlPoint.applyTransform(transform);
}

void GradientMesh::Patch::flipControlPointsHorizontally()
{
    std::swap(options.upperLeftCorner.position, options.upperRightCorner.position);
    std::swap(options.lowerLeftCorner.position, options.lowerRightCorner.position);
    std::swap(options.leftEdge, options.rightEdge);
    std::swap(options.topEdge.leftControlPoint, options.topEdge.rightControlPoint);
    std::swap(options.bottomEdge.leftControlPoint, options.bottomEdge.rightControlPoint);
}

void GradientMesh::Patch::flipColorsHorizontally()
{
    std::swap(options.upperLeftCorner.color, options.upperRightCorner.color);
    std::swap(options.lowerLeftCorner.color, options.lowerRightCorner.color);
}

void GradientMesh::Patch::flipControlPointsVertically()
{
    std::swap(options.upperLeftCorner.position, options.lowerLeftCorner.position);
    std::swap(options.upperRightCorner.position, options.lowerRightCorner.position);
    std::swap(options.topEdge, options.bottomEdge);
    std::swap(options.leftEdge.upperControlPoint, options.leftEdge.lowerControlPoint);
    std::swap(options.rightEdge.upperControlPoint, options.rightEdge.lowerControlPoint);
}

void GradientMesh::Patch::flipColorsVertically()
{
    std::swap(options.upperLeftCorner.color, options.lowerLeftCorner.color);
    std::swap(options.upperRightCorner.color, options.lowerRightCorner.color);
}

void GradientMesh::Patch::setUpperLeftColor(juce::Colour color)
{
    options.upperLeftCorner.color = color;
}

void GradientMesh::Patch::setUpperRightColor(juce::Colour color)
{
    options.upperRightCorner.color = color;
}

void GradientMesh::Patch::setLowerLeftColor(juce::Colour color)
{
    options.lowerLeftCorner.color = color;
}

void GradientMesh::Patch::setLowerRightColor(juce::Colour color)
{
    options.lowerRightCorner.color = color;
}
