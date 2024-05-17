#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <d2d1_3helper.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#include <JuceHeader.h>
#include <juce_graphics/native/juce_DirectX_windows.h>
#include <juce_graphics/native/juce_Direct2DImage_windows.h>
#include "HalfEdgeMesh.h"

struct HalfEdgeMesh::Pimpl
{
    Pimpl(HalfEdgeMesh& owner_) : owner(owner_)
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

    void faceToPatch(const HalfEdgeMesh::Face& face, D2D1_GRADIENT_MESH_PATCH& patch)
    {
        struct Corner
        {
            D2D1_POINT_2F& point;
            D2D1_POINT_2F& innerControlPoint;
            D2D1_COLOR_F& color;
            std::pair<D2D1_POINT_2F&, D2D1_POINT_2F&> controlPoints;

            void set(HalfEdgeMesh::Vertex* vertex)
            {
                point = { vertex->point.x, vertex->point.y };
                innerControlPoint = point;
                color = D2DUtilities::toCOLOR_F(vertex->color);
                controlPoints.first = point;
                controlPoints.second = point;
            }
        };

        Corner topLeft{ patch.point00, patch.point11, patch.color00, { patch.point01, patch.point10 } };
        Corner topRight{ patch.point03, patch.point12, patch.color03, { patch.point02, patch.point13 } };
        Corner bottomLeft{ patch.point30, patch.point21, patch.color30, { patch.point20, patch.point31 } };
        Corner bottomRight{ patch.point33, patch.point22, patch.color33, { patch.point23, patch.point32 } };

        topLeft.set(face.halfedges[0]->tailVertex); // top edge collapses; top left & top right corners coincide
        topRight.set(face.halfedges[0]->tailVertex);
        bottomRight.set(face.halfedges[1]->tailVertex);
        bottomLeft.set(face.halfedges[1]->headVertex);

        struct Edge
        {
            std::pair<D2D1_POINT_2F&, D2D1_POINT_2F&> controlPoints;

            void set(HalfEdgeMesh::Halfedge* halfedge)
            {
                if (halfedge->controlPoints.has_value())
                {
                    controlPoints.first = { halfedge->controlPoints->first.x, halfedge->controlPoints->first.y };
                    controlPoints.second = { halfedge->controlPoints->second.x, halfedge->controlPoints->second.y };
                }
            }
        };

        Edge leftToRightTopEdge{ { patch.point01, patch.point02 } };
        Edge topToBottomRightEdge{ { patch.point13, patch.point23 } };
        Edge rightToLeftBottomEdge{ { patch.point32, patch.point31 } };
        Edge bottomToTopLeftEdge{ { patch.point20, patch.point10 } };

        topToBottomRightEdge.set(face.halfedges[0]);
        rightToLeftBottomEdge.set(face.halfedges[1]);
        bottomToTopLeftEdge.set(face.halfedges[2]);
    }

    HalfEdgeMesh& owner;
    winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
    winrt::com_ptr<ID2D1GradientMesh> gradientMesh;
};

void HalfEdgeMesh::Vertex::dump() const
{
    DBG("Vertex " << point.toString() << "  halfedge:" << (halfedge ? halfedge->print() : "null"));
}

HalfEdgeMesh::HalfEdgeMesh(Path&& p) :
    pimpl(std::make_unique<Pimpl>(*this)),
    path(p)
{

}

HalfEdgeMesh::~HalfEdgeMesh()
{

}

void HalfEdgeMesh::updateMesh(int numPatchEdges /*= 4*/)
{
    subpaths.clear();

    juce::Path::Iterator it{ path };

    //
    // Find perimeter vertices
    //
    Vertex subpathStart;
    while (it.next())
    {
        //DBG(print(it));

        switch (it.elementType)
        {
        case Path::Iterator::startNewSubPath:
            iterateSubpath(it, juce::Point<float>{ it.x1, it.y1 });
            break;

        case Path::Iterator::lineTo:
        case Path::Iterator::cubicTo:
        case Path::Iterator::quadraticTo:
            iterateSubpath(it, juce::Point<float>{ it.x1, it.y1 });
            break;

        default:
            break;
        }
    }
}

