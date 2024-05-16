#include "GradientMeshEditor.h"

static Path makePath()
{
    juce::Path p;
    //p.addRoundedRectangle(juce::Rectangle<float>{ 100.0f, 100.0f, 500.0f, 500.0f }, 25.0f);
    //p.addStar({ 350.0f, 350.0f }, 8, 200.0f, 300.0f);
    //p.addEllipse(10.0f, 10.0f, 500.0f, 500.0f);
    p.addPolygon({ 400.0f, 400.0f }, 7, 300.0f);
    //p.applyTransform(juce::AffineTransform::rotation(0.2f, p.getBounds().getCentreX(), p.getBounds().getCentreY()));
    //p.addRectangle(10.0f, 10.0f, 500.0f, 500.0f);
    //p.addRectangle(200.0f, 200.0f, 500.0f, 500.0f);

    return p;
}

GradientMeshEditor::GradientMeshEditor() :
    mesher(makePath()),
    halfedgeMesh(makePath())
{
    setOpaque(false);

    halfedgeMesh.updateMesh();
#if 0
    auto clone = mesh.clonePatch(firstPatch, GradientMesh::Direction::north);
    clone->setUpperLeftColor(colors[1]);
    clone->setUpperRightColor(colors[2]);
    clone->setLowerRightColor(colors[2]);
    clone->setLowerLeftColor(colors[1]);
    clone->applyTransform(juce::AffineTransform::rotation(angle + juce::MathConstants<float>::twoPi * -0.25f, radius, radius));

    clone = mesh.clonePatch(clone, GradientMesh::Direction::west);
    clone->setUpperLeftColor(colors[2]);
    clone->setUpperRightColor(colors[3]);
    clone->setLowerRightColor(colors[3]);
    clone->setLowerLeftColor(colors[2]);
    clone->applyTransform(juce::AffineTransform::rotation(angle + juce::MathConstants<float>::twoPi * -0.25f, radius, radius));

    clone = mesh.clonePatch(clone, GradientMesh::Direction::south);
    clone->setUpperLeftColor(colors[3]);
    clone->setUpperRightColor(endColor);
    clone->setLowerRightColor(endColor);
    clone->setLowerLeftColor(colors[3]);
    clone->applyTransform(juce::AffineTransform::rotation(angle + juce::MathConstants<float>::twoPi * -0.25f, radius, radius));
#endif

#if 0
    for (auto patch : mesh.getPatches())
    {
        auto patchComponent = std::make_unique<PatchComponent>(patch);
        addAndMakeVisible(patchComponent.get());
        patchComponents.emplace_back(std::move(patchComponent));
    }
#endif

    for (auto const& subpath : halfedgeMesh.subpaths)
    {
        for (auto const& vertex : subpath.vertices)
        {
            auto vertexComponent = std::make_unique<VertexComponent>(vertex.get());
            addAndMakeVisible(vertexComponent.get());

            juce::Component::SafePointer<VertexComponent> safePointer{ vertexComponent.get() };
            vertexComponent->onMouseOver = [this, safePointer]()
                {
                    if (safePointer)
                        highlightVertex(safePointer);
                };

            vertexComponents.emplace_back(std::move(vertexComponent));
        }

        for (auto const& halfedge : subpath.halfedges)
        {
            auto edgeComponent = std::make_unique<HalfEdgeComponent>(halfedge.get());
            addAndMakeVisible(edgeComponent.get());
            edgeComponent->toBack();

            juce::Component::SafePointer<HalfEdgeComponent> edgeComponentSafePointer{ edgeComponent.get() };
            edgeComponent->onMouseOver = [this, edgeComponentSafePointer]()
                {
                    if (edgeComponentSafePointer)
                        highlightEdge(edgeComponentSafePointer);
                };
            edgeComponent->onMouseExit = [this]()
                {
                    clearHighlights();
                };
            edgeComponents.emplace_back(std::move(edgeComponent));
        }

        for (auto const& face : subpath.faces)
        {
            auto faceComponent = std::make_unique<FaceComponent>(face.get());
            addAndMakeVisible(faceComponent.get());
            faceComponent->toBack();

            faceComponents.emplace_back(std::move(faceComponent));
        }
    }

}

