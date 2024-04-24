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

struct GradientMesh::Patch::PatchPimpl
{
    PatchPimpl(juce::Rectangle<float> area)
    {
        GridPosition gridPosition;
        juce::Point<float> position = area.getTopLeft();
        float y = area.getY();
        for (int i = 0; i < numControlPoints; ++i)
        {
            controlPoints.emplace_back(ControlPoint{ gridPosition, position });

            gridPosition.column++;
            position += { area.getWidth() / (float)(numColumns - 1), 0.0f };
            if (gridPosition.column >= numColumns)
            {
                gridPosition.column = 0;
                gridPosition.row++;
                y += area.getHeight() / (float)(numRows - 1);
                position = { area.getX(), y };
            }
        }

        for (auto const& outerCorner : outerCorners)
        {
            auto& controlPoint = getControlPoint(outerCorner.gridPosition);
            controlPoint.color = outerCorner.defaultColor;
            controlPoint.edgeFlags = outerCorner.edgeFlags;

            auto& clockwiseControlPoint = getControlPoint(outerCorner.clockwiseCubicSpineControl);
            clockwiseControlPoint.edgeFlags = outerCorner.clockwiseEdgeFlags;

            auto& counterclockwiseControlPoint = getControlPoint(outerCorner.counterclockWiseCubicSplineControl);
            counterclockwiseControlPoint.edgeFlags = outerCorner.counterclockwiseEdgeFlags;
        }
    }

    ~PatchPimpl()
    {
    }

    enum class EdgeFlags
    {
        unknown = 0,
        leftEdge = 1,
        topEdge = 2,
        rightEdge = 4,
        bottomEdge = 8,

        topLeftCorner = leftEdge | topEdge,
        topRightCorner = topEdge | rightEdge,
        bottomRightCorner = rightEdge | bottomEdge,
        bottomLeftCorner = bottomEdge | leftEdge
    };

    struct OuterCorner
    {
        GridPosition gridPosition;
        EdgeFlags edgeFlags;
        
        GridPosition clockwiseCubicSpineControl;
        EdgeFlags clockwiseEdgeFlags;

        GridPosition counterclockWiseCubicSplineControl;
        EdgeFlags counterclockwiseEdgeFlags;
        
        juce::Colour defaultColor;
    };

    std::array<OuterCorner, 4> const outerCorners
    {
        OuterCorner{ { 0, 0 }, EdgeFlags::topLeftCorner,
            { 0, 1 }, EdgeFlags::topEdge,
            { 1, 0 }, EdgeFlags::leftEdge,
            juce::Colours::red }, // top left

        OuterCorner{ { 0, 3 }, EdgeFlags::topRightCorner,
            { 1, 3 }, EdgeFlags::rightEdge,
            { 0, 2 }, EdgeFlags::topEdge,
            juce::Colours::yellow }, // top right

        OuterCorner{ { 3, 3 }, EdgeFlags::bottomRightCorner,
            { 3, 2 }, EdgeFlags::bottomEdge,
            { 2, 3 }, EdgeFlags::rightEdge,
            juce::Colours::indigo }, // bottom right

        OuterCorner{ { 3, 0 }, EdgeFlags::bottomLeftCorner,
            { 3, 1 }, EdgeFlags::bottomEdge,
            { 2, 0 }, EdgeFlags::leftEdge,
            juce::Colours::violet } // bottom left
    };

    struct ControlPoint
    {
        GridPosition const gridPosition;
        juce::Point<float> position{};
        std::optional<juce::Colour> color;
        EdgeFlags edgeFlags = EdgeFlags::unknown;
    };

    ControlPoint& getControlPoint(GridPosition gridPosition)
    {
        return controlPoints[gridPosition.row * numColumns + gridPosition.column];
    }

    D2D1_POINT_2F getControlPointPositionPOINT2F(GridPosition gridPosition)
    {
        auto p = getControlPoint(gridPosition).position;
        return D2D1::Point2F(p.x, p.y);
    }

