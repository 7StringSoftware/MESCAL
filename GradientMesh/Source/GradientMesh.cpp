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

const std::array<juce::Colour, 4> GradientMesh::Patch::defaultColors =
{
    juce::Colours::red,
    juce::Colours::yellow,
    juce::Colours::blue,
    juce::Colours::violet
};

#if JUCE_DEBUG
juce::String GradientMesh::Vertex::dump() const
{
    juce::String text = name + "\n";
    text << "   Position: " << position.toString() << "\n";
    text << "   Grid index: " << gridIndex.toString() << "\n";
    text << "   Edges: ";
    for (auto const& edge : edges)
    {
        if (edge != nullptr)
        {
            text << edge->name << " ";
        }
    }

    return text;
}

juce::String GradientMesh::Edge::dump() const
{
    juce::String text = name + "\n";
    text << "   Orientation: " << ((orientation == GradientMesh::Orientation::horizontal) ? "horizontal" : "vertical") << "\n";
    text << "   Control points: " << edgeControlPoints[0].toString() << ", " << edgeControlPoints[1].toString() << "\n";
    for (auto const& vertex : vertices)
    {
        if (vertex != nullptr)
        {
            text << vertex->name << " ";
        }
    }

    return text;
}

void GradientMesh::Face::setInnerControlPoints()
{
    innerControlPoints[0] = vertices[0]->position;
    innerControlPoints[1] = vertices[1]->position;
    innerControlPoints[2] = vertices[2]->position;
    innerControlPoints[3] = vertices[3]->position;

#if 0
    juce::Line<float> line{ vertices[(size_t)Corner::topLeft]->position, vertices[(size_t)Corner::bottomRight]->position };
    innerControlPoints[(size_t)Corner::topLeft] = line.getPointAlongLineProportionally(0.25f);
    innerControlPoints[(size_t)Corner::bottomRight] = line.getPointAlongLineProportionally(0.75f);

    line = { vertices[(size_t)Corner::topRight]->position, vertices[(size_t)Corner::bottomLeft]->position };
    innerControlPoints[(size_t)Corner::topRight] = line.getPointAlongLineProportionally(0.25f);
    innerControlPoints[(size_t)Corner::bottomLeft] = line.getPointAlongLineProportionally(0.75f);
#endif
}

juce::String GradientMesh::Face::dump() const
{
    juce::String text = name + "\n";
    text << "---Vertices\n";
    for (auto const& vertex : vertices)
    {
        if (vertex != nullptr)
        {
            text << vertex->dump() << "\n";
        }
    }
    text << "---Edges:\n";
    for (auto const& edge : edges)
    {
        if (edge != nullptr)
        {
            text << edge->dump() << "\n";
        }
    }

    return text;
}

#endif


struct GradientMesh::Patch::PatchPimpl
{
    PatchPimpl(Patch& owner_) :
        owner(owner_)
    {
#if 0
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
#endif
    }

    ~PatchPimpl()
    {
    }