void GradientMeshEditor::createConic(float rotationAngle)
{
#if 0
    //
    // Squish the upper right corner onto the upper left corner so the top edge has zero length
    //
    //                            RIGHT EDGE
    //     P00 / P01 / P02 / P03 -----------P33
    //              |                        |
    //              |                       /
    //              |                      /
    //              |                     /
    //              |                    /
    //              |                   /
    //            L |                  /
    //            E |                 / B
    //            F |                / O
    //            T |               / T
    //              |              / T
    //            E |             / O
    //            D |            / M
    //            G |           /
    //            E |          / E
    //              |         / D
    //              |        / G
    //              |       / E
    //              |      /
    //              |     /
    //              |    /
    //              |   /
    //               P30
    //

    float radius = (float)juce::jmin(getWidth() * 0.5f, getHeight() * 0.5f);
    auto center = juce::Point<float>{ getWidth() * 0.5f, getHeight() * 0.5f };
    auto upperRight = center + juce::Point<float>{ radius, 0.0f };
    auto lowerLeft = center + juce::Point<float>{ 0.0f, radius};
    juce::Point<float> radialPoint = center.getPointOnCircumference(radius, juce::MathConstants<float>::twoPi * (90.0f + 45.0f) / 360.0f);

    GradientMesh::PatchOptions options;
    auto startColor = juce::Colours::red;
    auto endColor = juce::Colours::violet;
    std::array<juce::Colour, 5> colors
    {
        startColor,
        juce::Colours::orange,
        juce::Colours::cyan,
        juce::Colours::orange,
        endColor
    };

    options.upperLeftCorner = { center, startColor };
    options.upperRightCorner = { center, colors[1] };
    options.lowerRightCorner = { upperRight, colors[1] };
    options.lowerLeftCorner = { lowerLeft, startColor };

    options.topEdge = { center, center };

    juce::Line<float> leftEdgeLine(center, lowerLeft);
    options.leftEdge = GradientMesh::PatchOptions::VerticalEdge{ leftEdgeLine.getPointAlongLineProportionally(0.5f), leftEdgeLine.getPointAlongLineProportionally(0.5f), GradientMesh::EdgeAliasingMode::inflated };

    juce::Line<float> rightEdgeLine(center, upperRight);
    options.rightEdge = { rightEdgeLine.getPointAlongLineProportionally(0.5f), rightEdgeLine.getPointAlongLineProportionally(0.5f), GradientMesh::EdgeAliasingMode::inflated };

    auto controlPointOffset = radius * 0.55f;
    options.bottomEdge = { lowerLeft.translated(controlPointOffset, 0.0f), upperRight.translated(0.0f, controlPointOffset), GradientMesh::EdgeAliasingMode::inflated };

    mesh.reset();
    auto firstPatch = mesh.addPatch(options);
    firstPatch->applyTransform(juce::AffineTransform::rotation(rotationAngle, center.x, center.y));

    auto clone = mesh.clonePatch(firstPatch, GradientMesh::Direction::north);
    clone->setEdgeAliasingMode(GradientMesh::Edge::leftEdge, GradientMesh::EdgeAliasingMode::inflated);
    clone->setEdgeAliasingMode(GradientMesh::Edge::rightEdge, GradientMesh::EdgeAliasingMode::inflated);
    clone->setUpperLeftColor(colors[1]);
    clone->setUpperRightColor(colors[2]);
    clone->setLowerRightColor(colors[2]);
    clone->setLowerLeftColor(colors[1]);
    clone->applyTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::twoPi * -0.25f, center.x, center.y));

    clone = mesh.clonePatch(clone, GradientMesh::Direction::west);
    clone->setUpperLeftColor(colors[2]);
    clone->setUpperRightColor(colors[3]);
    clone->setLowerRightColor(colors[3]);
    clone->setLowerLeftColor(colors[2]);
    clone->applyTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::twoPi * -0.25f, center.x, center.y));

    clone = mesh.clonePatch(clone, GradientMesh::Direction::south);
    clone->setEdgeAliasingMode(GradientMesh::Edge::rightEdge, GradientMesh::EdgeAliasingMode::inflated);
    clone->setUpperLeftColor(colors[3]);
    clone->setUpperRightColor(endColor);
    clone->setLowerRightColor(endColor);
    clone->setLowerLeftColor(colors[3]);
    clone->applyTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::twoPi * -0.25f, center.x, center.y));
