#include "GradientMeshEditor.h"

static Path makePath()
{
    juce::Path p;
    p.addRoundedRectangle(juce::Rectangle<float>{ 100.0f, 100.0f, 500.0f, 500.0f }, 25.0f);
    //p.addStar({ 350.0f, 350.0f }, 8, 200.0f, 300.0f);
    //p.addEllipse(10.0f, 10.0f, 500.0f, 500.0f);
    p.addPolygon({ 400.0f, 400.0f }, 7, 300.0f);
    p.applyTransform(juce::AffineTransform::rotation(0.2f, p.getBounds().getCentreX(), p.getBounds().getCentreY()));
    //p.addRectangle(100.0f, 100.0f, 500.0f, 500.0f);
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
#if 0
    auto now = juce::Time::getMillisecondCounterHiRes();
    auto elapsedSeconds = (now - lastMsec) * 0.001;
    lastMsec = now;
    auto constexpr cyclesPerSecond = 0.1;
    phase += cyclesPerSecond * elapsedSeconds * juce::MathConstants<double>::twoPi;

    createSinglePatch();
    //createConic((float)phase);
    mesh.draw(meshImage, {});

    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::darkgrey);
    g.setFont(getHeight() * 0.3f);
    g.drawFittedText("Gradient Mesh", getLocalBounds(), juce::Justification::centred, 1);

    {
        g.beginTransparencyLayer(0.75f);
        g.drawImageAt(meshImage, 0, 0);
        g.endTransparencyLayer();
    }

    g.setTiledImageFill(meshImage, 0, 0, 1.0f);
    g.drawFittedText("Gradient Mesh", getLocalBounds(), juce::Justification::centred, 1);
#endif

#if 0
    Path p;
    p.addRectangle(getLocalBounds().reduced(100).toFloat());

    PathFlatteningIterator iterator{ p };
    std::vector<juce::Point<float>> points;
    if (iterator.next())
    {
        points.push_back({ iterator.x1, iterator.y1 });
        points.push_back({ iterator.x2, iterator.y2 });
    }
    while (iterator.next())
    {
        points.push_back({ iterator.x2, iterator.y2 });
    }

    std::vector<Vector2d> triangles;
    Triangulate::Process(points, triangles);

    {
        g.setColour(juce::Colours::darkgrey);
        g.fillPath(p);

        auto num = triangles.size() / 3;
        jassert(triangles.size() % 3 == 0);
        for (auto i = 0; i < num; i += 3)
        {
            g.setColour(juce::Colours::white);
            juce::Path p2;
            p2.addTriangle(triangles[i], triangles[i + 1], triangles[i + 2]);
            g.fillPath(p2);
        }
    }
#endif

    g.setColour(juce::Colours::darkgrey);
    g.fillPath(mesher.path);

    auto bounds = mesher.path.getBounds();

    for (auto const& vertex : mesher.perimeterVertices)
    {
        g.setColour(juce::Colours::red);
        g.fillEllipse(juce::Rectangle<float>{ 10.0f, 10.0f}.withCentre(vertex.point));
    }

    for (auto const& edge : mesher.perimeterEdges)
    {
        if (edge.type == Mesher::Edge::Type::cubic)
        {
            g.setColour(juce::Colours::white);

            Path p;
            p.startNewSubPath(edge.line.getStart());
            p.cubicTo(edge.controlPoints[0].value(), edge.controlPoints[1].value(), edge.line.getEnd());
            g.strokePath(p, juce::PathStrokeType(2.0f));
            continue;
        }

        g.setColour(juce::Colours::pink);

        g.drawArrow(edge.line, 2.0f, 10.0f, 10.0f);
    }

#if 0
    for (auto const& quad : mesher.quads)
    {
        g.setColour(juce::Colours::white);
        juce::Path p;
        //p.addQuadrilateral(quad.vertices[0].x, quad.vertices[0].y, quad.vertices[1].x, quad.vertices[1].y, quad.vertices[2].x, quad.vertices[2].y, quad.vertices[3].x, quad.vertices[3].y);
        p.addArrow({ quad.vertices[0], quad.vertices[1] }, 3.0f, 10.0f, 10.0f);
        p.addArrow({ quad.vertices[1], quad.vertices[2] }, 3.0f, 10.0f, 10.0f);
        p.addArrow({ quad.vertices[2], quad.vertices[3] }, 3.0f, 10.0f, 10.0f);
        p.addArrow({ quad.vertices[3], quad.vertices[0] }, 3.0f, 10.0f, 10.0f);

        //g.strokePath(p, juce::PathStrokeType(1.0f));
        g.fillPath(p);
    }
#endif
}

void GradientMeshEditor::resized()
{
    meshImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
    //createConic();
    //createSinglePatch();
    //mesh.draw(meshImage, {});
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

GradientMeshEditor::PatchComponent::PatchComponent(GradientMesh::Patch::Ptr patch_) :
    patch(patch_)
{
    setOpaque(false);
    setRepaintsOnMouseActivity(true);
}

bool GradientMeshEditor::PatchComponent::hitTest(int x, int y)
{
    return juce::Component::hitTest(x, y);
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
        g.fillAll(juce::Colours::lightgrey.withAlpha(0.15f));
        g.setColour(juce::Colours::white);
        g.drawRect(getLocalBounds());
    }
}
