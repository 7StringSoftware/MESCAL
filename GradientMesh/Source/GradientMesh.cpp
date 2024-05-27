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

std::shared_ptr<GradientMesh::Patch> GradientMesh::Patch::create()
{
    auto patch = std::make_shared<Patch>();

    juce::Rectangle<float> r{ 100.0f, 100.0f };

    patch->getControlPoint(0, 0)->setPosition(r.getTopLeft());
    patch->getControlPoint(0, 1)->setPosition(r.getTopLeft() + juce::Point<float>{ 25.0f, -10.0f });
    patch->getControlPoint(0, 2)->setPosition(r.getTopLeft() + juce::Point<float>{ 75.0f, 10.0f });

    patch->getControlPoint(0, 3)->setPosition(r.getTopRight());
    patch->getControlPoint(1, 3)->setPosition(r.getTopRight() + juce::Point<float>{ 10.0f, 25.0f });
    patch->getControlPoint(2, 3)->setPosition(r.getTopRight() + juce::Point<float>{ -10.0f, 75.0f });

    patch->getControlPoint(3, 3)->setPosition(r.getBottomRight());
    patch->getControlPoint(3, 2)->setPosition(r.getBottomRight() + juce::Point<float>{ -25.0f, 10.0f });
    patch->getControlPoint(3, 1)->setPosition(r.getBottomRight() + juce::Point<float>{ -75.0f, -10.0f });

    patch->getControlPoint(3, 0)->setPosition(r.getBottomLeft());
    patch->getControlPoint(2, 0)->setPosition(r.getBottomLeft() + juce::Point<float>{ -10.0f, -25.0f });
    patch->getControlPoint(1, 0)->setPosition(r.getBottomLeft() + juce::Point<float>{ 10.0f, -75.0f });

    patch->getControlPoint(1, 1)->setPosition(patch->getControlPoint(0, 0)->getPosition());
    patch->getControlPoint(1, 2)->setPosition(patch->getControlPoint(0, 3)->getPosition());
    patch->getControlPoint(2, 1)->setPosition(patch->getControlPoint(3, 0)->getPosition());
    patch->getControlPoint(2, 2)->setPosition(patch->getControlPoint(3, 3)->getPosition());

    patch->getControlPoint(0, 0)->setColor(juce::Colours::red);
    patch->getControlPoint(0, 3)->setColor(juce::Colours::green);
    patch->getControlPoint(3, 0)->setColor(juce::Colours::blue);
    patch->getControlPoint(3, 3)->setColor(juce::Colours::yellow);

    return patch;
}

std::shared_ptr<GradientMesh::Patch> GradientMesh::Patch::createConnectedPatch(size_t edgePosition) const
{
    auto patch = std::make_shared<Patch>();

    juce::Point<float> delta;

    for (int row = 0; row < numRows; ++row)
    {
        for (int column = 0; column < numColumns; ++column)
        {
            auto sourcePoint = getControlPoint(row, column);
            auto destPoint = patch->getControlPoint(row, column);
            destPoint->setPosition(sourcePoint->getPosition());
            if (sourcePoint->hasColor())
            {
                destPoint->setColor(sourcePoint->getColor());
            }
        }
    }

    switch (edgePosition)
    {
    case Edge::top:
    {
        delta = getControlPoint(0, 0)->getPosition() - getControlPoint(3, 0)->getPosition();
        break;
    }

    case Edge::right:
    {
        delta = getControlPoint(0, 3)->getPosition() - getControlPoint(0, 0)->getPosition();
        break;
    }

    case Edge::bottom:
    {
        delta = getControlPoint(3, 0)->getPosition() - getControlPoint(0, 0)->getPosition();
        break;
    }

    case Edge::left:
    {
        delta = getControlPoint(0, 0)->getPosition() - getControlPoint(0, 3)->getPosition();
        break;
    }
    }

    patch->applyTransform(juce::AffineTransform::translation(delta));

    switch (edgePosition)
    {
    case Edge::top:
    {
        int destRow = 3;
        int sourceRow = 0;
        for (int column = 0; column < numColumns; ++column)
        {
            auto sourcePoint = getControlPoint(sourceRow, column);
            auto destPoint = patch->getControlPoint(destRow, column);
            destPoint->setPosition(sourcePoint->getPosition());

            sourcePoint->addNeighbor(destPoint);
            destPoint->addNeighbor(sourcePoint);

            if (sourcePoint->hasColor())
            {
                destPoint->setColor(sourcePoint->getColor());
            }
        }
        break;
    }

    case Edge::right:
    {
        int destColumn = 0;
        int sourceColumn = 3;
        for (int row = 0; row < numRows; ++row)
        {
            auto sourcePoint = getControlPoint(row, sourceColumn);
            auto destPoint = patch->getControlPoint(row, destColumn);
            destPoint->setPosition(sourcePoint->getPosition());

            sourcePoint->addNeighbor(destPoint);
            destPoint->addNeighbor(sourcePoint);

            if (sourcePoint->hasColor())
            {
                destPoint->setColor(sourcePoint->getColor());
            }
        }
        break;
    }

    case Edge::bottom:
    {
        int destRow = 0;
        int sourceRow = 3;
        for (int column = 0; column < numColumns; ++column)
        {
            auto sourcePoint = getControlPoint(sourceRow, column);
            auto destPoint = patch->getControlPoint(destRow, column);
            destPoint->setPosition(sourcePoint->getPosition());

            sourcePoint->addNeighbor(destPoint);
            destPoint->addNeighbor(sourcePoint);

            if (sourcePoint->hasColor())
            {
                destPoint->setColor(sourcePoint->getColor());
            }
        }
        break;
    }

    case Edge::left:
    {
        int destColumn = 3;
        int sourceColumn = 0;
        for (int row = 0; row < numRows; ++row)
        {
            auto sourcePoint = getControlPoint(row, sourceColumn);
            auto destPoint = patch->getControlPoint(row, destColumn);
            destPoint->setPosition(sourcePoint->getPosition());

            sourcePoint->addNeighbor(destPoint);
            destPoint->addNeighbor(sourcePoint);

            if (sourcePoint->hasColor())
            {
                destPoint->setColor(sourcePoint->getColor());
            }
        }
        break;
    }
    }

    return patch;
}

