#include "GradientMeshEditor.h"

GradientMeshEditor::GradientMeshEditor() :
    mesh(juce::Rectangle<float>{ 50.0f, 50.0f, 400.0f, 400.0f })
{
    setOpaque(false);

    mesh.addConnectedPatch(nullptr, GradientMesh::Direction::east,
        { juce::Colours::yellow, juce::Colours::red, juce::Colours::violet, juce::Colours::blue });
    mesh.addConnectedPatch(nullptr, GradientMesh::Direction::south,
        { juce::Colours::blue, juce::Colours::violet, juce::Colours::red, juce::Colours::yellow });

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
