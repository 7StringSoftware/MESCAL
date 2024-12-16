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

        void createResources()
        {
            resources->create();
        }

        MeshGradient& owner;
        juce::SharedResourcePointer<DirectXResources> resources;
        winrt::com_ptr<ID2D1GradientMesh> gradientMesh;
    };


    MeshGradient::MeshGradient(int numRows_, int numColumns_, std::optional<juce::Rectangle<float>> bounds) :
        numRows(numRows_),
        numColumns(numColumns_),
        pimpl(std::make_unique<Pimpl>(*this))
    {
        jassert(numRows_ > 0);
        jassert(numColumns_ > 0);

        float rowHeight = 1.0f;
        float columnWidth = 1.0f;

        for (int row = 0; row < numRows_; ++row)
        {
            for (int column = 0; column < numColumns_; ++column)
            {
                auto& patch = patches.emplace_back(std::make_shared<Patch>(*this, row, column));
                patch->edgeModes[(size_t)EdgePlacement::top] = row == 0 ? Edge::antialiased : Edge::aliased;
                patch->edgeModes[(size_t)EdgePlacement::bottom] = row == numRows_ - 1 ? Edge::antialiased : Edge::aliased;
                patch->edgeModes[(size_t)EdgePlacement::left] = column == 0 ? Edge::antialiased : Edge::aliased;
                patch->edgeModes[(size_t)EdgePlacement::right] = column == numColumns_ - 1 ? Edge::antialiased : Edge::aliased;
            }
        }

        int numVertices = (numRows_ + 1) * (numColumns_ + 1);
        for (int i = 0; i < numVertices; ++i)
        {
            sharedVertices.emplace_back(std::make_shared<SharedVertex>());
        }

        {
            auto vertexIterator = sharedVertices.begin();
            for (int vertexRow = 0; vertexRow < numRows_ + 1; vertexRow++)
            {
                for (int vertexColumn = 0; vertexColumn < numColumns_ + 1; vertexColumn++)
                {
                    auto& vertex = *vertexIterator++;

                    auto addPatch = [&](int rowOffset, int columnOffset, CornerPlacement cornerPlacement)
                        {
                            int patchRow = vertexRow + rowOffset;
                            int patchColumn = vertexColumn + columnOffset;
                            if (auto patch = getPatch(patchRow, patchColumn))
                            {
                                vertex->patches.push_back(patch);
                                patch->sharedVertices[(size_t)cornerPlacement] = vertex;
                            }
                        };

                    addPatch(0, 0, CornerPlacement::topLeft);
                    addPatch(0, -1, CornerPlacement::topRight);
                    addPatch(-1, -1, CornerPlacement::bottomRight);
                    addPatch(-1, 0, CornerPlacement::bottomLeft);
                }
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
        for (auto& patch : patches)
        {
            for (auto& point : patch->points)
            {
                if (point.has_value())
                {
                    point = point->transformedBy(transform);
                }
            }
        }
    }

    MeshGradient::Patch::Patch(MeshGradient& owner_, int row_, int column_) :
        row(row_),
        column(column_),
        owner(owner_)
    {
    }

    void MeshGradient::Patch::setBounds(juce::Rectangle<float> rect)
    {
        setCornerPosition(mescal::MeshGradient::CornerPlacement::topLeft, rect.getTopLeft());
        setCornerPosition(mescal::MeshGradient::CornerPlacement::topRight, rect.getTopRight());
        setCornerPosition(mescal::MeshGradient::CornerPlacement::bottomLeft, rect.getBottomLeft());
        setCornerPosition(mescal::MeshGradient::CornerPlacement::bottomRight, rect.getBottomRight());

        for (auto edgePlacement : edgePlacements)
        {
            auto edge = getEdge(edgePlacement);

            juce::Line<float> line{ edge.tail, edge.head };
            edge.internalControlPoints =
            {
                line.getPointAlongLineProportionally(0.33f),
                line.getPointAlongLineProportionally(0.66f)
            };

            setEdge(edgePlacement, edge);
        }

        {
            juce::Line diagonal{ rect.getTopLeft(), rect.getBottomRight() };
            setInteriorControlPointPosition(mescal::MeshGradient::CornerPlacement::topLeft, diagonal.getPointAlongLineProportionally(0.25f));
            setInteriorControlPointPosition(mescal::MeshGradient::CornerPlacement::bottomRight, diagonal.getPointAlongLineProportionally(0.75f));
        }

        {
            juce::Line diagonal{ rect.getTopRight(), rect.getBottomLeft() };
            setInteriorControlPointPosition(mescal::MeshGradient::CornerPlacement::topRight, diagonal.getPointAlongLineProportionally(0.25f));
            setInteriorControlPointPosition(mescal::MeshGradient::CornerPlacement::bottomLeft, diagonal.getPointAlongLineProportionally(0.75f));
        }
    }

    std::optional<juce::Point<float>> MeshGradient::Patch::getPosition(int matrixRow, int matrixColumn) const noexcept
    {
        return points[matrixRow * numMatrixColumns + matrixColumn];
    }

    juce::Point<float> MeshGradient::Patch::getCornerPosition(CornerPlacement placement) const noexcept
    {
        auto index = cornerIndices[(size_t)placement];
        jassert(points[index].has_value());
        return points[index].value_or(juce::Point<float>{});
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

    void MeshGradient::Patch::setCornerPositions(juce::Span<juce::Point<float>> positions)
    {
        {
            auto it = positions.begin();
            for (auto index : cornerIndices)
            {
                points[index] = *it;
                it++;
            }
        }
    }

    Color128 MeshGradient::Patch::getColor(CornerPlacement placement) const noexcept
    {
        return colors[(size_t)placement];
    }

    void MeshGradient::Patch::setColor(CornerPlacement placement, juce::Colour color)
    {
        colors[(size_t)placement] = color;
    }

    void MeshGradient::Patch::setColor(CornerPlacement placement, Color128 color)
    {
        colors[(size_t)placement] = color;
    }

    void MeshGradient::Patch::setEdge(EdgePlacement edgePlacement, Edge edge)
    {
        auto [tailCornerPlacement, headCornerPlacement] = edgeToCornerPlacements(edgePlacement);
        setCornerPosition(tailCornerPlacement, edge.tail);
        setCornerPosition(headCornerPlacement, edge.head);

        auto [bezierIndex0, bezierIndex1] = edgeBezierControlPointIndices[(size_t)edgePlacement];
        points[bezierIndex0] = edge.bezierControlPoints.first;
        points[bezierIndex1] = edge.bezierControlPoints.second;
    }

    MeshGradient::Edge MeshGradient::Patch::getEdge(EdgePlacement edgePlacement) const noexcept
    {
        Edge edge;

        auto [tailCornerPlacement, headCornerPlacement] = edgeToCornerPlacements(edgePlacement);
        edge.tail = getCornerPosition(tailCornerPlacement);
        edge.head = getCornerPosition(headCornerPlacement);

        auto [bezierIndex0, bezierIndex1] = edgeBezierControlPointIndices[(size_t)edgePlacement];
        edge.bezierControlPoints = std::make_pair(points[bezierIndex0], points[bezierIndex1]);

        return edge;
    }

    std::optional<juce::Point<float>> MeshGradient::Patch::getBezierControlPointPosition(EdgePlacement edgePlacement, BezierControlPointPlacement bezierControlPointPlacement)
    {
        auto indexPair = MeshGradient::edgeBezierControlPointIndices[(size_t)edgePlacement];
        return points[ bezierControlPointPlacement == MeshGradient::BezierControlPointPlacement::first ? indexPair.first : indexPair.second];
    }

    void MeshGradient::Patch::setBezierControlPointPosition(EdgePlacement edgePlacement, BezierControlPointPlacement bezierControlPointPlacement, juce::Point<float> position)
    {
        auto indexPair = MeshGradient::edgeBezierControlPointIndices[(size_t)edgePlacement];
        points[bezierControlPointPlacement == MeshGradient::BezierControlPointPlacement::first ? indexPair.first : indexPair.second] = position;
    }

    std::optional<juce::Point<float>> MeshGradient::Patch::getInteriorControlPointPosition(CornerPlacement placement)
    {
        auto index = interiorControlIndices[(size_t)placement];
        return points[index];
    }

    void MeshGradient::Patch::setInteriorControlPointPosition(CornerPlacement placement, juce::Point<float> position)
    {
        auto index = interiorControlIndices[(size_t)placement];
        points[index] = position;
    }

    std::pair<MeshGradient::CornerPlacement, MeshGradient::CornerPlacement> MeshGradient::Patch::edgeToCornerPlacements(EdgePlacement edgePlacement)
    {
        auto tailCornerPlacement = (CornerPlacement)edgePlacement;
        auto nextCornerPlacement = (CornerPlacement)(((size_t)(edgePlacement)+1) & (edgePlacements.size() - 1));
        return { tailCornerPlacement, nextCornerPlacement };
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
        for (auto const& patch : patches)
        {
            auto& d2dPatch = *d2dPatchIterator++;
            std::array<D2D1_POINT_2F*, 16> d2dPoints
            {
                &d2dPatch.point00, &d2dPatch.point01, &d2dPatch.point02, &d2dPatch.point03,
                &d2dPatch.point10, &d2dPatch.point11, &d2dPatch.point12, &d2dPatch.point13,
                &d2dPatch.point20, &d2dPatch.point21, &d2dPatch.point22, &d2dPatch.point23,
                &d2dPatch.point30, &d2dPatch.point31, &d2dPatch.point32, &d2dPatch.point33
            };

            std::array<D2D1_POINT_2F*, 16> d2dFallbackPoints
            {
                &d2dPatch.point00, &d2dPatch.point00, &d2dPatch.point03, &d2dPatch.point03,
                &d2dPatch.point00, &d2dPatch.point00, &d2dPatch.point03, &d2dPatch.point03,
                &d2dPatch.point30, &d2dPatch.point30, &d2dPatch.point33, &d2dPatch.point33,
                &d2dPatch.point30, &d2dPatch.point30, &d2dPatch.point33, &d2dPatch.point33
            };

            auto destinationIterator = d2dPoints.begin();
            for (auto point : patch->points)
            {
                if (point.has_value())
                {
                    **destinationIterator = pointToPOINT_2F(*point);
                }

                ++destinationIterator;
            }

            destinationIterator = d2dPoints.begin();
            auto fallbackIterator = d2dFallbackPoints.begin();
            for (auto point : patch->points)
            {
                if (!point.has_value())
                {
                    **destinationIterator = **fallbackIterator;
                }

                ++destinationIterator;
                ++fallbackIterator;
            }

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

            d2dPatch.bottomEdgeMode = (D2D1_PATCH_EDGE_MODE)patch->edgeModes[(size_t)EdgePlacement::bottom];
            d2dPatch.topEdgeMode = (D2D1_PATCH_EDGE_MODE)patch->edgeModes[(size_t)EdgePlacement::top];
            d2dPatch.leftEdgeMode = (D2D1_PATCH_EDGE_MODE)patch->edgeModes[(size_t)EdgePlacement::left];
            d2dPatch.rightEdgeMode= (D2D1_PATCH_EDGE_MODE)patch->edgeModes[(size_t)EdgePlacement::right];
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
                    //if (auto bitmap = pixelData->getFirstPageForDevice(pimpl->resources->adapter->direct2DDevice))
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


    juce::Rectangle<float> MeshGradient::getBounds() const noexcept
    {
        juce::Rectangle<float> bounds;
        for (auto const& patch : patches)
        {
            bounds = bounds
                .getUnion({ patch->getCornerPosition(CornerPlacement::topLeft), patch->getCornerPosition(CornerPlacement::bottomRight) })
                .getUnion({ patch->getCornerPosition(CornerPlacement::topRight), patch->getCornerPosition(CornerPlacement::bottomLeft) });
        }

        return bounds;
    }

} // namespace mescal
