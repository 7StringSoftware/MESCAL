#include "GradientMeshEditor.h"

GradientMeshEditor::GradientMeshEditor()
{
    setOpaque(true);

    auto const& patch = mesh.getPatches().front();

    for (int index = 0; index < GradientMesh::Patch::numControlPoints; ++index)
    {
        auto gridPosition = GradientMesh::Patch::indexToGridPosition(index);
        auto controlPointComponent = std::make_unique<ControlPointComponent>(gridPosition, patch->getControlPointColorValue(gridPosition));

        String name;
        name << gridPosition.row << gridPosition.column;
        controlPointComponent->setName(name);

        juce::Component::SafePointer<ControlPointComponent> compSafePointer = controlPointComponent.get();
        controlPointComponent->onMove([=]()
            {
                if (compSafePointer != nullptr)
                {
                    auto pos = compSafePointer->getCentrePosition();
                    auto normalizedPos = juce::Point<float>{ pos.x, pos.y }.toFloat() / getLocalBounds().toFloat().reduced(100);
                    patch->setControlPointPosition(gridPosition, normalizedPos);
                }
            });

            void[=]()
            {
                auto gridPosition = controlPointComponent->gridPosition;
                auto pos = patch->getNormalizedControlPointPosition(gridPosition);
                controlPointComponent->setCentrePosition(pos.x, pos.y);
            });

        addAndMakeVisible(controlPointComponent.get());
        controlPointComponents.emplace_back(std::move(controlPointComponent));
    }
}

GradientMeshEditor::~GradientMeshEditor()
{
}

void GradientMeshEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    mesh.draw(g, {});
}

void GradientMeshEditor::resized()
{
    auto const& patch = mesh.getPatches().front();

    juce::Rectangle<float> area = getLocalBounds().reduced(100).toFloat();
    for (auto& controlPointComponent : controlPointComponents)
    {
        auto normalizedCenter = patch->getNormalizedControlPointPosition(controlPointComponent->gridPosition);

        auto x = area.proportionOfWidth(normalizedCenter.x);
        auto y = area.proportionOfHeight(normalizedCenter.y);
        auto center = (juce::Point<float>{ x, y } + area.getTopLeft()).roundToInt();

        controlPointComponent->setSize(32, 32);
        controlPointComponent->setCentrePosition(center.x, center.y);
    }
}
