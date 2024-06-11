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
                vertices.push_back(std::make_shared<Vertex>(juce::Point<float>{x, y}));
            }
        }

        for (int row = 0; row < numRows_ - 1; row += 2)
        {
            for (int column = numColumns_ - 1; column >= 1; --column)
            {
                auto tail = vertices[row * numColumns_ + column];
                auto head = vertices[row  * numColumns_ + column - 1];
                auto halfedge = addHalfedge(tail, head);
                tail->halfedge = halfedge;
            }

            for (int column = 0; column < numColumns_ - 1; ++column)
            {
                auto tail = vertices[(row + 1) * numColumns_ + column];
                auto head = vertices[(row + 1) * numColumns_ + column + 1];
                auto halfedge = addHalfedge(tail, head);
                tail->halfedge = halfedge;
            }
        }

        for (int column = 0; column < numColumns_ - 1; column += 2)
        {
            for (int row = 0; row < numRows_ - 1; ++row)
            {
                auto tail = vertices[row * numColumns_ + column];
                auto head = vertices[(row + 1) * numColumns_ + column];
                auto halfedge = addHalfedge(tail, head);
                tail->halfedge = halfedge;
            }

            for (int row = numRows_ - 1; row >= 1; --row)
            {
                auto tail = vertices[row * numColumns_ + column + 1];
                auto head = vertices[(row - 1) * numColumns_ + column + 1];
                auto halfedge = addHalfedge(tail, head);
                tail->halfedge = halfedge;
            }
        }
    }

#if 0
    GradientMesh GradientMesh::fromPath(const juce::Path& path)
    {
        GradientMesh mesh;
        std::unordered_map<juce::Point<float>, std::shared_ptr<Vertex>> vertexMap;
        std::shared_ptr<Vertex> subpathStart;
        std::shared_ptr<Vertex> previousVertex;

        auto addMappedVertex = [&](juce::Point<float> point) -> std::shared_ptr<Vertex>
            {
                auto newVertex = std::make_shared<Vertex>(point);
                auto storedVertex = vertexMap[point];
                if (newVertex == storedVertex)
                {
                    return storedVertex;
                }

                mesh.vertices.push_back(newVertex);
                vertexMap[point] = newVertex;
                return newVertex;
            };

        juce::Path::Iterator it{ path };
        while (it.next())
        {
            switch (it.elementType)
            {
            case Path::Iterator::startNewSubPath:
            {
                subpathStart = addMappedVertex({ it.x1, it.y1 });
                previousVertex = subpathStart;
                break;
            }

            case Path::Iterator::lineTo:
            {
                auto vertex = addMappedVertex({ it.x1, it.y1 });
                auto halfedge = mesh.addHalfedge(previousVertex, vertex);

                previousVertex = vertex;
                break;
            }

            case Path::Iterator::quadraticTo:
            {
                //
                // GradientMesh edges need to be straight lines or cubic splines
                //
                jassertfalse;
                break;
            }

            case Path::Iterator::cubicTo:
            {
                auto vertex = addMappedVertex({ it.x3, it.y3 });
                previousVertex = vertex;
                break;
            }

            case Path::Iterator::closePath:
            {
                if (previousVertex.get() != subpathStart.get())
                {
                }

                subpathStart = {};
                break;
            }
            }
        }

        return mesh;
    }
#endif

    GradientMesh::~GradientMesh()
    {
    }

    std::shared_ptr<GradientMesh::Vertex> GradientMesh::addVertex(juce::Point<float> point)
    {
        auto vertex = std::make_shared<Vertex>(point);
        vertices.push_back(vertex);
        return vertex;
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

    void GradientMesh::applyTransform(juce::AffineTransform const& transform)
    {
        jassertfalse;
    }

    void GradientMesh::configureVertices(std::function<void(int row, int column, std::shared_ptr<Vertex> vertex)> callback)
    {
        jassert(callback);

        int row = 0;
        int column = 0;
        for (auto& vertex : vertices)
        {
            callback(row, column, vertex);
            ++column;
            if (column >= numColumns)
            {
                column = 0;
                ++row;
            }
        }
    }

    void GradientMesh::draw(juce::Image image, juce::AffineTransform transform)
    {
        auto toPOINT_2F = [&](int row, int column)
            {
                auto& vertex = vertices[row * numColumns + column];
                auto transformedPoint = vertex->position.transformedBy(transform);
                return D2D1_POINT_2F{ transformedPoint.x, transformedPoint.y };
            };

        std::vector<D2D1_GRADIENT_MESH_PATCH> d2dPatches;
        d2dPatches.reserve(numRows * numColumns);
        for (int row = 0; row < numRows - 1; ++row)
        {
            for (int column = 0; column < numColumns - 1; ++column)
            {
                d2dPatches.emplace_back(D2D1_GRADIENT_MESH_PATCH{});
                auto& d2dPatch = d2dPatches.back();

                d2dPatch.point00 = toPOINT_2F(row, column);
                d2dPatch.point03 = toPOINT_2F(row, column + 1);
                d2dPatch.point30 = toPOINT_2F(row + 1, column);
                d2dPatch.point33 = toPOINT_2F(row + 1, column + 1);

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

                d2dPatch.color00 = juce::D2DUtilities::toCOLOR_F(vertices[row * numColumns + column]->color);
                d2dPatch.color03 = juce::D2DUtilities::toCOLOR_F(vertices[row * numColumns + column + 1]->color);
                d2dPatch.color30 = juce::D2DUtilities::toCOLOR_F(vertices[(row + 1) * numColumns + column]->color);
                d2dPatch.color33 = juce::D2DUtilities::toCOLOR_F(vertices[(row + 1) * numColumns + column + 1]->color);
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

    Color128 Color128::fromHSV(float hue, float saturation, float brightness, float alpha) noexcept
    {
        brightness = jlimit(0.0f, 1.0f, brightness);

        if (saturation <= 0)
            return { brightness, brightness, brightness, alpha };

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

} // namespace mescal

