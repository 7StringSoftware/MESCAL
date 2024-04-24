#include "GradientMeshEditor.h"

GradientMeshEditor::GradientMeshEditor()
{
    setOpaque(false);

    juce::Rectangle<float> patchBounds{ 50.0f, 50.0f, 200.0f, 200.0f };
    mesh.addPatch(patchBounds);

    patchBounds.translate(200.0f, 0.0f);
    mesh.addPatch(patchBounds);

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
    return mesh.getBounds().toNearestInt().expanded(50);
}

void GradientMeshEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    mesh.draw(meshImage, {});
    g.drawImageAt(meshImage, 0, 0);
}

void GradientMeshEditor::resized()
{
    auto const firstPatch = mesh.getPatches().getFirst();

    for (auto& controlPointComponent : controlPointComponents)
    {
        auto pos = firstPatch->getControlPointPosition(controlPointComponent->gridPosition).roundToInt();
        controlPointComponent->setSize(32, 32);
        controlPointComponent->setCentrePosition(pos.x, pos.y);
    }

    meshImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);

    for (auto& patchComponent : patchComponents)
    {
        auto bounds = patchComponent->patch->getBounds().toNearestInt();
        patchComponent->setBounds(bounds);
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

GradientMeshEditor::PatchComponent::PatchComponent(GradientMesh::Patch::Ptr patch_) :
    patch(patch_)
{
    setOpaque(false);
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
        g.setColour(juce::Colours::white);
        g.drawRect(getLocalBounds());
    }
}
