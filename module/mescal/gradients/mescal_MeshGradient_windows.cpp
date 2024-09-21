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
        /   |                    / | \
        P10 |                  P02 |  P13
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

#include "mescal_MeshGradient_windows.h"

#endif

    struct MeshGradient::Pimpl
    {
        Pimpl(MeshGradient& owner_) : owner(owner_)
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

        MeshGradient& owner;
        winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
        winrt::com_ptr<ID2D1GradientMesh> gradientMesh;
    };


    MeshGradient::MeshGradient(int numRows_, int numColumns_, std::optional<juce::Rectangle<float>> bounds) :
        pimpl(std::make_unique<Pimpl>(*this)),
        numRows(numRows_),
        numColumns(numColumns_)
    {
        jassert(numRows_ > 0);
        jassert(numColumns_ > 0);

        float rowHeight = 1.0f;
        float columnWidth = 1.0f;

        for (int row = 0; row < numRows_; ++row)
        {
            for (int column = 0; column < numColumns_; ++column)
            {
                patches.emplace_back(std::make_shared<Patch>(*this, row, column));
            }
        }

        float startX = 0.0f, startY = 0.0f;

        if (bounds.has_value())
        {
            startX = bounds->getX();
            startY = bounds->getY();
            rowHeight = bounds->getHeight() / numRows_;
            columnWidth = bounds->getWidth() / numColumns_;

            float y = startY;
            for (int row = 0; row < numRows_; ++row)
            {
                float x = startX;
                for (int column = 0; column < numColumns_; ++column)
                {
                    getPatch(row, column)->setBounds({ x, y, columnWidth, rowHeight });

                    x += columnWidth;
                }

                y += rowHeight;
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

    MeshGradient::Patch::Patch(MeshGradient& owner_, int row_, int column_) :
        owner(owner_),
        row(row_),
        column(column_)
    {
    }

    void MeshGradient::Patch::setBounds(juce::Rectangle<float> rect)
    {
        setCornerPosition(mescal::MeshGradient::CornerPlacement::topLeft, rect.getTopLeft());
        setCornerPosition(mescal::MeshGradient::CornerPlacement::topRight, rect.getTopRight());
        setCornerPosition(mescal::MeshGradient::CornerPlacement::bottomLeft, rect.getBottomLeft());
        setCornerPosition(mescal::MeshGradient::CornerPlacement::bottomRight, rect.getBottomRight());
    }

    juce::Point<float> MeshGradient::Patch::getPosition(int matrixRow, int matrixColumn) const noexcept
    {
        return points[matrixRow * numMatrixColumns + matrixColumn];
    }

    juce::Point<float> MeshGradient::Patch::getCornerPosition(CornerPlacement placement) const noexcept
    {
        auto index = cornerIndices[(size_t)placement];
        return points[index];
    }

    void MeshGradient::Patch::setPosition(int matrixRow, int matrixColumn, juce::Point<float> position)
    {
        points[matrixRow * numMatrixColumns + matrixColumn] = position;
    }

    void MeshGradient::Patch::setCornerPosition(CornerPlacement placement, juce::Point<float> position)
    {
        auto index = cornerIndices[(size_t)placement];
        points[index] = position;
    }

    Color128 MeshGradient::Patch::getColor(CornerPlacement placement) const noexcept
    {
        return colors[(size_t)placement];
    }

    void MeshGradient::Patch::setColor(CornerPlacement placement, juce::Colour color)
    {
        colors[(size_t)placement] = color;
    }

    void MeshGradient::Patch::setEdge(EdgePlacement placement, Edge edge)
    {
        
    }

    MeshGradient::Edge MeshGradient::Patch::getEdge(EdgePlacement placement) const noexcept
    {

    }

    void MeshGradient::draw(juce::Image image, juce::AffineTransform transform, juce::Colour backgroundColor)
    {
        auto pointToPOINT_2F = [&](juce::Point<float> p)
            {
                p = p.transformedBy(transform);
                return D2D1_POINT_2F{ p.x, p.y };
            };

        auto toCOLOR_F = [](Color128 color)
            {
                return D2D1::ColorF(color.red, color.green, color.blue, color.alpha);
            };

        std::vector<D2D1_GRADIENT_MESH_PATCH> d2dPatches{ patches.size(), D2D1_GRADIENT_MESH_PATCH{} };
        auto d2dPatchIterator = d2dPatches.begin();
        for (auto const patch : patches)
        {
            auto& d2dPatch = *d2dPatchIterator++;
            std::array<D2D1_POINT_2F*, 16> d2dPoints
            {
                &d2dPatch.point00, &d2dPatch.point01, &d2dPatch.point02, &d2dPatch.point03,
                &d2dPatch.point10, &d2dPatch.point11, &d2dPatch.point12, &d2dPatch.point13,
                &d2dPatch.point20, &d2dPatch.point21, &d2dPatch.point22, &d2dPatch.point23,
                &d2dPatch.point30, &d2dPatch.point31, &d2dPatch.point32, &d2dPatch.point33
            };

            auto pointIterator = d2dPoints.begin();
            for (auto point : patch->points)
            {
                **pointIterator++ = pointToPOINT_2F(point);
            }

            d2dPatch.point01 = d2dPatch.point00;
            d2dPatch.point02 = d2dPatch.point03;
            d2dPatch.point10 = d2dPatch.point00;
            d2dPatch.point20 = d2dPatch.point30;
            d2dPatch.point13 = d2dPatch.point03;
            d2dPatch.point23 = d2dPatch.point33;
            d2dPatch.point31 = d2dPatch.point30;
            d2dPatch.point32 = d2dPatch.point33;

            d2dPatch.point11 = d2dPatch.point00;
            d2dPatch.point12 = d2dPatch.point03;
            d2dPatch.point21 = d2dPatch.point30;
            d2dPatch.point22 = d2dPatch.point33;

            std::array < D2D1_COLOR_F*, 4> d2dColors
            {
                &d2dPatch.color00, &d2dPatch.color30, &d2dPatch.color33, &d2dPatch.color03
            };

            auto colorIterator = d2dColors.begin();
            for (auto color : patch->colors)
            {
                **colorIterator = toCOLOR_F(color);
                colorIterator++;
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
                        pimpl->deviceContext->Clear(juce::D2DUtilities::toCOLOR_F(backgroundColor));
                        pimpl->deviceContext->DrawGradientMesh(pimpl->gradientMesh.get());
                        [[maybe_unused]] auto hr = pimpl->deviceContext->EndDraw();
                        jassert(SUCCEEDED(hr));
                        pimpl->deviceContext->SetTarget(nullptr);
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