#endif
}

void GradientMeshEditor::createSinglePatch()
{
#if 0
    mesh.reset();

    GradientMesh::PatchOptions options;
    auto bounds = getLocalBounds().toFloat().reduced(100.0f);

    float alpha = (float)std::sin(phase) * 0.5f + 0.5f;
    options.upperLeftCorner = { bounds.getTopLeft(), juce::Colours::magenta.withAlpha(alpha) };

    alpha = (float)std::sin(phase + juce::MathConstants<float>::halfPi) * 0.5f + 0.5f;
    options.upperRightCorner = { bounds.getTopRight(), juce::Colours::cyan.withAlpha(alpha) };

    alpha = (float)std::sin(phase + juce::MathConstants<float>::pi) * 0.5f + 0.5f;
    options.lowerRightCorner = { bounds.getBottomRight(), juce::Colours::yellow.withAlpha(alpha) };

    alpha = (float)std::sin(phase + 1.5f * juce::MathConstants<float>::pi) * 0.5f + 0.5f;
    options.lowerLeftCorner = { bounds.getBottomLeft(), juce::Colours::aliceblue.withAlpha(alpha) };

    options.topEdge = { { bounds.proportionOfWidth(0.25f), bounds.getY() - 50.0f }, { bounds.proportionOfWidth(0.75f), bounds.getY() + 50.0f} };
    options.leftEdge = { { bounds.getX() - 50.0f, bounds.proportionOfHeight(0.25f) }, { bounds.getX() - 50.0f, bounds.proportionOfHeight(0.75f) } };
    options.rightEdge = { { bounds.getRight() + 50.0f, bounds.proportionOfHeight(0.25f) }, { bounds.getRight() + 50.0f, bounds.proportionOfHeight(0.75f) } };
    options.bottomEdge = { { bounds.proportionOfWidth(0.25f), bounds.getBottom() - 50.0f }, { bounds.proportionOfWidth(0.75f), bounds.getBottom() + 50.0f } };

    mesh.addPatch(options);
#endif
}

GradientMeshEditor::~GradientMeshEditor()
{
}

juce::Rectangle<int> GradientMeshEditor::getPreferredSize()
{
    return { 2048, 1024 };
    //return mesh.getBounds().toNearestInt().expanded(50);
}

void GradientMeshEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

#if 0
    mesher.path.applyTransform(juce::AffineTransform::rotation((float)rotationAngle, mesher.path.getBounds().getCentreX(), mesher.path.getBounds().getCentreY()));
    mesher.updateMesh();
    mesher.draw(meshImage, {});
    g.drawImageAt(meshImage, 0, 0);
#endif

    for (auto const& subpath : halfedgeMesh.subpaths)
    {
        paintSubpath(g, subpath, mesher.path.getBounds());
    }
}