void GradientMesh::Patch::applyTransform(const AffineTransform& transform) noexcept
{
    for (auto& controlPoint : controlPoints)
    {
        controlPoint->applyTransform(transform);
    }

    update();
}

void GradientMesh::Patch::flipHorizontally()
{
    for (size_t row = 0; row < numRows; ++row)
    {
        for (size_t column = 0; column < numColumns / 2; ++column)
        {
            getControlPoint(row, column)->swapWith(getControlPoint(row, numColumns - 1 - column));
        }
    }
}

void GradientMesh::addPatch(std::shared_ptr<Patch> patch)
{
    patch->update();

    patches.push_back(patch);
}

void GradientMesh::applyTransform(const AffineTransform& transform) noexcept
{
    for (auto& patch : patches)
    {
        patch->applyTransform(transform);
    }
}

void GradientMesh::draw(juce::Image image, juce::AffineTransform transform)
{
    std::vector<D2D1_GRADIENT_MESH_PATCH> d2dPatches;
    for (auto const& patch : patches)
    {
        d2dPatches.emplace_back(D2D1_GRADIENT_MESH_PATCH{});
        auto& d2dPatch = d2dPatches.back();

        auto convertPoint = [&](int row, int column)
            {
                auto p = patch->getControlPoint(row, column)->getPosition().transformedBy(transform);
                return D2D1_POINT_2F{ p.x, p.y };
            };

        auto convertEdge = [&](Edge::Type edgeType,
            D2D1_POINT_2F& cp0,
            D2D1_POINT_2F& cp1,
            D2D1_POINT_2F& corner0,
            D2D1_POINT_2F& corner1)
            {
                switch (edgeType)
                {
                case Edge::Type::straight:
                    cp0 = corner0;
                    cp1 = corner1;
                    break;

                case Edge::Type::quadratic:
                    cp1 = cp0;
                    break;
                }
            };

        d2dPatch.point00 = convertPoint(0, 0);
        d2dPatch.point01 = convertPoint(0, 1);
        d2dPatch.point02 = convertPoint(0, 2);
        d2dPatch.point03 = convertPoint(0, 3);
        d2dPatch.point10 = convertPoint(1, 0);
        d2dPatch.point11 = convertPoint(1, 1);
        d2dPatch.point12 = convertPoint(1, 2);
        d2dPatch.point13 = convertPoint(1, 3);
        d2dPatch.point20 = convertPoint(2, 0);
        d2dPatch.point21 = convertPoint(2, 1);
        d2dPatch.point22 = convertPoint(2, 2);
        d2dPatch.point23 = convertPoint(2, 3);
        d2dPatch.point30 = convertPoint(3, 0);
        d2dPatch.point31 = convertPoint(3, 1);
        d2dPatch.point32 = convertPoint(3, 2);
        d2dPatch.point33 = convertPoint(3, 3);

        convertEdge(patch->getEdgeType(Edge::top),
            d2dPatch.point01, d2dPatch.point02, d2dPatch.point00, d2dPatch.point03);
        convertEdge(patch->getEdgeType(Edge::right),
            d2dPatch.point13, d2dPatch.point23, d2dPatch.point03, d2dPatch.point33);
        convertEdge(patch->getEdgeType(Edge::bottom),
            d2dPatch.point32, d2dPatch.point31, d2dPatch.point33, d2dPatch.point30);
        convertEdge(patch->getEdgeType(Edge::left),
            d2dPatch.point20, d2dPatch.point10, d2dPatch.point30, d2dPatch.point00);

        auto convertColor = [&](int row, int column)
            {
                return D2DUtilities::toCOLOR_F(patch->getControlPoint(row, column)->getColor());
            };

        d2dPatch.color00 = convertColor(0, 0);
        d2dPatch.color03 = convertColor(0, 3);
        d2dPatch.color30 = convertColor(3, 0);
        d2dPatch.color33 = convertColor(3, 3);
    }

    pimpl->createResources(image);

    if (pimpl->deviceContext && image.isValid())
    {
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

juce::Rectangle<float> GradientMesh::getBounds() const noexcept
{
    juce::Rectangle<float> bounds;

    for (auto const& patch : patches)
    {
        bounds = bounds.getUnion(patch->getPath().getBounds());
    }

    return bounds;
}

GradientMesh::Patch::Patch()
{
    int row = 0, column = 0;
    for (auto& controlPoint : controlPoints)
    {
        controlPoint = std::make_shared<ControlPoint>(*this, row, column);
        ++column;
        if (column == numColumns)
        {
            column = 0;
            ++row;
        }
    }
}

void GradientMesh::Patch::update()
{
    auto getPosition = [&](int row, int column)
        {
            return getControlPoint(row, column)->getPosition();
        };

    path.clear();
    path.startNewSubPath(getPosition(0, 0));

    for (size_t position = Edge::top; position <= Edge::left; ++position)
    {
        auto edge = getEdge(position);
        switch (edge.type)
        {
        case Edge::Type::straight:
            path.lineTo(edge.corners.second->getPosition());
            break;

        case Edge::Type::quadratic:
            path.cubicTo(edge.bezierControlPoints.first->getPosition(),
                edge.bezierControlPoints.first->getPosition(),
                edge.corners.second->getPosition());
            break;

        case Edge::Type::cubic:
            path.cubicTo(edge.bezierControlPoints.first->getPosition(),
                edge.bezierControlPoints.second->getPosition(),
                edge.corners.second->getPosition());
            break;
        }
    }

    path.closeSubPath();
}

GradientMesh::Edge GradientMesh::Patch::getEdge(size_t edge) const
{
    switch (edge)
    {
    case Edge::top:
        return Edge
        {
            edgeTypes[Edge::top],
            { getControlPoint(0, 0), getControlPoint(0, 3) },
            { getControlPoint(0, 1), getControlPoint(0, 2) }
        };

    case Edge::right:
        return Edge
        {
            edgeTypes[Edge::right],
            { getControlPoint(0, 3), getControlPoint(3, 3) },
            { getControlPoint(1, 3), getControlPoint(2, 3) }
        };

    case Edge::bottom:
        return Edge
        {
            edgeTypes[Edge::bottom],
            { getControlPoint(3, 3), getControlPoint(3, 0) },
            { getControlPoint(3, 2), getControlPoint(3, 1) }
        };

    case Edge::left:
        return Edge
        {
            edgeTypes[Edge::left],
            { getControlPoint(3, 0), getControlPoint(0, 0) },
            { getControlPoint(2, 0), getControlPoint(1, 0) }
        };
    }

    return Edge{};
}

void GradientMesh::ControlPoint::setPosition(juce::Point<float> position_)
{
    position = position_;
    patch.update();

    for (auto& neighbor : neighbors)
    {
        neighbor->position = position_;
        neighbor->patch.update();
    }
}

void GradientMesh::ControlPoint::applyTransform(const AffineTransform& transform) noexcept
{
    position.applyTransform(transform);
}

void GradientMesh::ControlPoint::swapWith(std::shared_ptr<ControlPoint> other) noexcept
{
    std::swap(position, other->position);

    if (hasColor())
    {
        std::swap(*color, *other->color);
    }
}