    void toD2DPatch(D2D1_GRADIENT_MESH_PATCH* d2dPatch)
    {
        auto toPoint2F = [&](juce::Point<float> p)
            {
                return D2D1::Point2F(p.x, p.y);
            };

        d2dPatch->point00 = toPoint2F(owner.face->vertices[(size_t)Corner::topLeft]->position);
        d2dPatch->point03 = toPoint2F(owner.face->vertices[(size_t)Corner::topRight]->position);
        d2dPatch->point30 = toPoint2F(owner.face->vertices[(size_t)Corner::bottomLeft]->position);
        d2dPatch->point33 = toPoint2F(owner.face->vertices[(size_t)Corner::bottomRight]->position);

        auto topEdge = owner.face->edges[(size_t)Direction::north];
        d2dPatch->point01 = toPoint2F(topEdge->edgeControlPoints[0]);
        d2dPatch->point02 = toPoint2F(topEdge->edgeControlPoints[1]);

        auto rightEdge = owner.face->edges[(size_t)Direction::east];
        d2dPatch->point13 = toPoint2F(rightEdge->edgeControlPoints[0]);
        d2dPatch->point23 = toPoint2F(rightEdge->edgeControlPoints[1]);

        auto bottomEdge = owner.face->edges[(size_t)Direction::south];
        d2dPatch->point32 = toPoint2F(bottomEdge->edgeControlPoints[0]);
        d2dPatch->point31 = toPoint2F(bottomEdge->edgeControlPoints[1]);

        auto leftEdge = owner.face->edges[(size_t)Direction::west];
        d2dPatch->point10 = toPoint2F(leftEdge->edgeControlPoints[0]);
        d2dPatch->point20 = toPoint2F(leftEdge->edgeControlPoints[1]);

        d2dPatch->point11 = toPoint2F(owner.face->innerControlPoints[(size_t)Corner::topLeft]);
        d2dPatch->point12 = toPoint2F(owner.face->innerControlPoints[(size_t)Corner::topRight]);
        d2dPatch->point22 = toPoint2F(owner.face->innerControlPoints[(size_t)Corner::bottomRight]);
        d2dPatch->point21 = toPoint2F(owner.face->innerControlPoints[(size_t)Corner::bottomLeft]);

        d2dPatch->color00 = juce::D2DUtilities::toCOLOR_F(owner.face->colors[(size_t)Corner::topLeft]);
        d2dPatch->color03 = juce::D2DUtilities::toCOLOR_F(owner.face->colors[(size_t)Corner::topRight]);
        d2dPatch->color30 = juce::D2DUtilities::toCOLOR_F(owner.face->colors[(size_t)Corner::bottomLeft]);
        d2dPatch->color33 = juce::D2DUtilities::toCOLOR_F(owner.face->colors[(size_t)Corner::bottomRight]);

        d2dPatch->leftEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
        d2dPatch->topEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
        d2dPatch->rightEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
        d2dPatch->bottomEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
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

    auto addVertex(int x, int y, juce::Point<float> position)
    {
        auto vertex = vertices.emplace_back(std::make_unique<Vertex>(x, y, position)).get();
#if JUCE_DEBUG
        vertex->name = "Vertex " + juce::String(vertices.size());
#endif
        return vertex;
    }

    auto addEdge(Orientation orientation, Vertex* v1, Vertex* v2)
    {
        juce::Line<float> line{ v1->position, v2->position };
        auto cp1 = line.getPointAlongLineProportionally(0.25f);
        auto cp2 = line.getPointAlongLineProportionally(0.75f);
        auto edge = edges.emplace_back(std::make_unique<Edge>(orientation,
            cp1, cp2,
            v1, v2)).get();
#if JUCE_DEBUG
        edge->name = "Edge " + juce::String(edges.size());
#endif
        return edge;
    }

    auto addFace(Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3,
        Edge* e0, Edge* e1, Edge* e2, Edge* e3)
    {
        auto face = faces.emplace_back(std::make_unique<Face>()).get();
        face->vertices = { v0, v1, v2, v3 };
        face->edges = { e0, e1, e2, e3 };
#if JUCE_DEBUG
        face->name = "Face" + juce::String(faces.size());
#endif
        return face;
    }

    GradientMesh& owner;
    winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
    winrt::com_ptr<ID2D1GradientMesh> gradientMesh;

    std::vector<std::unique_ptr<Vertex>> vertices; // top left, top right, bottom right, bottom left
    std::vector<std::unique_ptr<Edge>> edges; // top, right, bottom, left
    std::vector<std::unique_ptr<Face>> faces;
};

GradientMesh::GradientMesh(juce::Rectangle<float> initialPatchArea) :
    pimpl(std::make_unique<Pimpl>(*this))
{
    auto topLeft = pimpl->addVertex(0, 0, initialPatchArea.getTopLeft());
    auto topRight = pimpl->addVertex(1, 0, initialPatchArea.getTopRight());
    auto bottomRight = pimpl->addVertex(1, 1, initialPatchArea.getBottomRight());
    auto bottomLeft = pimpl->addVertex(0, 1, initialPatchArea.getBottomLeft());

    auto topEdge = pimpl->addEdge(Orientation::horizontal, topLeft, topRight);
    auto rightEdge = pimpl->addEdge(Orientation::vertical, topRight, bottomRight);
    auto bottomEdge = pimpl->addEdge(Orientation::horizontal, bottomRight, bottomLeft);
    auto leftEdge = pimpl->addEdge(Orientation::vertical, bottomLeft, topLeft);

    topLeft->edges[(size_t)Direction::east] = topEdge;
    topLeft->edges[(size_t)Direction::south] = leftEdge;
    topRight->edges[(size_t)Direction::west] = topEdge;
    topRight->edges[(size_t)Direction::south] = rightEdge;
    bottomRight->edges[(size_t)Direction::north] = rightEdge;
    bottomRight->edges[(size_t)Direction::west] = bottomEdge;
    bottomLeft->edges[(size_t)Direction::north] = leftEdge;
    bottomLeft->edges[(size_t)Direction::east] = bottomEdge;

    auto face = pimpl->addFace(topLeft, topRight, bottomRight, bottomLeft,
        topEdge, rightEdge, bottomEdge, leftEdge);
    face->colors = Patch::defaultColors;
    face->setInnerControlPoints();

#if JUCE_DEBUG
    face->name = "Face 0";
#endif

    patches.add(new Patch{ face });
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
    if (pimpl->deviceContext && !pimpl->gradientMesh)
    {
        std::vector<D2D1_GRADIENT_MESH_PATCH> d2dPatches;
        d2dPatches.reserve(patches.size());

        auto d2dPatch = d2dPatches.data();
        for (auto const& patch : patches)
        {
            patch->pimpl->toD2DPatch(d2dPatch);
            d2dPatch++;
        }

#if 1
        DBG("");
        auto const firstPatch = d2dPatches.data();
        d2dPatch = d2dPatches.data();
        for (auto i = 0; i < patches.size(); ++i)
        {
            DBG("Patch " << i);
#if 1
            DBG("   Point00: " << d2dPatch->point00.x - firstPatch->point00.x << ", " << d2dPatch->point00.y - firstPatch->point00.y);
            DBG("   Point01: " << d2dPatch->point01.x - firstPatch->point01.x << ", " << d2dPatch->point01.y - firstPatch->point01.y);
            DBG("   Point02: " << d2dPatch->point02.x - firstPatch->point02.x << ", " << d2dPatch->point02.y - firstPatch->point02.y);
            DBG("   Point03: " << d2dPatch->point03.x - firstPatch->point03.x << ", " << d2dPatch->point03.y - firstPatch->point03.y);
            DBG("   Point10: " << d2dPatch->point10.x - firstPatch->point10.x << ", " << d2dPatch->point10.y - firstPatch->point10.y);
            DBG("   Point11: " << d2dPatch->point11.x - firstPatch->point11.x << ", " << d2dPatch->point11.y - firstPatch->point11.y);
            DBG("   Point12: " << d2dPatch->point12.x - firstPatch->point12.x << ", " << d2dPatch->point12.y - firstPatch->point12.y);
            DBG("   Point13: " << d2dPatch->point13.x - firstPatch->point13.x << ", " << d2dPatch->point13.y - firstPatch->point13.y);
            DBG("   Point20: " << d2dPatch->point20.x - firstPatch->point20.x << ", " << d2dPatch->point20.y - firstPatch->point20.y);
            DBG("   Point21: " << d2dPatch->point21.x - firstPatch->point21.x << ", " << d2dPatch->point21.y - firstPatch->point21.y);
            DBG("   Point22: " << d2dPatch->point22.x - firstPatch->point22.x << ", " << d2dPatch->point22.y - firstPatch->point22.y);
            DBG("   Point23: " << d2dPatch->point23.x - firstPatch->point23.x << ", " << d2dPatch->point23.y - firstPatch->point23.y);
            DBG("   Point30: " << d2dPatch->point30.x - firstPatch->point30.x << ", " << d2dPatch->point30.y - firstPatch->point30.y);
            DBG("   Point31: " << d2dPatch->point31.x - firstPatch->point31.x << ", " << d2dPatch->point31.y - firstPatch->point31.y);
            DBG("   Point32: " << d2dPatch->point32.x - firstPatch->point32.x << ", " << d2dPatch->point32.y - firstPatch->point32.y);
            DBG("   Point33: " << d2dPatch->point33.x - firstPatch->point33.x << ", " << d2dPatch->point33.y - firstPatch->point33.y);
#endif

#if 0
            DBG("   Point00: " << d2dPatch->point00.x << ", " << d2dPatch->point00.y);
            DBG("   Point01: " << d2dPatch->point01.x << ", " << d2dPatch->point01.y);
            DBG("   Point02: " << d2dPatch->point02.x << ", " << d2dPatch->point02.y);
            DBG("   Point03: " << d2dPatch->point03.x << ", " << d2dPatch->point03.y);
            DBG("   Point10: " << d2dPatch->point10.x << ", " << d2dPatch->point10.y);
            DBG("   Point11: " << d2dPatch->point11.x << ", " << d2dPatch->point11.y);
            DBG("   Point12: " << d2dPatch->point12.x << ", " << d2dPatch->point12.y);
            //DBG("   Point13: " << d2dPatch->point13.x << ", " << d2dPatch->point13.y);
            //DBG("   Point20: " << d2dPatch->point20.x << ", " << d2dPatch->point20.y);
            DBG("   Point21: " << d2dPatch->point21.x << ", " << d2dPatch->point21.y);
            DBG("   Point22: " << d2dPatch->point22.x << ", " << d2dPatch->point22.y);
            DBG("   Point23: " << d2dPatch->point23.x << ", " << d2dPatch->point23.y);
            DBG("   Point30: " << d2dPatch->point30.x << ", " << d2dPatch->point30.y);
            DBG("   Point31: " << d2dPatch->point31.x << ", " << d2dPatch->point31.y);
            DBG("   Point32: " << d2dPatch->point32.x << ", " << d2dPatch->point32.y);
            DBG("   Point33: " << d2dPatch->point33.x << ", " << d2dPatch->point33.y);
            DBG("   Color00: " << d2dPatch->color00.r << ", " << d2dPatch->color00.g << ", " << d2dPatch->color00.b << ", " << d2dPatch->color00.a);
            DBG("   Color03: " << d2dPatch->color03.r << ", " << d2dPatch->color03.g << ", " << d2dPatch->color03.b << ", " << d2dPatch->color03.a);
            DBG("   Color30: " << d2dPatch->color30.r << ", " << d2dPatch->color30.g << ", " << d2dPatch->color30.b << ", " << d2dPatch->color30.a);
            DBG("   Color33: " << d2dPatch->color33.r << ", " << d2dPatch->color33.g << ", " << d2dPatch->color33.b << ", " << d2dPatch->color33.a);
#endif

            d2dPatch++;
        }
#endif

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

#if 1
    juce::Graphics g{ image };

    auto drawDot = [&](juce::Point<float> pos, juce::Colour c, juce::StringRef text)
        {
            juce::Rectangle<float> r{ 200.0f, 30.0f };
            r.setCentre(pos);
            g.setColour(c);
            g.fillRect(r);
            g.setColour(juce::Colours::black);
            g.drawText(pos.toString(), r, juce::Justification::centred);
        };

    for (auto& vertex : pimpl->vertices)
    {
        drawDot(vertex->position, juce::Colours::white, vertex->name);
    }

    g.setColour(juce::Colours::lightgrey);
    for (auto& edge : pimpl->edges)
    {
        drawDot(edge->edgeControlPoints[0], juce::Colours::lightgrey, edge->name);
        drawDot(edge->edgeControlPoints[1], juce::Colours::lightgrey, edge->name);
    }

    for (auto& face : pimpl->faces)
    {
        for (auto& cp : face->innerControlPoints)
        {
            drawDot(cp, juce::Colours::lightblue, face->name);
        }
    }
#endif
}

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
        auto bottomRightPosition = existingBottomEdge.getPointAlongLineProportionally(-1.0f);

        auto topRight = pimpl->addVertex(topLeft->gridIndex.x + 1, topLeft->gridIndex.y, topRightPosition);
        auto bottomRight = pimpl->addVertex(bottomLeft->gridIndex.x + 1, bottomLeft->gridIndex.y, bottomRightPosition);

        auto topEdge = pimpl->addEdge(Orientation::horizontal, topLeft, topRight);
        auto rightEdge = pimpl->addEdge(Orientation::vertical, topRight, bottomRight);
        auto bottomEdge = pimpl->addEdge(Orientation::horizontal, bottomLeft, bottomRight);
        auto leftEdge = existingFace->edges[(size_t)Direction::east];

        auto face = pimpl->addFace(topLeft, topRight, bottomRight, bottomLeft,
            topEdge, rightEdge, bottomEdge, leftEdge);
        face->colors = colors;
        face->setInnerControlPoints();

        patches.add(new Patch{ face });
        break;
    }

