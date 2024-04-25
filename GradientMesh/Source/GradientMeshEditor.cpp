#include "GradientMeshEditor.h"

GradientMeshEditor::GradientMeshEditor()
{
    setOpaque(false);

    float radius = 500.0f;
    auto center = juce::Point<float>{ 0.0f, 0.0f };
    auto upperRight = center + juce::Point<float>{ radius, 0.0f };
    auto lowerLeft = center + juce::Point<float>{ 0.0f, radius};
    juce::Point<float> radialPoint = center.getPointOnCircumference(radius, juce::MathConstants<float>::twoPi * (90.0f + 45.0f) / 360.0f);

    GradientMesh::PatchOptions options;
#if 0
    options.upperLeftCorner = { center, juce::Colours::yellow };
    options.upperRightCorner = { upperRight, juce::Colours::blue };
    options.lowerRightCorner = { radialPoint, juce::Colours::yellow };
    options.lowerLeftCorner = { lowerLeft, juce::Colours::violet };

    juce::Line<float> leftEdge(center, lowerLeft);
    options.leftEdge = { leftEdge.getPointAlongLineProportionally(0.25f), leftEdge.getPointAlongLineProportionally(0.75f) };

    juce::Line<float> topEdge(center, upperRight);
    options.topEdge = { topEdge.getPointAlongLineProportionally(0.25f), topEdge.getPointAlongLineProportionally(0.75f) };

    auto distance = radius * 0.25f;
    auto angle = center.getAngleToPoint(radialPoint);
    auto tangentLine = juce::Line<float>::fromStartAndAngle(radialPoint, distance, angle + juce::MathConstants<float>::halfPi);
    options.rightEdge = { upperRight.translated(0.0f, distance), tangentLine.getPointAlongLineProportionally(-1.0f) };

    options.bottomEdge = { lowerLeft.translated(distance, 0.0f), tangentLine.getPointAlongLineProportionally(1.0f) };
#else
    options.upperLeftCorner = { center, juce::Colours::yellow };
    options.upperRightCorner = { center, juce::Colours::blue };
    options.lowerRightCorner = { upperRight, juce::Colours::blue };
    options.lowerLeftCorner = { lowerLeft, juce::Colours::yellow };

    options.topEdge = { center, center };

    juce::Line<float> leftEdge(center, lowerLeft);
    options.leftEdge = { leftEdge.getPointAlongLineProportionally(0.33f), leftEdge.getPointAlongLineProportionally(0.66f) };

    juce::Line<float> rightEdge(center, upperRight);
    options.rightEdge = { rightEdge.getPointAlongLineProportionally(0.33f), rightEdge.getPointAlongLineProportionally(0.66f) };


    auto distance = radius * 0.50f;
    auto angle = center.getAngleToPoint(radialPoint);
    auto tangentLine = juce::Line<float>::fromStartAndAngle(radialPoint, distance, angle + juce::MathConstants<float>::halfPi);
    //options.rightEdge = { upperRight.translated(0.0f, distance), tangentLine.getPointAlongLineProportionally(-1.0f) };

    options.bottomEdge = { lowerLeft.translated(distance, 0.0f), upperRight.translated(0.0f, distance) };
#endif

    auto firstPatch = mesh.addPatch(options);

#if 0
    auto clone = mesh.clonePatch(firstPatch, GradientMesh::Direction::east);
    clone->translate(500.0f, 0.0f);
    clone->flipColorsHorizontally();

    clone = mesh.clonePatch(clone, GradientMesh::Direction::south);
    clone->translate(0.0f, 500.0f);
    clone->flipControlPointsVertically();

    clone = mesh.clonePatch(firstPatch, GradientMesh::Direction::south);
    clone->translate(0.0f, 500.0f);
    clone->flipControlPointsVertically();
#endif


#if 0
    mesh.addConnectedPatch(nullptr, GradientMesh::Direction::east,
        { juce::Colours::yellow, juce::Colours::red, juce::Colours::violet, juce::Colours::blue });

    mesh.addConnectedPatch(nullptr, GradientMesh::Direction::south,
        { juce::Colours::blue, juce::Colours::violet, juce::Colours::red, juce::Colours::yellow });

    mesh.addConnectedPatch(nullptr, GradientMesh::Direction::west,
        { juce::Colours::violet, juce::Colours::blue, juce::Colours::yellow, juce::Colours::red });
#endif

#if 0
    auto patch = mesh.getPatches().getFirst();

    for (int index = 0; index < GradientMesh::Patch::numControlPoints; ++index)
    {
        auto gridPosition = GradientMesh::Patch::indexToGridPosition(index);
        auto controlPointComponent = std::make_unique<ControlPointComponent>(gridPosition, patch->getControlPointColor(gridPosition));

        String name;
        name << gridPosition.row << gridPosition.column;
        controlPointComponent->setName(name);

        juce::Component::SafePointer<ControlPointComponent> compSafePointer = controlPointComponent.get();
        controlPointComponent->onMove = [=]()
            {
                if (compSafePointer != nullptr)
                {
                    auto pos = compSafePointer->getBounds().toFloat().getCentre();
                    mesh.setControlPointPosition(patch, gridPosition, pos.toFloat());
                    if (compSafePointer->color.has_value())
                        mesh.setControlPointColor(patch, gridPosition, *(compSafePointer->color));

                    repaint();
                }
            };

        addAndMakeVisible(controlPointComponent.get());
        controlPointComponents.emplace_back(std::move(controlPointComponent));
    }
#endif

    for (auto patch : mesh.getPatches())
    {
        auto patchComponent = std::make_unique<PatchComponent>(patch);
        addAndMakeVisible(patchComponent.get());
        patchComponents.emplace_back(std::move(patchComponent));
    }
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

    mesh.draw(meshImage, {});
    g.drawImageAt(meshImage, 0, 0);
}

void GradientMeshEditor::resized()
{
#if 0
    auto const firstPatch = mesh.getPatches().getFirst();

    for (auto& controlPointComponent : controlPointComponents)
    {
        auto pos = firstPatch->getControlPointPosition(controlPointComponent->gridPosition).roundToInt();
        controlPointComponent->setSize(32, 32);
        controlPointComponent->setCentrePosition(pos.x, pos.y);
    }

    for (auto& patchComponent : patchComponents)
    {
        auto bounds = patchComponent->patch->getBounds().toNearestInt();
        patchComponent->setBounds(bounds);
    }
#endif

    meshImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);

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
