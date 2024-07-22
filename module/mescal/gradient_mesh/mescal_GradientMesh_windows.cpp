namespace mescal
{

#ifdef __INTELLISENSE__

#include "mescal_GradientMesh_windows.h"

#endif

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


    GradientMesh::GradientMesh(int numRows_, int numColumns_) :
        pimpl(std::make_unique<Pimpl>(*this)),
        numRows(numRows_),
        numColumns(numColumns_)
    {
        jassert(numRows_ > 1);
        jassert(numColumns_ > 1);

        float rowHeight = 1.0f;
        float columnWidth = 1.0f;
        auto numVertices = numRows_ * numColumns_;
        vertices.reserve(numVertices);

        for (int row = 0; row < numRows_; ++row)
        {
            for (int column = 0; column < numColumns_; ++column)
            {
                auto x = column * columnWidth;
                auto y = row * rowHeight;
                vertices.push_back(std::make_shared<Vertex>(row, column, juce::Point<float>{x, y}));
            }
        }

        for (int row = 0; row < numRows_ - 1; row += 2)
        {
            //
            // Move right to left across this row and add halfedges
            //
            for (int column = numColumns_ - 1; column >= 1; --column)
            {
                auto tail = vertices[row * numColumns_ + column];
                auto head = vertices[row * numColumns_ + column - 1];
                auto halfedge = addHalfedge(tail, head);
                tail->eastHalfedge = halfedge;
                head->westHalfedge = halfedge->twin;
            }

            //
            // Left to right across next row; add halfedges
            //
            for (int column = 0; column < numColumns_ - 1; ++column)
            {
                auto tail = vertices[(row + 1) * numColumns_ + column];
                auto head = vertices[(row + 1) * numColumns_ + column + 1];
                auto halfedge = addHalfedge(tail, head);
                tail->westHalfedge = halfedge;
                head->eastHalfedge = halfedge->twin;
            }
        }

        for (int column = 0; column < numColumns_ - 1; column += 2)
        {
            //
            // Add halfedges top to bottom
            //
            for (int row = 0; row < numRows_ - 1; ++row)
            {
                auto tail = vertices[row * numColumns_ + column];
                auto head = vertices[(row + 1) * numColumns_ + column];
                auto halfedge = addHalfedge(tail, head);
                tail->southHalfedge = halfedge;
                head->northHalfedge = halfedge->twin;
            }

            //
            // Add halfedges bottom to top for the next column
            //
            for (int row = numRows_ - 1; row >= 1; --row)
            {
                auto tail = vertices[row * numColumns_ + column + 1];
                auto head = vertices[(row - 1) * numColumns_ + column + 1];
                auto halfedge = addHalfedge(tail, head);
                tail->northHalfedge = halfedge;
                head->southHalfedge = halfedge->twin;
            }
        }
    }

    GradientMesh::~GradientMesh()
    {
    }

    std::shared_ptr<GradientMesh::Halfedge> GradientMesh::addHalfedge(std::shared_ptr<Vertex> tail, std::shared_ptr<Vertex> head)
    {
        auto halfedge = std::make_shared<Halfedge>();
        halfedge->tail = tail;
        halfedge->head = head;

        auto twin = std::make_shared<Halfedge>();
        twin->tail = head;
        twin->head = tail;

        halfedge->twin = twin;
        twin->twin = halfedge;

        halfedges.push_back(halfedge);
        halfedges.push_back(twin);

        return halfedge;
    }

    void GradientMesh::applyTransform([[maybe_unused]] juce::AffineTransform const& transform)
    {
        jassertfalse;
    }

    void GradientMesh::configureVertices(std::function<void(std::shared_ptr<Vertex> vertex)> callback)
    {
        jassert(callback);

        for (auto& vertex : vertices)
        {
            callback(vertex);
        }
    }

    std::shared_ptr<GradientMesh::Vertex> GradientMesh::getVertex(int row, int column)
    {
        return vertices[row * numColumns + column];
    }

    std::shared_ptr<GradientMesh::Halfedge> GradientMesh::getHalfedge(std::shared_ptr<Vertex> tail, std::shared_ptr<Vertex> head)
    {
        if (!tail || !head)
        {
            return {};
        }

        std::array<std::weak_ptr<Halfedge>, 4> tailHalfedges
        {
            tail->northHalfedge,
            tail->westHalfedge,
            tail->southHalfedge,
            tail->eastHalfedge
        };

        for (auto& tailHalfedge : tailHalfedges)
        {
            if (auto halfedgeLock = tailHalfedge.lock())
            {
                if (halfedgeLock->head.lock() == head)
                    return tailHalfedge.lock();
            }
        }

        return {};
    }

    void GradientMesh::draw(juce::Image image, juce::AffineTransform transform, juce::Colour backgroundColor)
    {
        auto vertexToPOINT_2F = [&](int row, int column)
            {
                auto vertex = getVertex(row, column);
                auto transformedPoint = vertex->position.transformedBy(transform);
                return D2D1_POINT_2F{ transformedPoint.x, transformedPoint.y };
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

                d2dPatch.point00 = vertexToPOINT_2F(row, column);
                d2dPatch.point03 = vertexToPOINT_2F(row, column + 1);
                d2dPatch.point30 = vertexToPOINT_2F(row + 1, column);
                d2dPatch.point33 = vertexToPOINT_2F(row + 1, column + 1);

                d2dPatch.point01 = d2dPatch.point00;
                d2dPatch.point02 = d2dPatch.point03;

                d2dPatch.point10 = d2dPatch.point00;
                d2dPatch.point20 = d2dPatch.point30;

                d2dPatch.point31 = d2dPatch.point30;
                d2dPatch.point32 = d2dPatch.point33;

                d2dPatch.point13 = d2dPatch.point03;
                d2dPatch.point23 = d2dPatch.point33;

                d2dPatch.point11 = d2dPatch.point00;
                d2dPatch.point12 = d2dPatch.point03;
                d2dPatch.point21 = d2dPatch.point30;
                d2dPatch.point22 = d2dPatch.point33;

                auto northwestVertex = getVertex(row, column);
                auto southwestVertex = getVertex(row + 1, column);
                auto northeastVertex = getVertex(row, column + 1);
                auto southeastVertex = getVertex(row + 1, column + 1);

                if (auto eastHalfedge = getHalfedge(northeastVertex, southeastVertex))
                {
                    if (eastHalfedge->bezierControlPoints.has_value())
                    {
                        d2dPatch.point13 = pointToPOINT_2F(eastHalfedge->bezierControlPoints->first);
                        d2dPatch.point23 = pointToPOINT_2F(eastHalfedge->bezierControlPoints->second);
                    }

                    if (eastHalfedge->antialiasing)
                        d2dPatch.rightEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
                }

                if (auto westHalfedge = getHalfedge(northwestVertex, southwestVertex))
                {
                    if (westHalfedge->bezierControlPoints.has_value())
                    {
                        d2dPatch.point10 = pointToPOINT_2F(westHalfedge->bezierControlPoints->first);
                        d2dPatch.point20 = pointToPOINT_2F(westHalfedge->bezierControlPoints->second);
                    }

                    if (westHalfedge->antialiasing)
                        d2dPatch.leftEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
                }

                d2dPatch.color00 = toCOLOR_F(northwestVertex->southeastColor);
                d2dPatch.color03 = toCOLOR_F(northeastVertex->southwestColor);
                d2dPatch.color30 = toCOLOR_F(southwestVertex->northeastColor);
                d2dPatch.color33 = toCOLOR_F(southeastVertex->northwestColor);
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
                        pimpl->deviceContext->EndDraw();
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
    void GradientMesh::makeConicGradient(juce::Rectangle<float> bounds)
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