void GradientMeshEditor::paintSubpath(juce::Graphics& g, const HalfEdgeMesh::Subpath& subpath, juce::Rectangle<float> area)
{
#if 0
    auto drawArrow = [&](juce::Colour color, HalfEdgeMesh::Halfedge* halfedge, float offset)
        {
            g.setColour(color);
            juce::Line<float> line{ halfedge->tailVertex->point, halfedge->headVertex->point };
            line = line.withShortenedEnd(40.0f).withShortenedStart(40.0f);
            auto angle = line.getAngle();
            auto tail = line.getStart().getPointOnCircumference(offset, angle + juce::MathConstants<float>::halfPi);
            auto head = line.getEnd().getPointOnCircumference(offset, angle + juce::MathConstants<float>::halfPi);
            g.drawArrow(juce::Line<float>{ tail, head}, 2.0f, 10.0f, 10.0f);
        };


    for (auto const& face : subpath.faces)
    {
        Path path;
//         path.addArrow({ face->halfedges[0]->tailVertex->point, face->halfedges[0]->headVertex->point }, 2.0f, 10.0f, 10.0f);
//         path.addArrow({ face->halfedges[1]->tailVertex->point, face->halfedges[1]->headVertex->point }, 2.0f, 10.0f, 10.0f);
//         path.addArrow({ face->halfedges[2]->tailVertex->point, face->halfedges[2]->headVertex->point }, 2.0f, 10.0f, 10.0f);
        path.addTriangle(face->halfedges[0]->tailVertex->point, face->halfedges[1]->tailVertex->point, face->halfedges[2]->tailVertex->point);

        path.applyTransform(juce::AffineTransform::scale(0.9f, 0.9f, path.getBounds().getCentreX(), path.getBounds().getCentreY()));
        g.setColour(juce::Colours::green);
        //g.strokePath(path, juce::PathStrokeType(2.0f));
        g.fillPath(path);
    }
#endif
}

void GradientMeshEditor::resized()
{
    meshImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
    //createConic();
    //createSinglePatch();
    //mesh.draw(meshImage, {});

    for (auto& vertexComponent : vertexComponents)
    {
        vertexComponent->setBounds(getLocalBounds());
    }

    for (auto& edgeComponent : edgeComponents)
    {
        edgeComponent->setBounds(getLocalBounds());
    }

    for (auto& faceComponent : faceComponents)
    {
        faceComponent->setBounds(getLocalBounds());
    }
}

void GradientMeshEditor::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
#if 0
    wheel.deltaY > 0 ? zoom *= 1.1f : zoom *= 0.9f;
    zoom = juce::jlimit(0.1f, 10.0f, zoom);
    setTransform(juce::AffineTransform::scale(zoom, zoom, getWidth() * 0.5f, getHeight() * 0.5f));
    repaint();
#endif
}

void GradientMeshEditor::mouseMove(const MouseEvent& event)
{
}

void GradientMeshEditor::clearHighlights()
{
    for (auto const& vertexComponent : vertexComponents)
    {
        vertexComponent->setAlpha(0.1f);
        vertexComponent->highlighted = false;
    }

    for (auto const& edgeComponent : edgeComponents)
    {
        edgeComponent->setAlpha(0.1f);
        edgeComponent->highlighted = false;
    }
}

void GradientMeshEditor::highlightVertex(VertexComponent* vertexComponent)
{
    clearHighlights();

    vertexComponent->setAlpha(0.75f);
    vertexComponent->highlighted = true;

#if 0
    for (auto const& edgeComponent : edgeComponents)
    {
        if (auto edge = edgeComponent->edge.lock())
        {
            auto vertexPair = edge->getVertices();
            if (vertexPair.first == vertexComponent->vertex.lock() || vertexPair.second == vertexComponent->vertex.lock())
            {
                edgeComponent->setAlpha(1.0f);
                edgeComponent->highlighted = true;
            }
        }
    }
#endif
}

void GradientMeshEditor::highlightEdge(HalfEdgeComponent* edgeComponent)
{
    clearHighlights();

    edgeComponent->setAlpha(0.75f);
    edgeComponent->highlighted = true;

#if 0
    if (auto edge = edgeComponent->edge.lock())
    {
        for (auto const& vertexComponent : vertexComponents)
        {
            auto vertexPair = edge->getVertices();
            if (vertexPair.first == vertexComponent->vertex.lock() || vertexPair.second == vertexComponent->vertex.lock())
            {
                vertexComponent->setAlpha(1.0f);
            }
        }
    }
#endif
}

GradientMeshEditor::FaceComponent::FaceComponent(const HalfEdgeMesh::Face* const face_) :
    face(face_)
{
    setOpaque(false);
    setRepaintsOnMouseActivity(true);

    path.addTriangle(face->halfedges[0]->tailVertex->point, face->halfedges[1]->tailVertex->point, face->halfedges[2]->tailVertex->point);
}