    case GradientMesh::Direction::south:
    {
        auto topLeft = existingFace->vertices[(size_t)Corner::bottomLeft];
        auto topRight = existingFace->vertices[(size_t)Corner::bottomRight];

        auto existingLeftEdge = existingFace->edges[(size_t)Direction::west]->toLine();
        auto bottomLeftPosition = existingLeftEdge.getPointAlongLineProportionally(2.0f);

        auto existingRightEdge = existingFace->edges[(size_t)Direction::east]->toLine();
        auto bottomRightPosition = existingRightEdge.getPointAlongLineProportionally(2.0f);

        auto bottomLeft = pimpl->addVertex(topLeft->gridIndex.x, topLeft->gridIndex.y + 1, bottomLeftPosition);
        auto bottomRight = pimpl->addVertex(topRight->gridIndex.x, topRight->gridIndex.y + 1, bottomRightPosition);

        auto topEdge = existingFace->edges[(size_t)Direction::south];
        auto rightEdge = pimpl->addEdge(Orientation::vertical, topRight, bottomRight);
        auto bottomEdge = pimpl->addEdge(Orientation::horizontal, bottomRight, bottomLeft);
        auto leftEdge = pimpl->addEdge(Orientation::vertical, bottomLeft, topLeft);

        auto face = pimpl->addFace(topLeft, topRight, bottomRight, bottomLeft,
            topEdge, rightEdge, bottomEdge, leftEdge);
        face->colors = colors;
        face->setInnerControlPoints();

        patches.add(new Patch{ face });
        break;
    }
    }

    return nullptr;
}

GradientMesh::Patch::Patch(Face* face_) :
    pimpl(std::make_unique<PatchPimpl>(*this)),
    face(face_)
{
}

GradientMesh::Patch::~Patch()
{
}

juce::Rectangle<float> GradientMesh::Patch::getBounds() const noexcept
{
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
}

#if 0
juce::Point<float> GradientMesh::Patch::getControlPointPosition(GridPosition gridPosition) const
{
    return pimpl->getControlPoint(gridPosition).position;
}

void GradientMesh::Patch::setControlPointPosition(GridPosition gridPosition, juce::Point<float> pos)
{
    pimpl->getControlPoint(gridPosition).position = pos;
}

std::optional<juce::Colour> GradientMesh::Patch::getControlPointColor(GridPosition gridPosition) const
{
    return pimpl->getControlPoint(gridPosition).color;
}

void GradientMesh::Patch::setControlPointColor(GridPosition gridPosition, juce::Colour color)
{
    pimpl->getControlPoint(gridPosition).color = color;
}
#endif