    void toD2DPatch(D2D1_GRADIENT_MESH_PATCH* d2dPatch)
    {
        d2dPatch->point00 = getControlPointPositionPOINT2F({ 0, 0 });
        d2dPatch->point01 = getControlPointPositionPOINT2F({ 0, 1 });
        d2dPatch->point02 = getControlPointPositionPOINT2F({ 0, 2 });
        d2dPatch->point03 = getControlPointPositionPOINT2F({ 0, 3 });
        d2dPatch->point10 = getControlPointPositionPOINT2F({ 1, 0 });
        d2dPatch->point11 = getControlPointPositionPOINT2F({ 1, 1 });
        d2dPatch->point12 = getControlPointPositionPOINT2F({ 1, 2 });
        d2dPatch->point13 = getControlPointPositionPOINT2F({ 1, 3 });
        d2dPatch->point20 = getControlPointPositionPOINT2F({ 2, 0 });
        d2dPatch->point21 = getControlPointPositionPOINT2F({ 2, 1 });
        d2dPatch->point22 = getControlPointPositionPOINT2F({ 2, 2 });
        d2dPatch->point23 = getControlPointPositionPOINT2F({ 2, 3 });
        d2dPatch->point30 = getControlPointPositionPOINT2F({ 3, 0 });
        d2dPatch->point31 = getControlPointPositionPOINT2F({ 3, 1 });
        d2dPatch->point32 = getControlPointPositionPOINT2F({ 3, 2 });
        d2dPatch->point33 = getControlPointPositionPOINT2F({ 3, 3 });

        auto getColorF = [&](GridPosition gridPosition)
            {
                auto color = getControlPoint(gridPosition).color;
                jassert(color.has_value());
                return juce::D2DUtilities::toCOLOR_F(*color);
            };

        d2dPatch->color00 = getColorF(GridPosition{ 0, 0 });
        d2dPatch->color03 = getColorF(GridPosition{ 0, 3 });
        d2dPatch->color30 = getColorF(GridPosition{ 3, 0 });
        d2dPatch->color33 = getColorF(GridPosition{ 3, 3 });

        d2dPatch->leftEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
        d2dPatch->topEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
        d2dPatch->rightEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
        d2dPatch->bottomEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
    }

    std::vector<ControlPoint> controlPoints;
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

    juce::HeapBlock< D2D1_GRADIENT_MESH_PATCH> d2dPatches;
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

void GradientMesh::setControlPointPosition(Patch::Ptr patch, GridPosition gridPosition, juce::Point<float> pos)
{
    patch->setControlPointPosition(gridPosition, pos);
    pimpl->gradientMesh = nullptr;
}

void GradientMesh::setControlPointColor(Patch::Ptr patch, GridPosition gridPosition, juce::Colour color)
{
    patch->setControlPointColor(gridPosition, color);
    pimpl->gradientMesh = nullptr;
}

void GradientMesh::updateMesh()
{
    if (pimpl->deviceContext && !pimpl->gradientMesh)
    {
        D2D1_GRADIENT_MESH_PATCH* d2d1Patch = pimpl->d2dPatches.getData();

        memset(d2d1Patch, 0, sizeof(D2D1_GRADIENT_MESH_PATCH) * patches.size());

        for (auto const& patch : patches)
        {
            patch->pimpl->toD2DPatch(d2d1Patch);
            d2d1Patch++;
        }

        pimpl->deviceContext->CreateGradientMesh(pimpl->d2dPatches.getData(), patches.size(), pimpl->gradientMesh.put());
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

void GradientMesh::addPatch(juce::Rectangle<float> area)
{
    auto existingPatch = patches.getLast();
    auto newPatch = patches.add(new Patch{ area });

    if (existingPatch)
    {
        existingPatch->rightNeighbor = newPatch;
        newPatch->leftNeighbor = existingPatch.get();
    }

    pimpl->d2dPatches.realloc(patches.size());
}

GradientMesh::Patch::Patch(juce::Rectangle<float> area) :
    pimpl(std::make_unique<PatchPimpl>(area))
{
}

GradientMesh::Patch::~Patch()
{
}

juce::Rectangle<float> GradientMesh::Patch::getBounds() const noexcept
{
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
}

juce::Point<float> GradientMesh::Patch::getControlPointPosition(GridPosition gridPosition) const
{
    return pimpl->getControlPoint(gridPosition).position;
}

void GradientMesh::Patch::setControlPointPosition(GridPosition gridPosition, juce::Point<float> pos)
{
    pimpl->getControlPoint(gridPosition).position = pos;
    if (rightNeighbor)
    {
        auto neighborPos = rightNeighbor->getControlPointPosition(gridPosition);
        if (!approximatelyEqual(pos, neighborPos))
        {
            rightNeighbor->setControlPointPosition(gridPosition, pos);
        }
    }
}

std::optional<juce::Colour> GradientMesh::Patch::getControlPointColor(GridPosition gridPosition) const
{
    return pimpl->getControlPoint(gridPosition).color;
}

void GradientMesh::Patch::setControlPointColor(GridPosition gridPosition, juce::Colour color)
{
    pimpl->getControlPoint(gridPosition).color = color;
}