bool GradientMeshEditor::FaceComponent::hitTest(int x, int y)
{
    return path.contains((float)x, (float)y);
}

void GradientMeshEditor::FaceComponent::mouseEnter(const juce::MouseEvent& event)
{

}

void GradientMeshEditor::FaceComponent::mouseExit(const MouseEvent& event)
{

}

void GradientMeshEditor::FaceComponent::paint(juce::Graphics& g)
{
    if (isMouseOver(true))
    {
        g.setColour(juce::Colours::purple);
        g.fillPath(path);
    }
}

GradientMeshEditor::VertexComponent::VertexComponent(const HalfEdgeMesh::Vertex* const vertex_) :
    vertex(vertex_)
{
    setAlpha(0.1f);
    //setInterceptsMouseClicks(false, false);
}

bool GradientMeshEditor::VertexComponent::hitTest(int x, int y)
{
    auto hitPoint = juce::Point<int>{ x, y }.toFloat();
    auto distance = vertex->point.getDistanceFrom(hitPoint);
    return distance < 20.0f;
}

void GradientMeshEditor::VertexComponent::mouseEnter(const juce::MouseEvent& event)
{
    if (onMouseOver)
        onMouseOver();
}

void GradientMeshEditor::VertexComponent::mouseExit(const MouseEvent& event)
{
}

void GradientMeshEditor::VertexComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::red);

    g.fillEllipse(juce::Rectangle<float>{ 30.0f, 30.0f}.withCentre(vertex->point));
    if (highlighted)
        g.drawArrow({ vertex->halfedge->tailVertex->point, vertex->halfedge->headVertex->point }, 2.0f, 8.0f, 8.0f);
}

GradientMeshEditor::HalfEdgeComponent::HalfEdgeComponent(const HalfEdgeMesh::Halfedge* halfedge_) :
    halfedge(halfedge_)
{
    setAlpha(0.1f);

    juce::Line<float> line{ halfedge->tailVertex->point, halfedge->headVertex->point };
    line = line.withShortenedEnd(40.0f).withShortenedStart(40.0f);
    auto angle = line.getAngle();
    auto tail = line.getStart().getPointOnCircumference(10.0f, angle + juce::MathConstants<float>::halfPi);
    auto head = line.getEnd().getPointOnCircumference(10.0f, angle + juce::MathConstants<float>::halfPi);
    paintedLine = juce::Line<float>{ tail, head };
    //setInterceptsMouseClicks(false, false);
}

bool GradientMeshEditor::HalfEdgeComponent::hitTest(int x, int y)
{
    auto hitPoint = juce::Point<int>{ x, y }.toFloat();
    auto nearestPoint = paintedLine.findNearestPointTo(hitPoint);
    auto distance = nearestPoint.getDistanceFrom(hitPoint);
    return distance < 3.0f;
}

void GradientMeshEditor::HalfEdgeComponent::mouseEnter(const juce::MouseEvent& event)
{
    if (onMouseOver)
        onMouseOver();
}

void GradientMeshEditor::HalfEdgeComponent::mouseExit(const MouseEvent& event)
{
   if (onMouseExit)
     onMouseExit();
}

void GradientMeshEditor::HalfEdgeComponent::paint(juce::Graphics& g)
{
    auto origin = getPosition().toFloat();

    g.setColour(juce::Colours::aqua);
    g.drawArrow(paintedLine, 2.0f, 10.0f, 10.0f);

    if (highlighted)
    {
        if (halfedge->next)
        {
            g.setColour(juce::Colours::orange);
            g.drawArrow({ halfedge->next->tailVertex->point, halfedge->next->headVertex->point }, 2.0f, 8.0f, 8.0f);
        }

        if (halfedge->previous)
        {
            g.setColour(juce::Colours::yellow);
            g.drawArrow({ halfedge->previous->tailVertex->point, halfedge->previous->headVertex->point }, 2.0f, 8.0f, 8.0f);
        }
    }
}
