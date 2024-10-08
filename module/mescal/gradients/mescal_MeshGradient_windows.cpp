namespace mescal
{

    /*

        D2D1_GRADIENT_MESH_PATCH

        P00  Top left corner
        P03  Top right corner
        P30  Bottom left corner
        P33  Bottom right corner

        P01  Top edge control point #1
        P02  Top edge control point #1

        P10  Left edge control point #1
        P20  Left edge control point #1

        P13  Right edge control point #1
        P23  Right edge control point #1

        P31  Bottom edge control point #1
        P32  Bottom edge control point #1

        P11  Top left corner (inner)
        P12  Top right corner (inner)
        P21  Bottom left corner (inner)
        P22  Bottom right corner (inner)


                P01
            /
            P00--------------------P03
        / |                    / | \
        P10  |                  P02 |  P13
            |                      |
            |     P11     P12      |
            |                      |
            |     P21     P22      |
            |                      |
            | P20                  |
            | /   P31          P23 |
            |/   /               \ |
            P30--------------------P33
                                /
                            P32

    */

#ifdef __INTELLISENSE__

#include "mescal_GradientMesh_windows.h"

#endif

    struct MeshGradient::Pimpl
    {
        Pimpl(MeshGradient& owner_) : owner(owner_)
        {
        }

        void createResources()
        {
            resources->create();
        }

        MeshGradient& owner;
        juce::SharedResourcePointer<DirectXResources> resources;
        winrt::com_ptr<ID2D1GradientMesh> gradientMesh;
    };


    MeshGradient::MeshGradient(int numRows_, int numColumns_, std::optional<juce::Rectangle<float>> bounds) :
        pimpl(std::make_unique<Pimpl>(*this)),
        numRows(numRows_),
        numColumns(numColumns_)
    {
        jassert(numRows_ > 1);
        jassert(numColumns_ > 1);

        float rowHeight = 1.0f;
        float columnWidth = 1.0f;
        auto numVertices = numRows_ * numColumns_;
        vertices.clear();
        vertices.reserve(numVertices);
        float startX = 0.0f, startY = 0.0f;

        if (bounds.has_value())
        {
            startX = bounds->getX();
            startY = bounds->getY();
            rowHeight = bounds->getHeight() / (numRows_ - 1);
            columnWidth = bounds->getWidth() / (numColumns_ - 1);
        }

        for (int row = 0; row < numRows_; ++row)
        {
            for (int column = 0; column < numColumns_; ++column)
            {
                auto x = column * columnWidth + startX;
                auto y = row * rowHeight + startY;
                vertices.push_back(std::make_shared<Vertex>(*this, row, column, juce::Point<float>{x, y}));
            }
        }
    }

    MeshGradient::~MeshGradient()
    {
    }

    void MeshGradient::applyTransform([[maybe_unused]] juce::AffineTransform const& transform)
    {
        jassertfalse;
    }

    std::shared_ptr<MeshGradient::Vertex> MeshGradient::getVertex(int row, int column)
    {
        if (row < 0 || row >= numRows || column < 0 || column >= numColumns)
            return {};

        return vertices[row * numColumns + column];
    }

    std::shared_ptr<MeshGradient::Vertex> MeshGradient::Vertex::getAdjacentVertex(Placement placement) const
    {
        switch (placement)
        {
        case Placement::top:
            return owner.getVertex(row - 1, column);

        case Placement::left:
            return owner.getVertex(row, column - 1);

        case Placement::bottom:
            return owner.getVertex(row + 1, column);

        case Placement::right:
            return owner.getVertex(row, column + 1);
        }

        jassertfalse;
        return {};
    }

    std::optional<juce::Point<float>> MeshGradient::Vertex::BezierControlPoints::getControlPoint(Placement placement) const
    {
        switch (placement)
        {
        case Placement::top:
            return topControlPoint;

        case Placement::left:
            return leftControlPoint;

        case Placement::bottom:
            return bottomControlPoint;

        case Placement::right:
            return rightControlPoint;
        }

        jassertfalse;
        return {};
    }

    void MeshGradient::Vertex::BezierControlPoints::setControlPoint(Placement placement, juce::Point<float> point)
    {
        switch (placement)
        {
        case Placement::top:
            topControlPoint = point;
            break;

        case Placement::left:
            leftControlPoint = point;
            break;

        case Placement::bottom:
            bottomControlPoint = point;
            break;

        case Placement::right:
            rightControlPoint = point;
            break;

        default:
            jassertfalse;
            break;
        }
    }

    std::optional<juce::Point<float>> MeshGradient::Vertex::InteriorControlPoints::getControlPoint(Placement placement) const
    {
        switch (placement)
        {
        case Placement::topLeft:
            return topLeftControlPoint;

        case Placement::bottomLeft:
            return bottomLeftControlPoint;

        case Placement::bottomRight:
            return bottomRightControlPoint;

        case Placement::topRight:
            return topRightControlPoint;
        }

        jassertfalse;
        return {};
    }

    void MeshGradient::Vertex::InteriorControlPoints::setControlPoint(Placement placement, juce::Point<float> point)
    {
        switch (placement)
        {
        case Placement::topLeft:
            topLeftControlPoint = point;
            break;

        case Placement::bottomLeft:
            bottomLeftControlPoint = point;
            break;

        case Placement::bottomRight:
            bottomRightControlPoint = point;
            break;

        case MeshGradient::Placement::topRight:
            topRightControlPoint = point;
            break;
        }
    }

    void MeshGradient::draw(juce::Image image, juce::AffineTransform transform, juce::Colour backgroundColor)
    {
        auto vertexToPOINT_2F = [&](int row, int column)
            {
                auto vertex = getVertex(row, column);
                auto transformedPoint = vertex->position.transformedBy(transform);
                return D2D1_POINT_2F{ transformedPoint.x, transformedPoint.y };
            };

        auto bezierToPOINT_2F = [&](std::shared_ptr<Vertex> vertex, Placement placement, D2D1_POINT_2F& point2F)
            {
                auto controlPoint = vertex->bezier.getControlPoint(placement);
                if (controlPoint.has_value())
                {
                    auto transformedPoint = controlPoint.value().transformedBy(transform);
                    point2F = D2D1_POINT_2F{ transformedPoint.x, transformedPoint.y };
                }
                else
                {
                    point2F = D2D1_POINT_2F{ vertex->position.x, vertex->position.y };
                }
            };

        auto interiorToPOINT_2F = [&](std::shared_ptr<Vertex> vertex, Placement placement, D2D1_POINT_2F& point2F)
            {
                auto controlPoint = vertex->interior.getControlPoint(placement);
                if (controlPoint.has_value())
                {
                    auto transformedPoint = controlPoint.value().transformedBy(transform);
                    point2F = D2D1_POINT_2F{ transformedPoint.x, transformedPoint.y };
                }
                else
                {
                    point2F = D2D1_POINT_2F{ vertex->position.x, vertex->position.y };
                }
            };

        auto pointToPOINT_2F = [&](juce::Point<float> p)
            {
                p = p.transformedBy(transform);
                return D2D1_POINT_2F{ p.x, p.y };
            };

        auto toCOLOR_F = [](Color128 color)
            {
                return D2D1::ColorF(color.red, color.green, color.blue, color.alpha);
            };

        std::vector<D2D1_GRADIENT_MESH_PATCH> d2dPatches;
        d2dPatches.reserve(numRows * numColumns);
        for (int row = 0; row < numRows - 1; ++row)
        {
            for (int column = 0; column < numColumns - 1; ++column)
            {
                d2dPatches.emplace_back(D2D1_GRADIENT_MESH_PATCH{});
                auto& d2dPatch = d2dPatches.back();
                auto vertex = getVertex(row, column);

                auto topLeft = getVertex(row, column);
                auto topRight = getVertex(row, column + 1);
                auto bottomRight = getVertex(row + 1, column + 1);
                auto bottomLeft = getVertex(row + 1, column);

                d2dPatch.point00 = pointToPOINT_2F(topLeft->position);
                d2dPatch.point03 = pointToPOINT_2F(topRight->position);
                d2dPatch.point30 = pointToPOINT_2F(bottomLeft->position);
                d2dPatch.point33 = pointToPOINT_2F(bottomRight->position);

                bezierToPOINT_2F(topLeft, Placement::right, d2dPatch.point01);
                bezierToPOINT_2F(topLeft, Placement::bottom, d2dPatch.point10);

                bezierToPOINT_2F(topRight, Placement::left, d2dPatch.point02);
                bezierToPOINT_2F(topRight, Placement::bottom, d2dPatch.point13);

                bezierToPOINT_2F(bottomLeft, Placement::top, d2dPatch.point20);
                bezierToPOINT_2F(bottomLeft, Placement::right, d2dPatch.point31);

                bezierToPOINT_2F(bottomRight, Placement::top, d2dPatch.point23);
                bezierToPOINT_2F(bottomRight, Placement::left, d2dPatch.point32);

                interiorToPOINT_2F(topLeft, Placement::bottomRight, d2dPatch.point11);
                interiorToPOINT_2F(bottomLeft, Placement::topRight, d2dPatch.point12);
                interiorToPOINT_2F(bottomRight, Placement::topLeft, d2dPatch.point21);
                interiorToPOINT_2F(topRight, Placement::bottomLeft, d2dPatch.point22);

                d2dPatch.color00 = toCOLOR_F(topLeft->color);
                d2dPatch.color03 = toCOLOR_F(topRight->color);
                d2dPatch.color30 = toCOLOR_F(bottomLeft->color);
                d2dPatch.color33 = toCOLOR_F(bottomRight->color);

                d2dPatch.topEdgeMode = (row == 0) ? D2D1_PATCH_EDGE_MODE_ANTIALIASED : D2D1_PATCH_EDGE_MODE_ALIASED;
                d2dPatch.leftEdgeMode = (column == 0) ? D2D1_PATCH_EDGE_MODE_ANTIALIASED : D2D1_PATCH_EDGE_MODE_ALIASED;
                d2dPatch.bottomEdgeMode = (row >= numRows - 2) ? D2D1_PATCH_EDGE_MODE_ANTIALIASED : D2D1_PATCH_EDGE_MODE_ALIASED;
                d2dPatch.rightEdgeMode = (column >= numColumns - 2) ? D2D1_PATCH_EDGE_MODE_ANTIALIASED : D2D1_PATCH_EDGE_MODE_ALIASED;
            }
        }

        pimpl->createResources();

        auto& deviceContext = pimpl->resources->deviceContext;
        if (deviceContext && image.isValid())
        {
            pimpl->gradientMesh = {};
            deviceContext->CreateGradientMesh(d2dPatches.data(), (uint32_t)d2dPatches.size(), pimpl->gradientMesh.put());

            if (pimpl->gradientMesh)
            {
                if (auto pixelData = dynamic_cast<juce::Direct2DPixelData*>(image.getPixelData()))
                {
                    if (auto bitmap = pixelData->getFirstPageForContext(deviceContext))
                    {
                        deviceContext->SetTarget(bitmap);
                        deviceContext->BeginDraw();
                        deviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
                        deviceContext->Clear(juce::D2DUtilities::toCOLOR_F(backgroundColor));
                        deviceContext->DrawGradientMesh(pimpl->gradientMesh.get());
                        [[maybe_unused]] auto hr = deviceContext->EndDraw();
                        jassert(SUCCEEDED(hr));
                        deviceContext->SetTarget(nullptr);
                    }
                }
            }
        }
    }

    Color128 Color128::fromHSV(float hue, float saturation, float brightness, float alpha) noexcept
    {
        brightness = juce::jlimit(0.0f, 1.0f, brightness);

        if (saturation <= 0)
            return Color128{ brightness, brightness, brightness, alpha };

        saturation = juce::jmin(1.0f, saturation);
        hue = ((hue - std::floor(hue)) * 360.0f) / 60.0f;
        auto f = hue - std::floor(hue);
        auto x = brightness * (1.0f - saturation);

        if (hue < 1.0f)   return Color128{ brightness, brightness * (1.0f - (saturation * (1.0f - f))), x, alpha, };
        if (hue < 2.0f)   return Color128{ brightness * (1.0f - saturation * f), brightness, x, alpha, };
        if (hue < 3.0f)   return Color128{ x, brightness, brightness * (1.0f - (saturation * (1.0f - f))), alpha, };
        if (hue < 4.0f)   return Color128{ x, brightness * (1.0f - saturation * f), brightness, alpha };
        if (hue < 5.0f)   return Color128{ brightness * (1.0f - (saturation * (1.0f - f))), x, brightness, alpha, };
        return                 Color128{ brightness, x, brightness * (1.0f - saturation * f), alpha, };
    }

    Color128 Color128::grayLevel(float level) noexcept
    {
        return Color128{ level, level, level, 1.0f };
    }


#if 0
    void MeshGradient::makeConicGradient(juce::Rectangle<float> bounds)
    {
        /*
                      A---B B---C           B   B
        A---B---C     |   | |   |          /|   | \
        |   |   |     |   | |   |         / |   |  \
        |   |   |     D---E E---F        A--DE  EF--C
        D---E---F
        |   |   |     D---E E---F        G--DE  EF--I
        |   |   |     |   | |   |        \  |   | /
        G---H---I     |   | |   |         \ |   |/
                      G---H H---I           H   H


            01     01
          / |      |  \
        /   |      |   \
       00--10 11  11 12-02

       20--10 11  11 12-22
         \  |     |  /
          \ |     | /
            21     21

        00 -> 01 -> 11 -> 10
        01 -> 02 -> 12 -> 11
        22 -> 21 -> 11 -> 12
        21 -> 20 -> 10 -> 11

        */


}
#endif


    } // namespace mescal
