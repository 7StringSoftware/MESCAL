#include "GradientMeshEditor.h"

static Path makePath()
{
    juce::Path p;
    //p.addRoundedRectangle(juce::Rectangle<float>{ 100.0f, 100.0f, 500.0f, 500.0f }, 25.0f);
    //p.addStar({ 350.0f, 350.0f }, 8, 200.0f, 300.0f);
    //p.addEllipse(10.0f, 10.0f, 500.0f, 500.0f);
    //p.addPolygon({ 400.0f, 400.0f }, 7, 300.0f);
    //p.applyTransform(juce::AffineTransform::rotation(0.2f, p.getBounds().getCentreX(), p.getBounds().getCentreY()));
    p.addRectangle(10.0f, 10.0f, 500.0f, 500.0f);
    //p.addRectangle(200.0f, 200.0f, 500.0f, 500.0f);

    return p;
}

GradientMeshEditor::GradientMeshEditor() :
    mesher(makePath())
{
    setOpaque(false);

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

    for (auto const& subpath : mesher.subpaths)
    {
        for (auto const& vertex : subpath.vertices)
        {
            auto vertexComponent = std::make_unique<VertexComponent>(vertex);
            //addAndMakeVisible(vertexComponent.get());

            juce::Component::SafePointer<VertexComponent> safePointer{ vertexComponent.get() };
            vertexComponent->onMouseOver = [this, safePointer]()
                {
                    if (safePointer)
                        highlightVertex(safePointer);
                };

            vertexComponents.emplace_back(std::move(vertexComponent));
        }

        for (auto const& edge : subpath.edges)
        {
            auto edgeComponent = std::make_unique<EdgeComponent>(edge);
            //addAndMakeVisible(edgeComponent.get());
            edgeComponent->toBack();

            juce::Component::SafePointer<EdgeComponent> edgeComponentSafePointer{ edgeComponent.get() };
            edgeComponent->onMouseOver = [this, edgeComponentSafePointer]()
                {
                    if (edgeComponentSafePointer)
                        highlightEdge(edgeComponentSafePointer);
                };

            edgeComponents.emplace_back(std::move(edgeComponent));
        }

        for (auto const& patch : subpath.patches)
        {
            auto patchComponent = std::make_unique<PatchComponent>(patch);
            //addAndMakeVisible(patchComponent.get());
            patchComponent->toBack();

            patchComponents.emplace_back(std::move(patchComponent));
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

     mesher.path.applyTransform(juce::AffineTransform::rotation((float)rotationAngle, mesher.path.getBounds().getCentreX(), mesher.path.getBounds().getCentreY()));
     mesher.updateMesh();
     mesher.draw(meshImage, {});
    g.drawImageAt(meshImage, 0, 0);

    for (auto const& subpath : mesher.subpaths)
    {
        //paintSubpath(g, subpath, mesher.path.getBounds());
    }
}

void GradientMeshEditor::paintSubpath(juce::Graphics& g, const Mesher::Subpath& subpath, juce::Rectangle<float> area)
{
    for (auto const& vertex : subpath.vertices)
    {
        g.setColour(juce::Colours::red.withAlpha(0.5f));
        g.fillEllipse(juce::Rectangle<float>{ 10.0f, 10.0f }.withCentre(vertex->point));
    }

    float x = 0.0f;
    for (auto const& edge : subpath.edges)
    {
        g.setColour(juce::Colours::green.withAlpha(0.75f));

        auto vertexPair = edge->getVertices();
        if (vertexPair.first && vertexPair.second)
        {
            auto offset = Point<float>{ x, 0.0f };
            g.drawLine({ vertexPair.first->point + offset, vertexPair.second->point + offset }, 2.0f);
        }

        x += area.getWidth();
    }
}

void GradientMeshEditor::resized()
{
    meshImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
    //createConic();
    //createSinglePatch();
    //mesh.draw(meshImage, {});

    for (auto& vertexComponent : vertexComponents)
    {
        if (auto vertex = vertexComponent->vertex.lock())
        {
            vertexComponent->setSize(30, 30);

            auto center = vertex->point.roundToInt();
            vertexComponent->setCentrePosition(center.x, center.y);
        }
    }

    for (auto& edgeComponent : edgeComponents)
    {
        if (auto edge = edgeComponent->edge.lock())
        {
            auto vertexPair = edge->getVertices();
            if (vertexPair.first && vertexPair.second)
            {
                edgeComponent->setBounds(juce::Rectangle<float>{ vertexPair.first->point, vertexPair.second->point }.expanded(5.0f).toNearestInt());
            }
        }
    }

    for (auto& patchComponent : patchComponents)
    {
        patchComponent->setBounds(getLocalBounds());
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
        vertexComponent->setAlpha(0.5f);
    }

    for (auto const& edgeComponent : edgeComponents)
    {
        edgeComponent->setAlpha(0.5f);
        edgeComponent->highlighted = false;
    }
}

void GradientMeshEditor::highlightVertex(VertexComponent* vertexComponent)
{
    clearHighlights();

    vertexComponent->setAlpha(1.0f);

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
}

void GradientMeshEditor::highlightEdge(EdgeComponent* edgeComponent)
{
    clearHighlights();

    edgeComponent->setAlpha(1.0f);
    edgeComponent->highlighted = true;

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
}

GradientMeshEditor::PatchComponent::PatchComponent(std::weak_ptr<Mesher::Patch> patch_) :
    patch(patch_)
{
    setOpaque(false);
    setRepaintsOnMouseActivity(true);

    if (auto lock = patch.lock())
    {
        auto it = lock->edges.begin();
        auto lastPoint = it->lock()->endpoints[0].vertex.lock()->point;
        path.startNewSubPath(lastPoint);

        while (it != lock->edges.end())
        {
            auto nextPoint = it->lock()->endpoints[1].vertex.lock()->point;
            if (nextPoint == lastPoint)
                nextPoint = it->lock()->endpoints[0].vertex.lock()->point;

            switch (it->lock()->type)
            {
            case Mesher::Edge::Type::line:
                path.lineTo(nextPoint);
                break;

            case Mesher::Edge::Type::quadratic:
                path.quadraticTo(it->lock()->controlPoints[0].value_or(juce::Point<float>{}), nextPoint);
                break;

            case Mesher::Edge::Type::cubic:
                path.cubicTo(it->lock()->controlPoints[0].value_or(juce::Point<float>{}), it->lock()->controlPoints[1].value_or(juce::Point<float>{}), nextPoint);
                break;

            default:
                jassertfalse;
                break;
            }

            lastPoint = nextPoint;
            ++it;
        }
    }
}

bool GradientMeshEditor::PatchComponent::hitTest(int x, int y)
{
    return path.contains((float)x, (float)y);
}

void GradientMeshEditor::PatchComponent::mouseEnter(const juce::MouseEvent& event)
{

}

void GradientMeshEditor::PatchComponent::mouseExit(const MouseEvent& event)
{

}

void GradientMeshEditor::PatchComponent::paint(juce::Graphics& g)
{
    if (isMouseOver(true))
    {
        g.setColour(juce::Colours::purple);
        g.fillPath(path);
    }
}

GradientMeshEditor::VertexComponent::VertexComponent(std::weak_ptr<Mesher::Vertex> vertex_) :
    vertex(vertex_)
{
    setAlpha(0.7f);
    //setInterceptsMouseClicks(false, false);
}

bool GradientMeshEditor::VertexComponent::hitTest(int x, int y)
{
    return juce::Component::hitTest(x, y);
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
    g.fillEllipse(getLocalBounds().toFloat());
}

GradientMeshEditor::EdgeComponent::EdgeComponent(std::weak_ptr<Mesher::Edge> edge_) :
    edge(edge_)
{
    setAlpha(0.7f);
    //setInterceptsMouseClicks(false, false);
}

bool GradientMeshEditor::EdgeComponent::hitTest(int x, int y)
{
    return juce::Component::hitTest(x, y);
}

void GradientMeshEditor::EdgeComponent::mouseEnter(const juce::MouseEvent& event)
{
    if (onMouseOver)
        onMouseOver();
}

void GradientMeshEditor::EdgeComponent::mouseExit(const MouseEvent& event)
{

}

void GradientMeshEditor::EdgeComponent::paint(juce::Graphics& g)
{
    //g.drawRect(getLocalBounds());

    if (auto e = edge.lock())
    {
        auto vertexPair = e->getVertices();
        if (vertexPair.first && vertexPair.second)
        {
            auto origin = getPosition().toFloat();

            auto color = highlighted ? juce::Colours::green : juce::Colours::blue;
            g.setColour(color);
            g.drawLine({ vertexPair.first->point - origin, vertexPair.second->point - origin }, 4.0f);
        }
    }
}
