
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

    static void pathToGeometrySink(const Path& path, ID2D1GeometrySink* sink, const AffineTransform& transform, D2D1_FIGURE_BEGIN figureMode)
    {
        // Every call to BeginFigure must have a matching call to EndFigure. But - the Path does not necessarily
        // have matching startNewSubPath and closePath markers. The figureStarted flag indicates if an extra call
        // to BeginFigure or EndFigure is needed during the iteration loop or when exiting this function.
        Path::Iterator it(path);
        bool figureStarted = false;

        while (it.next())
        {
            switch (it.elementType)
            {
            case Path::Iterator::cubicTo:
            {
                if (!figureStarted)
                    break;

                transform.transformPoint(it.x1, it.y1);
                transform.transformPoint(it.x2, it.y2);
                transform.transformPoint(it.x3, it.y3);

                sink->AddBezier({ { it.x1, it.y1 }, { it.x2, it.y2 }, { it.x3, it.y3 } });
                break;
            }

            case Path::Iterator::lineTo:
            {
                if (!figureStarted)
                    break;

                transform.transformPoint(it.x1, it.y1);
                sink->AddLine({ it.x1, it.y1 });
                break;
            }

            case Path::Iterator::quadraticTo:
            {
                if (!figureStarted)
                    break;

                transform.transformPoint(it.x1, it.y1);
                transform.transformPoint(it.x2, it.y2);
                sink->AddQuadraticBezier({ { it.x1, it.y1 }, { it.x2, it.y2 } });
                break;
            }

            case Path::Iterator::closePath:
            {
                if (figureStarted)
                {
                    sink->EndFigure(D2D1_FIGURE_END_CLOSED);
                    figureStarted = false;
                }
                break;
            }

            case Path::Iterator::startNewSubPath:
            {
                if (figureStarted)
                    sink->EndFigure(D2D1_FIGURE_END_OPEN);

                transform.transformPoint(it.x1, it.y1);
                sink->BeginFigure({ it.x1, it.y1 }, figureMode);

                figureStarted = true;
                break;
            }
            }
        }

        if (figureStarted)
        {
            sink->EndFigure(D2D1_FIGURE_END_OPEN);
        }
    }

    static winrt::com_ptr<ID2D1Geometry> pathToGeometry(juce::Path const& path, juce::AffineTransform const& transform)
    {
        winrt::com_ptr<ID2D1PathGeometry1> geometry;
        juce::SharedResourcePointer<juce::DirectX> directX;

        if (auto hr = directX->getD2DFactory()->CreatePathGeometry(geometry.put()); FAILED(hr))
        {
            return {};
        }

        winrt::com_ptr<ID2D1GeometrySink> sink;
        if (auto hr = geometry->Open(sink.put()); FAILED(hr))
        {
            return {};
        }

        pathToGeometrySink(path, sink.get(), transform, D2D1_FIGURE_BEGIN_FILLED);

        sink->Close();

        return geometry;
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
#if 0
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

            auto halfedge = addHalfedge(tail, head, b0, b1, direction);

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
#endif
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
    for (auto it = halfedges.begin(); it != halfedges.end(); ++it)
    {
        if (it->lock().get() == halfedge.get())
        {
            halfedges.erase(it);
            break;
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

std::shared_ptr<GradientMesh::Patch> GradientMesh::addConnectedPatch(Patch* sourcePatch, EdgePlacement sourceConnectedEdge)
{
#if 0
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
                                patchHalfedges[(int)edgePlacement] = addHalfedge(tail, head, bcp0, bcp1, edgePlacement);
                        }
                        else
                        {
                            if (auto head = sourceHalfedge->head.lock())
                            {
                                auto vertex = addVertex(head->position + translation);
                                patchHalfedges[(int)edgePlacement] = addHalfedge(tail, vertex, bcp0, bcp1, edgePlacement);
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
#endif

    //return patch;
    return {};
}

std::shared_ptr<GradientMesh::Vertex> GradientMesh::addVertex(juce::Point<float> point)
{
    vertices.emplace_back(std::make_shared<Vertex>(point, *this, vertices.size()));
    return vertices.back();
}

#if 0
std::shared_ptr<GradientMesh::Halfedge> GradientMesh::addHalfedge(std::shared_ptr<Vertex> tail,
    std::shared_ptr<Vertex> head,
    std::shared_ptr<BezierControlPoint> b0,
    std::shared_ptr<BezierControlPoint> b1,
    Direction edgePlacement)
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

    jassert(!tail->getHalfedge(clockwiseFrom(edgePlacement)).lock());
    jassert(!head->getHalfedge(counterclockwiseFrom(edgePlacement)).lock());

    tail->halfedges[(int)clockwiseFrom(edgePlacement)] = halfedge;
    head->halfedges[(int)counterclockwiseFrom(edgePlacement)] = halfedge->twin.lock();

    halfedges.push_back(halfedge);
    halfedges.push_back(twin);

    return halfedge;
}
#endif

std::shared_ptr<GradientMesh::Halfedge> GradientMesh::addHalfedge(std::shared_ptr<Vertex> tail,
    std::shared_ptr<Vertex> head,
    std::shared_ptr<BezierControlPoint> b0,
    std::shared_ptr<BezierControlPoint> b1)
{
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

    halfedge->angle = tail->position.getAngleToPoint(head->position);
    twin->angle = halfedge->angle + juce::MathConstants<float>::pi;

    halfedges.push_back(halfedge);
    halfedges.push_back(twin);

    return halfedge;
}

std::shared_ptr<GradientMesh::BezierControlPoint> GradientMesh::addBezierControlPoint(juce::Point<float> position)
{
    bezierControlPoints.emplace_back(std::make_shared<BezierControlPoint>(position, *this));
    return bezierControlPoints.back();
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

    auto convertColor = [&](std::weak_ptr<Vertex> vertex)
        {
            if (auto v = vertex.lock())
            {
                return D2DUtilities::toCOLOR_F(v->color);
            }

            throw std::runtime_error("Invalid vertex");
        };

#if 0

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
                d2dEdgeMode = h->twin.lock()->patch.lock() ? D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ALIASED : D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ANTIALIASED;

                if (h->edgeType == EdgeType::straight)
                {
                    if (auto tail = h->tail.lock())
                    {
                        juce::Line<float> line{ tail->position.x, tail->position.y, head.x, head.y };
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
#endif

    std::vector<D2D1_GRADIENT_MESH_PATCH> d2dPatches;
    for (auto const& patch : patches)
    {
        d2dPatches.emplace_back(D2D1_GRADIENT_MESH_PATCH{});
        auto& d2dPatch = d2dPatches.back();
        std::array<D2D1_POINT_2F*, 4> d2dPatchCorners
        {
            &d2dPatch.point30, &d2dPatch.point00, &d2dPatch.point03, &d2dPatch.point33
        };
        std::array<D2D1_COLOR_F*, 4> d2dPatchColors
        {
            &d2dPatch.color30, &d2dPatch.color00, &d2dPatch.color03, &d2dPatch.color33
        };

        try
        {
            const auto& patchHalfedges = patch->getHalfedges();

            auto halfedge = patchHalfedges.front();
            auto halfedgeLock = halfedge.lock();
            if (!halfedgeLock)
                continue;

            auto normalizedAngle = std::abs(halfedgeLock->angle) / juce::MathConstants<float>::twoPi;
            auto cardinalDirection = (int)(normalizedAngle * 4.0f);
            cardinalDirection &= 3;
            if (halfedgeLock->angle < 0.0f)
                cardinalDirection = 4 - cardinalDirection;

            *(d2dPatchCorners[cardinalDirection]) = convertVertex(halfedgeLock->tail);
            *(d2dPatchColors[cardinalDirection]) = convertColor(halfedgeLock->tail);

            cardinalDirection = (cardinalDirection - 1) & 3;
            *(d2dPatchCorners[cardinalDirection]) = convertVertex(halfedgeLock->head);
            *(d2dPatchColors[cardinalDirection]) = convertColor(halfedgeLock->head);

            halfedge = patchHalfedges[2];
            halfedgeLock = halfedge.lock();
            if (!halfedgeLock)
                continue;

            cardinalDirection = (cardinalDirection - 1) & 3;
            *(d2dPatchCorners[cardinalDirection]) = convertVertex(halfedgeLock->tail);
            *(d2dPatchColors[cardinalDirection]) = convertColor(halfedgeLock->tail);

            cardinalDirection = (cardinalDirection - 1) & 3;
            *(d2dPatchCorners[cardinalDirection]) = convertVertex(halfedgeLock->head);
            *(d2dPatchColors[cardinalDirection]) = convertColor(halfedgeLock->head);

            d2dPatch.point01 = d2dPatch.point00;
            d2dPatch.point02 = d2dPatch.point03;
            d2dPatch.point13 = d2dPatch.point03;
            d2dPatch.point23 = d2dPatch.point33;
            d2dPatch.point32 = d2dPatch.point33;
            d2dPatch.point31 = d2dPatch.point30;
            d2dPatch.point20 = d2dPatch.point30;
            d2dPatch.point10 = d2dPatch.point00;

            //             const auto& colors = patch->getColors();
            //             d2dPatch.color00 = D2DUtilities::toCOLOR_F(colors[(int)CornerPlacement::topLeft]);
            //             d2dPatch.color03 = D2DUtilities::toCOLOR_F(colors[(int)CornerPlacement::topRight]);
            //             d2dPatch.color33 = D2DUtilities::toCOLOR_F(colors[(int)CornerPlacement::bottomRight]);
            //             d2dPatch.color30 = D2DUtilities::toCOLOR_F(colors[(int)CornerPlacement::bottomLeft]);

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

GradientMesh::Patch::Patch(const std::vector<std::shared_ptr<Halfedge>>& halfedges_)
{
    for (auto const& halfedge : halfedges_)
    {
        halfedges.push_back(halfedge);
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

void GradientMesh::Patch::createPath()
{
    path.clear();

    if (halfedges.size() == 0)
        return;

    {
        auto halfedge = halfedges.front().lock();
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
            default:
                if (head)
                    path.lineTo(head->position);
                break;

#if 0
            case EdgeType::approximateQuadratic:
            case EdgeType::cubic:
            {
                if (head && b0 && b1)
                    path.cubicTo(b0->position,
                        b1->position,
                        head->position);
                break;
            }
#endif
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

static void addGridPoints(juce::SortedSet<float>& set, float nominalSpacing)
{
    for (int index = set.size() - 1; index >= 0; --index)
    {
        float value = set[index];
        auto delta = set[index + 1] - value;
        if (delta > nominalSpacing * 2.0f)
        {
            auto count = (int)std::floor(delta / nominalSpacing);
            auto step = delta / count;
            value += step;
            while (count > 0)
            {
                set.add(value);
                value += step;
                --count;
            }
        }
    }
}

std::unique_ptr<GradientMesh> GradientMesh::pathToGrid(Path const& path, PathOptions const& options)
{
    auto mesh = std::make_unique<GradientMesh>();

    //
    // Convert perimeter points to Clipper2 path with metadata
    //
    Clipper2Lib::PathD subjectPath;
    static constexpr int64_t perimeterPointBit = 0x8000000000000000LL;
    static constexpr int64_t rowColumnEncodeBit = 0x4000000000000000LL;

    juce::PathFlatteningIterator it{ path, options.transform, options.tolerance };
    juce::SortedSet<float> xValues, yValues;
    while (it.next())
    {
        int64_t z = int64_t(subjectPath.size()) << 32;
        z |= perimeterPointBit;
        subjectPath.emplace_back(it.x1, it.y1, z);

        xValues.add(it.x1);
        yValues.add(it.y1);
    }

    if (options.nominalPatchWidth > 0.0f)
    {
        addGridPoints(xValues, options.nominalPatchWidth);
    }

    if (options.nominalPatchHeight > 0.0f)
    {
        addGridPoints(yValues, options.nominalPatchHeight);
    }

    //
    // Clip the clipper2 path to grid cells
    //
    Clipper2Lib::PathsD subjectPaths{ subjectPath };
    Clipper2Lib::PathsD gridPaths;

    //
    // Iterate through the grid and clip the grid cells to the path
    //
    for (auto itx = xValues.begin(); itx != xValues.end() - 1; ++itx)
    {
        for (auto ity = yValues.begin(); ity != yValues.end() - 1; ++ity)
        {
            int64_t gridColumn = itx - xValues.begin();
            int64_t gridRow = ity - yValues.begin();

            auto encode = [&](int64_t column, int64_t row)
                {
                    return rowColumnEncodeBit | (column << 16) | row;
                };

            auto decodeColumn = [](int64_t z)
                {
                    return (z >> 16) & 0xFFFF;
                };

            auto decodeRow = [](int64_t z)
                {
                    return z & 0xFFFF;
                };

            auto decodePerimeterIndex = [](int64_t z)
                {
                    return (z >> 32) & 0xffffff;
                };

            gridPaths.clear();
            gridPaths.emplace_back(Clipper2Lib::PathD
                {
                    { *(itx + 1), *(ity + 1), encode(gridColumn + 1, gridRow + 1) },
                    { *itx, *(ity + 1), encode(gridColumn, gridRow + 1)},
                    { *itx, *ity, encode(gridColumn, gridRow)},
                    { *(itx + 1), *ity, encode(gridColumn + 1, gridRow)}
                });
            auto intersectionPaths = Clipper2Lib::Intersect(subjectPaths, gridPaths, Clipper2Lib::FillRule::Positive);

            for (auto const& intersectionPath : intersectionPaths)
            {
                std::vector<std::shared_ptr<Vertex>> patchVertices;

                for (auto const& intersectionPoint : intersectionPath)
                {
                    auto const& vertices = mesh->getVertices();

                    auto findIterator = std::find_if(vertices.begin(), vertices.end(), [&](std::shared_ptr<Vertex> const& vertex)
                        {
                            return approximatelyEqual(vertex->position.x, (float)intersectionPoint.x) &&
                                approximatelyEqual(vertex->position.y, (float)intersectionPoint.y);
                        });

                    if (findIterator != vertices.end())
                    {
                        patchVertices.push_back(*findIterator);
                        continue;
                    }

                    auto vertex = mesh->addVertex({ (float)intersectionPoint.x, (float)intersectionPoint.y });
                    patchVertices.push_back(vertex);
                }

                if (patchVertices.size())
                {
                    std::vector<std::shared_ptr<Halfedge>> patchHalfedges;
                    auto lastVertex = patchVertices.back();
                    for (auto const& vertex : patchVertices)
                    {
                        auto halfedge = mesh->addHalfedge(lastVertex, vertex);

                        lastVertex->halfedges.push_back(halfedge);
                        vertex->halfedges.push_back(halfedge->twin);

                        patchHalfedges.push_back(halfedge);
                        lastVertex = vertex;
                    }

                    auto patch = std::make_shared<Patch>(patchHalfedges);
                    patch->update();
                    mesh->addPatch(patch);
                }
            }
        }

        mesh->check();
    }

    //
    // Assign colors
    //
    auto pathBounds = path.getBounds();
    float xScale = 1.0f / pathBounds.getWidth();
    float yScale = 1.0f / pathBounds.getHeight();
    auto center = pathBounds.getCentre();
    auto radius = juce::jmax(pathBounds.getWidth(), pathBounds.getHeight()) * 0.5f;
    auto radiusInverse = 1.0f / radius;
    juce::Colour startColour = juce::Colours::blue;
    juce::Colour endColour = juce::Colours::white;
    for (auto& vertex : mesh->vertices)
    {
        if (options.findVertexColor)
        {
            vertex->color = options.findVertexColor(vertex);
        }
        else
        {
            auto angle = center.getAngleToPoint(vertex->position);
            auto distance = center.getDistanceFrom(vertex->position);
            vertex->color = juce::Colour{ 0.75f + 0.25f * std::sin(angle), juce::jlimit(0.0f, 1.0f, distance / radius), 1.0f, 1.0f };
        }
    }

    return mesh;
}

std::unique_ptr<GradientMesh> GradientMesh::pathToGridAlt(juce::Path const& path, PathOptions const& options)
{
    auto mesh = std::make_unique<GradientMesh>();

    struct PerimeterEdge
    {
        juce::Point<float> tail, head;
        std::optional<juce::Point<float>> b0, b1;
        juce::Path::Iterator::PathElementType type = juce::Path::Iterator::lineTo;
    };

    struct Perimeter
    {
        Perimeter(juce::Path const& path, PathOptions const& options)
        {
            juce::Path::Iterator it{ path };
            juce::Point<float> lastPoint, subpathStart;

            auto nextPerimeterEdge = [&]() -> std::optional<PerimeterEdge>
                {
                    DBG("it " << it.elementType << " " << it.x1 << " " << it.y1 << " " << it.x2 << " " << it.y2 << " " << it.x3 << " " << it.y3);
                    switch (it.elementType)
                    {
                    case Path::Iterator::startNewSubPath:
                    {
                        subpathStart = { it.x1, it.y1 };
                        lastPoint = { it.x1, it.y1 };
                        return {};
                    }

                    case Path::Iterator::lineTo:
                    {
                        PerimeterEdge edge;
                        edge.tail = lastPoint;
                        edge.head = { it.x1, it.y1 };
                        edge.type = it.elementType;

                        lastPoint = edge.head;

                        return edge;
                    }

                    case Path::Iterator::quadraticTo:
                    {
                        PerimeterEdge edge;
                        edge.tail = lastPoint;
                        edge.b0 = { it.x1, it.y1 };
                        edge.head = { it.x2, it.y2 };
                        edge.type = it.elementType;

                        lastPoint = edge.head;

                        return edge;
                    }

                    case Path::Iterator::cubicTo:
                    {
                        PerimeterEdge edge;
                        edge.tail = lastPoint;
                        edge.b0 = { it.x1, it.y1 };
                        edge.b1 = { it.x2, it.y2 };
                        edge.head = { it.x3, it.y3 };
                        edge.type = it.elementType;

                        lastPoint = edge.head;

                        return edge;
                    }

                    case Path::Iterator::closePath:
                    {
                        PerimeterEdge edge;
                        edge.tail = lastPoint;
                        edge.head = subpathStart;
                        edge.type = juce::Path::Iterator::closePath;

                        lastPoint = edge.head;

                        return edge;
                    }

                    default:
                    {
                        break;
                    }
                    }

                    return {};
                };

            while (it.next())
            {
                auto edge = nextPerimeterEdge();
                if (!edge)
                    continue;

                float xDelta = std::abs(edge->head.x - edge->tail.x);
                float yDelta = std::abs(edge->head.y - edge->tail.y);

                switch (edge->type)
                {
                default:
                case Path::Iterator::lineTo:
                {
#if 0
                    juce::Line<float> line{ edge->tail, edge->head };

                    int numInterpolatedPoints = (int)std::floor(line.getLength() / juce::jmin(options.nominalPatchWidth, options.nominalPatchHeight)) - 1;

                    lastPoint = edge->tail;

                    for (int i = 0; i < numInterpolatedPoints; ++i)
                    {
                        auto point = line.getPointAlongLineProportionally((i + 1) / (float)(numInterpolatedPoints + 1));
                        edges.push_back(PerimeterEdge{ lastPoint, point });
                        lastPoint = point;
                    }
#endif

                    break;
                }

                case Path::Iterator::cubicTo:
                {
                    bezier::Bezier<3> edgeCurve
                    {
                        {
                            { edge->tail.x, edge->tail.y },
                            { edge->b0.value().x, edge->b0.value().y },
                            { edge->b1.value().x, edge->b1.value().y },
                            { edge->head.x, edge->head.y }
                        }
                    };

                    auto toPoint = [&](bezier::Point& bp)
                        {
                            return juce::Point<float>{ (float)bp.x, (float)bp.y };
                        };

                    juce::Line<float> line{ edge->tail, edge->head };

#if 0
                    {
                        float xDelta = edge->head.x - edge->tail.x;
                        int numInterpolatedPoints = (int)std::floor(std::abs(xDelta) / options.nominalPatchHeight - 1);
                        float xStep = xDelta / (numInterpolatedPoints + 1);
                        float x = edge->tail.x;// +xStep;

                        DBG("X " << x << " num:" << numInterpolatedPoints);

                        for (int i = 0; i < numInterpolatedPoints; ++i)
                        {
                            x += xStep;

                            float splitPosition = 0.5f;
                            float fraction = 0.25f;
                            auto bezierPoint = edgeCurve.valueAt(splitPosition);
                            while (std::abs(bezierPoint.x - x) > options.nominalPatchWidth * 0.5f && fraction > 0.03125f)
                            {
                                if (bezierPoint.x > x)
                                {
                                    splitPosition -= fraction;
                                }
                                else
                                {
                                    splitPosition += fraction;
                                }
                                fraction *= 0.5F;

                                bezierPoint = edgeCurve.valueAt(splitPosition);
                            }

                            auto splitSegment = edgeCurve.split(splitPosition);

                            PerimeterEdge splitEdge;
                            splitEdge.tail = toPoint(splitSegment.left[0]);
                            auto splitPoint = splitSegment.left.valueAt(1.0);
                            splitEdge.head = toPoint(splitPoint);
                            auto edgeB0 = splitSegment.left[1];
                            auto edgeB1 = splitSegment.left[2];
                            splitEdge.b0.emplace(toPoint(edgeB0));
                            splitEdge.b1.emplace(toPoint(edgeB1));
                            edges.push_back(splitEdge);

                            edgeCurve = splitSegment.right;
                            lastPoint = splitEdge.head;
                        }
                    }
#endif

#if 1
                    {
                        float yDelta = edge->head.y - edge->tail.y;
                        //int numInterpolatedPoints = (int)std::floor(std::abs(xDelta) / options.nominalPatchHeight);
                        int numInterpolatedPoints = 1;
                        float yStep = yDelta / (float)(numInterpolatedPoints + 1);
                        float y = edge->tail.y;

                        for (int i = 0; i < numInterpolatedPoints; ++i)
                        {
                            y += yStep;

                            float splitPosition = 0.5f;
                            float fraction = 0.25f;
                            auto bezierPoint = edgeCurve.valueAt(splitPosition);
                            while (std::abs(bezierPoint.y - y) > options.nominalPatchHeight * 0.5f && fraction > 0.03125f)
                            {
                                if (bezierPoint.y > y)
                                {
                                    splitPosition -= fraction;
                                }
                                else
                                {
                                    splitPosition += fraction;
                                }
                                fraction *= 0.5F;

                                bezierPoint = edgeCurve.valueAt(splitPosition);
                            }

                            auto splitSegment = edgeCurve.split(splitPosition);

                            PerimeterEdge splitEdge;
                            splitEdge.tail = toPoint(splitSegment.left[0]);
                            splitEdge.head = toPoint(splitSegment.left[3]);
                            auto edgeB0 = splitSegment.left[1];
                            auto edgeB1 = splitSegment.left[2];
                            splitEdge.b0.emplace(toPoint(edgeB0));
                            splitEdge.b1.emplace(toPoint(edgeB1));
                            splitEdge.type = Path::Iterator::cubicTo;
                            edges.push_back(splitEdge);

                            edgeCurve = splitSegment.right;
                            lastPoint = splitEdge.head;
                        }
                    }
#endif

                    break;
                }
                }

                edges.push_back(*edge);
            }
        }

        std::vector<PerimeterEdge> edges;
        juce::SortedSet<float> xValues, yValues;
        Clipper2Lib::PathD subjectPath;
    } perimeter{ path, options };

    if (perimeter.edges.size() > 0)
    {
        auto lastVertex = mesh->addVertex(perimeter.edges.front().tail);
        for (auto const& edge : perimeter.edges)
        {
            auto vertex = mesh->addVertex(edge.head);

            std::shared_ptr<BezierControlPoint> b0, b1;
            if (edge.b0)
            {
               b0 = mesh->addBezierControlPoint(edge.b0.value());
            }
            if (edge.b1)
            {
                b1 = mesh->addBezierControlPoint(edge.b1.value());
            }
            mesh->addHalfedge(lastVertex, vertex, b0, b1);

            lastVertex = vertex;
        }
    }

    return mesh;
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