void HalfEdgeMesh::draw(juce::Image image, juce::AffineTransform transform)
{
    std::vector<D2D1_GRADIENT_MESH_PATCH> patches;
    patches.reserve(subpaths.size());

    for (auto const& subpath : subpaths)
    {
        for (auto const& face : subpath.faces)
        {
            if (face->halfedges[0]->type == Halfedge::Type::line && face->halfedges[1]->type == Halfedge::Type::line && face->halfedges[2]->type == Halfedge::Type::line)
            {
                continue;
            }   

            auto& patch = patches.emplace_back(D2D1_GRADIENT_MESH_PATCH{});

            pimpl->faceToPatch(*face.get(), patch);
        }
    }

    pimpl->createResources(image);

    if (pimpl->deviceContext && image.isValid())
    {
        pimpl->deviceContext->CreateGradientMesh(patches.data(), patches.size(), pimpl->gradientMesh.put());

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

void HalfEdgeMesh::iterateSubpath(juce::Path::Iterator& it, Point<float> subpathStart)
{
    auto& subpath = subpaths.emplace_back(Subpath{});

    auto bounds = path.getBounds();
    auto center = bounds.getCentre();

    struct PathPoint
    {
        juce::Point<float> point;
        Halfedge::Type type = Halfedge::Type::unknown;
        std::optional<ControlPoints> controlPoints;
    };
    std::vector<PathPoint> pathPoints;

    while (it.next())
    {
        auto edgeType = Halfedge::Type::unknown;
        switch (it.elementType)
        {
        case Path::Iterator::startNewSubPath:
        {
            iterateSubpath(it, juce::Point<float>{ it.x1, it.y1 });
            return;
        }

        case Path::Iterator::lineTo:
        {
            pathPoints.emplace_back(PathPoint{ juce::Point<float>{ it.x1, it.y1 }, Halfedge::Type::line });
            break;
        }

        case Path::Iterator::quadraticTo:
        {
            pathPoints.emplace_back(PathPoint{ juce::Point<float>{ it.x2, it.y2 },
                Halfedge::Type::quadratic,
                ControlPoints{ juce::Point<float>{ it.x1, it.y1 }, juce::Point<float>{ it.x1, it.y1 } } });
            break;
        }

        case Path::Iterator::cubicTo:
        {
            pathPoints.emplace_back(PathPoint{ juce::Point<float>{ it.x3, it.y3 },
                Halfedge::Type::cubic,
                ControlPoints{ juce::Point<float>{ it.x1, it.y1 }, juce::Point<float>{ it.x2, it.y2 } } });
            break;
        }

        case Path::Iterator::closePath:
        {
            pathPoints.emplace_back(PathPoint{ subpathStart, Halfedge::Type::line });
            break;
        }
        }
    }

    auto addHalfedge = [&](Vertex* tailVertex, Vertex* headVertex, Halfedge::Type type, std::optional<ControlPoints> controlPoints) -> Halfedge*
        {
            auto halfedge = std::make_unique<Halfedge>();
            halfedge->type = type;
            halfedge->tailVertex = tailVertex;
            halfedge->headVertex = headVertex;

            auto twin = std::make_unique<Halfedge>();
            twin->type = halfedge->type;
            twin->tailVertex = halfedge->headVertex;
            twin->headVertex = halfedge->tailVertex;

            halfedge->twin = twin.get();
            twin->twin = halfedge.get();

            if (controlPoints.has_value())
            {
                halfedge->controlPoints = controlPoints;
                twin->controlPoints = ControlPoints{ controlPoints->second, controlPoints->first };
            }

            subpath.halfedges.push_back(std::move(twin));
            subpath.halfedges.push_back(std::move(halfedge));

            return subpath.halfedges.back().get();
        };

    //
    // Add vertices & halfedges around the perimeter
    //
    size_t colorIndex = 0;
    for (const auto& pathPoint : pathPoints)
    {
        std::array<juce::Colour, 8> colors = { juce::Colours::red, juce::Colours::green, juce::Colours::blue, juce::Colours::yellow, juce::Colours::cyan, juce::Colours::magenta, juce::Colours::orange, juce::Colours::purple };
        subpath.vertices.emplace_back(std::make_unique<Vertex>(pathPoint.point, colors[colorIndex]));
        colorIndex = (colorIndex + 1) % colors.size();
    }

    std::vector<Halfedge*> perimeterHalfedges;
    for (auto index = 0; index < pathPoints.size(); ++index)
    {
        auto& vertex = subpath.vertices[index];
        auto nextIndex = (index + 1) % pathPoints.size();
        auto& nextVertex = subpath.vertices[nextIndex];
        auto perimeterHalfedge = addHalfedge(vertex.get(), nextVertex.get(), pathPoints[nextIndex].type, pathPoints[nextIndex].controlPoints);
        vertex->halfedge = perimeterHalfedge;
        perimeterHalfedges.push_back(perimeterHalfedge);
    }

    //
    // Add center vertex & halfedges to/from center
    //
    auto centerVertex = std::make_unique<Vertex>(center);
    std::vector<Halfedge*> centerHalfedges;
    for (size_t index = 0; index < perimeterHalfedges.size(); index++)
    {
        auto centerHalfedge = addHalfedge(perimeterHalfedges[index]->tailVertex, centerVertex.get(), perimeterHalfedges[index]->type, {});
        centerHalfedges.push_back(centerHalfedge);

        auto& perimeterHalfedge = perimeterHalfedges[index];
        auto& previousPerimeterHalfedge = perimeterHalfedges[(index + perimeterHalfedges.size() - 1) % perimeterHalfedges.size()];
        perimeterHalfedge->next = centerHalfedge;
        perimeterHalfedge->previous = previousPerimeterHalfedge->twin;

        centerHalfedge->next = perimeterHalfedge;
        centerHalfedge->previous = previousPerimeterHalfedge->twin;

        previousPerimeterHalfedge->twin->previous = centerHalfedge;
        previousPerimeterHalfedge->twin->next = perimeterHalfedge;
    }

    //
    // Set next/previous pointers for center halfedges
    //
    for (size_t index = 0; index < centerHalfedges.size(); ++index)
    {
        size_t nextIndex = (index + 1) % centerHalfedges.size();
        size_t previousIndex = (index + centerHalfedges.size() - 1) % centerHalfedges.size();
        auto& centerHalfedge = centerHalfedges[index]->twin;
        centerHalfedge->next = centerHalfedges[nextIndex]->twin;
        centerHalfedge->previous = centerHalfedges[previousIndex]->twin;
    }

    centerVertex->halfedge = subpath.halfedges.back()->twin;
    centerVertex->color = juce::Colours::hotpink;
    subpath.vertices.push_back(std::move(centerVertex));

    subpath.iterateFaces(perimeterHalfedges);

    for (auto const& face : subpath.faces)
    {
        DBG("Face:");
        for (auto const& edge : face->halfedges)
        {
            edge->dump();
        }
    }
}

void HalfEdgeMesh::Subpath::iterateFaces(const std::vector<Halfedge*>& perimeterHalfedges)
{
    for (auto const& perimeterHalfedge : perimeterHalfedges)
    {
        auto face = std::make_unique<Face>();
        face->halfedges[0] = perimeterHalfedge;
        face->halfedges[1] = perimeterHalfedge->twin->previous;
        face->halfedges[2] = face->halfedges[1]->twin->previous;
        faces.push_back(std::move(face));
    }
}
