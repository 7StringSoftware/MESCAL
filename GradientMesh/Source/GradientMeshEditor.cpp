#include "GradientMeshEditor.h"

GradientMeshEditor::GradientMeshEditor()
{
    setOpaque(false);

    createConic();

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

void GradientMeshEditor::createConic()
{
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
    auto mousePos = getMouseXYRelative();
    auto center = juce::Point<float>{ getWidth() * 0.5f, getHeight() * 0.5f };
    auto upperRight = center + juce::Point<float>{ radius, 0.0f };
    auto lowerLeft = center + juce::Point<float>{ 0.0f, radius};
    juce::Point<float> radialPoint = center.getPointOnCircumference(radius, juce::MathConstants<float>::twoPi * (90.0f + 45.0f) / 360.0f);

    auto angle = center.getAngleToPoint(mousePos.toFloat());// +juce::MathConstants<float>::pi;

    GradientMesh::PatchOptions options;
    auto startColor = juce::Colours::magenta;
    auto endColor = juce::Colours::magenta.withAlpha(0.0f);
    std::array<juce::Colour, 5> colors
    {
        startColor,
        startColor.interpolatedWith(endColor, 0.25f),
        startColor.interpolatedWith(endColor, 0.50f),
        startColor.interpolatedWith(endColor, 0.75f),
        endColor
    };

    options.upperLeftCorner = { center, startColor };
    options.upperRightCorner = { center, colors[1] };
    options.lowerRightCorner = { upperRight, colors[1] };
    options.lowerLeftCorner = { lowerLeft, startColor };

    options.topEdge = { center, center };

    juce::Line<float> leftEdge(center, lowerLeft);
    options.leftEdge = { leftEdge.getPointAlongLineProportionally(0.5f), leftEdge.getPointAlongLineProportionally(0.5f) };

    juce::Line<float> rightEdge(center, upperRight);
    options.rightEdge = { rightEdge.getPointAlongLineProportionally(0.5f), rightEdge.getPointAlongLineProportionally(0.5f) };

    auto controlPointOffset = radius * 0.55f;
    options.bottomEdge = { lowerLeft.translated(controlPointOffset, 0.0f), upperRight.translated(0.0f, controlPointOffset) };

    mesh.reset();
    auto firstPatch = mesh.addPatch(options);

    auto clone = mesh.clonePatch(firstPatch, GradientMesh::Direction::north);
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
    clone->setUpperLeftColor(colors[3]);
    clone->setUpperRightColor(endColor);
    clone->setLowerRightColor(endColor);
    clone->setLowerLeftColor(colors[3]);
    clone->applyTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::twoPi * -0.25f, center.x, center.y));
}

void GradientMeshEditor::createSinglePatch()
{
    mesh.reset();


    GradientMesh::PatchOptions options;
    auto bounds = getLocalBounds().toFloat().reduced(100.0f);

    float alpha = (float)std::sin(phase) * 0.5f + 0.5f;
    options.upperLeftCorner = { bounds.getTopLeft(), juce::Colours::magenta.withAlpha(alpha)};

    alpha = (float)std::sin(phase + juce::MathConstants<float>::halfPi) * 0.5f + 0.5f;
    options.upperRightCorner = { bounds.getTopRight(), juce::Colours::cyan.withAlpha(alpha)};
    
    alpha = (float)std::sin(phase + juce::MathConstants<float>::pi) * 0.5f + 0.5f;
    options.lowerRightCorner = { bounds.getBottomRight(), juce::Colours::yellow.withAlpha(alpha)};
    
    alpha = (float)std::sin(phase + 1.5f * juce::MathConstants<float>::pi) * 0.5f + 0.5f;
    options.lowerLeftCorner = { bounds.getBottomLeft(), juce::Colours::aliceblue.withAlpha(alpha) };

    options.topEdge = { { bounds.proportionOfWidth(0.25f), bounds.getY() - 50.0f }, { bounds.proportionOfWidth(0.75f), bounds.getY() + 50.0f} };
    options.leftEdge = { { bounds.getX() - 50.0f, bounds.proportionOfHeight(0.25f) }, { bounds.getX() - 50.0f, bounds.proportionOfHeight(0.75f) } };
    options.rightEdge = { { bounds.getRight() + 50.0f, bounds.proportionOfHeight(0.25f) }, { bounds.getRight() + 50.0f, bounds.proportionOfHeight(0.75f) } };
    options.bottomEdge = { { bounds.proportionOfWidth(0.25f), bounds.getBottom() - 50.0f }, { bounds.proportionOfWidth(0.75f), bounds.getBottom() + 50.0f } };

    mesh.addPatch(options);

    phase += 0.01;
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
    createSinglePatch();
    mesh.draw(meshImage, {});

    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);
    g.setFont(getHeight() * 0.3f);
    g.drawFittedText("Gradient Mesh", getLocalBounds(), juce::Justification::centred, 1);

    {
        g.beginTransparencyLayer(0.75f);
        g.drawImageAt(meshImage, 0, 0);
        g.endTransparencyLayer();
    }

    g.setTiledImageFill(meshImage, 0, 0, 1.0f);
    g.drawFittedText("Gradient Mesh", getLocalBounds(), juce::Justification::centred, 1);
}

void GradientMeshEditor::resized()
{
    meshImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
    //createConic();
    createSinglePatch();
    mesh.draw(meshImage, {});
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
