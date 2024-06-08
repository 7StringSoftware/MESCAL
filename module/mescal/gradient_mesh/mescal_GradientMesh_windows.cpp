namespace mescal
{

#ifdef __INTELLISENSE__

#include "mescal_GradientMesh_windows.h"
#include "../json/mescal_JSON.h"

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

                auto halfedge = addHalfedge(tail, head, b0, b1, direction, EdgeType::cubic);

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
                        //
                        // Make new control points & halfedges
                        //
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
                                    patchHalfedges[(int)edgePlacement] = addHalfedge(tail, head, bcp0, bcp1, edgePlacement, sourceHalfedge->edgeType);
                            }
                            else
                            {
                                if (auto head = sourceHalfedge->head.lock())
                                {
                                    auto vertex = addVertex(head->position + translation);
                                    patchHalfedges[(int)edgePlacement] = addHalfedge(tail, vertex, bcp0, bcp1, edgePlacement, sourceHalfedge->edgeType);
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
        vertices.emplace_back(std::make_shared<Vertex>(point, *this));
        return vertices.back();
    }

    std::shared_ptr<GradientMesh::Halfedge> GradientMesh::addHalfedge(std::shared_ptr<Vertex> tail,
        std::shared_ptr<Vertex> head,
        std::shared_ptr<BezierControlPoint> b0,
        std::shared_ptr<BezierControlPoint> b1,
        Direction edgePlacement,
        EdgeType type)
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

        halfedge->edgeType = type;
        twin->edgeType = type;

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
                    //d2dEdgeMode = h->twin.lock()->patch.lock() ? D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ALIASED : D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ANTIALIASED;
                    d2dEdgeMode = D2D1_PATCH_EDGE_MODE_ALIASED_INFLATED;

                    if (h->edgeType == EdgeType::straight)
                    {
                        if (auto tail = h->tail.lock())
                        {
                            auto tailPosition = tail->position.transformedBy(transform);
                            juce::Line<float> line{ tailPosition.x, tailPosition.y, head.x, head.y };
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

    GradientMesh::Patch::Patch(std::array<std::shared_ptr<Halfedge>, 4>& halfedges_, juce::Uuid uuid_) :
        uuid(uuid_)
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

    mescal::JSONObject GradientMesh::Patch::toJSON() const noexcept
    {
        juce::SharedResourcePointer<GradientMesh::Identifiers> const ids;

        JSONArray halfedgesArray;
        for (auto const& halfedge : halfedges)
        {
            if (auto halfedgeLock = halfedge.lock())
                halfedgesArray.add<JSONObject>(halfedgeLock->toJSON());
            else
                halfedgesArray.add<JSONObject>({});
        }

        JSONArray colorsArray;
        for (auto const& color : cornerColors)
        {
            colorsArray.add<juce::String>(color.toString());
        }

        JSONObject patchObject;
        patchObject.set<JSONArray>(ids->halfedges, halfedgesArray);
        patchObject.set<JSONArray>(ids->colors, colorsArray);
        patchObject.set<juce::String>(ids->uniqueID, uuid.toDashedString());
        return patchObject;
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

    mescal::JSONObject GradientMesh::toJSON() const noexcept
    {
        juce::SharedResourcePointer<GradientMesh::Identifiers> const ids;

        JSONArray verticesArray;
        for (auto const& vertex : vertices)
        {
            verticesArray.add<JSONObject>(vertex->toJSON());
        }

        JSONArray bezierControlPointsArray;
        for (auto const& bezier : bezierControlPoints)
        {
            bezierControlPointsArray.add<JSONObject>(bezier->toJSON());
        }

        JSONArray halfedgesArray;
        for (auto const& halfedge : halfedges)
        {
            halfedgesArray.add<JSONObject>(halfedge->toJSON());
        }

        JSONArray patchesArray;
        for (auto const& patch : patches)
        {
            patchesArray.add<JSONObject>(patch->toJSON());
        }

        JSONObject root;
        root.setArray(ids->vertices, verticesArray);
        root.setArray(ids->bezierControlPoints, bezierControlPointsArray);
        root.setArray(ids->halfedges, halfedgesArray);
        root.setArray(ids->patches, patchesArray);

        return root;
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

    JSONObject GradientMesh::Vertex::toJSON() const noexcept
    {
        juce::SharedResourcePointer<GradientMesh::Identifiers> const ids;

        JSONObject vertexObject;

        vertexObject.set<juce::String>(ids->uniqueID, uuid.toDashedString());
        vertexObject.set<float>(ids->x, position.x);
        vertexObject.set<float>(ids->y, position.y);

        JSONArray halfedgesArray;
        for (auto const& halfedgeWeakPtr : halfedges)
        {
            if (auto halfedge = halfedgeWeakPtr.lock())
            {
                halfedgesArray.add<String>(halfedge->uuid.toDashedString());
            }
        }

        vertexObject.set<JSONArray>(ids->halfedges, halfedgesArray);

        return vertexObject;
    }

    mescal::JSONObject GradientMesh::BezierControlPoint::toJSON() const noexcept
    {
        juce::SharedResourcePointer<GradientMesh::Identifiers> const ids;
        JSONObject jsonObject;

        jsonObject.set<juce::String>(ids->uniqueID, uuid.toDashedString());
        jsonObject.set<float>(ids->x, position.x);
        jsonObject.set<float>(ids->y, position.y);

        return jsonObject;
    }

    JSONObject GradientMesh::Halfedge::toJSON() const noexcept
    {
        juce::SharedResourcePointer<GradientMesh::Identifiers> const ids;

        JSONObject halfedgeObject;

        if (auto tailLock = tail.lock())
            halfedgeObject.set<String>(ids->tail, tailLock->uuid.toDashedString());
        if (auto headLock = head.lock())
            halfedgeObject.set<String>(ids->head, headLock->uuid.toDashedString());
        if (auto b0Lock = b0.lock())
            halfedgeObject.set<String>(ids->bezierControlPoint0, b0Lock->uuid.toDashedString());
        if (auto b1Lock = b1.lock())
            halfedgeObject.set<String>(ids->bezierControlPoint1, b1Lock->uuid.toDashedString());
        if (auto twinLock = this->twin.lock())
            halfedgeObject.set<String>(ids->twin, twinLock->uuid.toDashedString());

        halfedgeObject.set<String>(ids->uniqueID, uuid.toDashedString());
        halfedgeObject.set<int>(ids->edgeType, (int)edgeType);

        return halfedgeObject;
    }

    void GradientMesh::loadFromJSON(JSONObject const& jsonObject)
    {
        juce::SharedResourcePointer<GradientMesh::Identifiers> const ids;

        //
        // Vertices
        //
        {
            auto const verticesArray = jsonObject.get<JSONArray>(ids->vertices);
            for (auto const& vertexObject : verticesArray.getArray<JSONObject>())
            {
                auto uuid = juce::Uuid{ vertexObject.get<juce::String>(ids->uniqueID) };
                juce::Point<float> position;
                position.x = vertexObject.get<float>(ids->x);
                position.y = vertexObject.get<float>(ids->y);

                auto vertex = std::make_shared<Vertex>(position, uuid, *this);
                vertices.push_back(vertex);
            }
        }

        //
        // Bezier control points
        //
        {
            auto const bezierControlPointsArray = jsonObject.get<JSONArray>(ids->bezierControlPoints);
            for (auto const& bezierObject : bezierControlPointsArray.getArray<JSONObject>())
            {
                auto uuid = juce::Uuid{ bezierObject.get<juce::String>(ids->uniqueID) };
                juce::Point<float> position;
                position.x = bezierObject.get<float>(ids->x);
                position.y = bezierObject.get<float>(ids->y);

                auto bezier = std::make_shared<BezierControlPoint>(position, uuid, *this);
                bezierControlPoints.push_back(bezier);
            }
        }

        //
        // Halfedges
        //
        using TwinUuid = std::pair<std::shared_ptr<Halfedge>, juce::Uuid>;
        std::vector<TwinUuid> twinUuids;
        {
            auto findVertex = [&](JSONObject const& json, juce::Identifier const& id) -> std::shared_ptr<Vertex>
                {
                    auto uuid = juce::Uuid{ json.get<juce::String>(id) };
                    for (auto const& vertex : vertices)
                    {
                        if (vertex->uuid == uuid)
                            return vertex;
                    }

                    return nullptr;
                };

            auto findBezier = [&](JSONObject const& json, juce::Identifier const& id) -> std::shared_ptr<BezierControlPoint>
                {
                    auto uuid = juce::Uuid{ json.get<juce::String>(id) };
                    for (auto const& bezier : bezierControlPoints)
                    {
                        if (bezier->uuid == uuid)
                            return bezier;
                    }

                    return nullptr;
                };

            auto const halfedgesArray = jsonObject.get<JSONArray>(ids->halfedges);
            for (auto const& halfedgeObject : halfedgesArray.getArray<JSONObject>())
            {
                juce::Uuid uuid{ halfedgeObject.get<juce::String>(ids->uniqueID) };
                auto edgeType = static_cast<EdgeType>(halfedgeObject.get<int>(ids->edgeType));

                auto tail = findVertex(halfedgeObject, ids->tail);
                auto head = findVertex(halfedgeObject, ids->head);
                auto b0 = findBezier(halfedgeObject, ids->bezierControlPoint0);
                auto b1 = findBezier(halfedgeObject, ids->bezierControlPoint1);

                if (tail && head && b0 && b1)
                {
                    auto halfedge = std::make_shared<Halfedge>(uuid);
                    halfedge->tail = tail;
                    halfedge->head = head;
                    halfedge->b0 = b0;
                    halfedge->b1 = b1;
                    halfedge->edgeType = edgeType;

                    halfedges.push_back(halfedge);

                    twinUuids.emplace_back(TwinUuid{ halfedge, juce::Uuid{ halfedgeObject.get<juce::String>(ids->twin) } });
                }
            }
        }

        //
        // Twins
        //
        for (auto const& twinUuid : twinUuids)
        {
            for (auto const& h : halfedges)
            {
                if (h->uuid == twinUuid.second)
                {
                    twinUuid.first->twin = h;
                    h->twin = twinUuid.first;
                }
            }
        }

        //
        // Vertex halfedges
        //
        {
            auto const verticesArray = jsonObject.get<JSONArray>(ids->vertices);
            for (auto const& vertexObject : verticesArray.getArray<JSONObject>())
            {
                auto const& vertexJSONHalfedges = vertexObject.get<JSONArray>(ids->halfedges);

                for (int direction = 0; direction < juce::jmin(4, vertexJSONHalfedges.size()); ++direction)
                {
                    auto halfedgeUuidString = vertexJSONHalfedges.get<juce::String>(direction);
                    if (halfedgeUuidString.isEmpty())
                        continue;

                    auto const halfedgeUuid = juce::Uuid{ halfedgeUuidString };
                    for (auto const& h : halfedges)
                    {
                        if (h->uuid == halfedgeUuid)
                        {
                            if (auto vertex = h->tail.lock())
                            {
                                vertex->halfedges[direction] = h;
                                break;
                            }
                        }
                    }
                }
            }
        }

        //
        // Patches
        //
        {
            auto const patchesArray = jsonObject.get<JSONArray>(ids->patches);
            for (auto const& patch : patchesArray.getArray<JSONObject>())
            {
                auto const halfedgesArray = patch.get<JSONArray>(ids->halfedges);
                std::array<std::shared_ptr<Halfedge>, 4> patchHalfedges;
                juce::Uuid patchUuid{ patch.get<juce::String>(ids->uniqueID) };

                for (size_t index = 0; index < 4; ++index)
                {
                    auto uuid = juce::Uuid{ halfedgesArray.getArray<JSONObject>()[index].get<juce::String>(ids->uniqueID) };
                    for (auto const& halfedge : halfedges)
                    {
                        if (halfedge->uuid == uuid)
                        {
                            patchHalfedges[index] = halfedge;
                        }
                    }
                }

                auto newPatch = std::make_shared<Patch>(patchHalfedges, patchUuid);

                auto colorsArray = patch.get<JSONArray>(ids->colors);
                int colorIndex = 0;
                for (auto const& color : colorsArray)
                {
                    newPatch->setColor((CornerPlacement)colorIndex, juce::Colour::fromString(color.toString()));
                    ++colorIndex;
                }

                for (auto& halfedge : patchHalfedges)
                {
                    halfedge->patch = newPatch;
                }

                newPatch->update();

                patches.push_back(newPatch);
            }
        }
    }

} // namespace mescal
