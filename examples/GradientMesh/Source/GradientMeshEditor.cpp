#include "GradientMeshEditor.h"

GradientMeshEditor::GradientMeshEditor()
{
    for (auto& vertexComponent : vertexComponents)
    {
        addAndMakeVisible(vertexComponent);
    }
}

void GradientMeshEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    paintMesh(g);
}

void GradientMeshEditor::resized()
{
    meshImageBounds = getLocalBounds().toFloat();

    createMesh();

    for (auto& vertexComponent : vertexComponents)
    {
        if (auto lock = vertexComponent.vertex.lock())
        {
            auto center = lock->position.toInt();
            vertexComponent.setBounds(juce::Rectangle<int>{ 30, 30 }.withCentre({ center.x, center.y }));
        }
        vertexComponent.onChange = [this] { repaint(); };
    }

    for (auto& patchComponent : patchComponents)
    {
        patchComponent->setBounds(getLocalBounds());
    }
}

void GradientMeshEditor::createMesh()
{
    mesh = std::make_unique<mescal::GradientMesh>(2, 2);

    auto meshBounds = meshImageBounds.withZeroOrigin().reduced(100);

    auto northwest = mesh->getVertex(0, 0);
    northwest->position = meshBounds.getTopLeft();
    northwest->setColors(juce::Colours::red);
    vertexComponents[0].vertex = northwest;

    auto northeast = mesh->getVertex(1, 0);
    northeast->position = meshBounds.getTopRight();
    northeast->setColors(juce::Colours::green);
    vertexComponents[1].vertex = northeast;

    auto southeast = mesh->getVertex(1, 1);
    southeast->position = meshBounds.getBottomRight();
    southeast->setColors(juce::Colours::yellow);
    vertexComponents[2].vertex = southeast;

    auto southwest = mesh->getVertex(0, 1);
    southwest->position = meshBounds.getBottomLeft();
    southwest->setColors(juce::Colours::blue);
    vertexComponents[3].vertex = southwest;

    patchComponents.clear();
    patchComponents.emplace_back(std::make_unique<PatchComponent>(northwest));
    addAndMakeVisible(patchComponents.back().get());
}

void GradientMeshEditor::paintMesh(juce::Graphics& g)
{
    if (meshImageBounds.isEmpty())
        return;

    if (meshImage.isNull() || meshImage.getWidth() != (int)getWidth() || meshImage.getHeight() != (int)meshImageBounds.getHeight())
        meshImage = juce::Image(juce::Image::ARGB, meshImageBounds.getWidth(), meshImageBounds.getHeight(), true);

    mesh->draw(meshImage, {}, juce::Colours::transparentBlack);
    g.drawImage(meshImage, meshImageBounds.toFloat(), juce::RectanglePlacement::centred);
}

void GradientMeshEditor::VertexComponent::mouseDown(const juce::MouseEvent& e)
{
    dragger.startDraggingComponent(this, e);
}

void GradientMeshEditor::VertexComponent::mouseDrag(const juce::MouseEvent& e)
{
    dragger.dragComponent(this, e, nullptr);
    if (auto lock = vertex.lock())
    {
        lock->position = getBounds().getCentre().toFloat();
        if (onChange)
            onChange();
    }
}

void GradientMeshEditor::VertexComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::white);
    g.fillEllipse(getLocalBounds().toFloat());
}

GradientMeshEditor::PatchComponent::PatchComponent(std::weak_ptr<mescal::GradientMesh::Vertex> northwestCorner_) :
    northwestCorner(northwestCorner_)
{
    updateOutline();
}

void GradientMeshEditor::PatchComponent::mouseDown(const juce::MouseEvent& e)
{

}

void GradientMeshEditor::PatchComponent::mouseMove(const juce::MouseEvent& e)
{

}

void GradientMeshEditor::PatchComponent::updateOutline()
{
    outline.clear();

    auto nw = northwestCorner.lock();
    if (!nw)
        return;

    outline.startNewSubPath(nw->position);

    auto east = nw->eastHalfedge.lock();
    if (!east)
        return;

    auto ne = east->head.lock();
    if (!ne)
        return;
    outline.lineTo(ne->position);

    auto south = ne->southHalfedge.lock();
    if (!south)
        return;

    auto se = south->head.lock();
    if (!se)
        return;

    auto west = se->westHalfedge.lock();
    if (!west)
        return;

    auto sw = west->tail.lock();
    if (!sw)
        return;

    outline.lineTo(sw->position);
    outline.closeSubPath();
}

void GradientMeshEditor::PatchComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::white);
    g.strokePath(outline, juce::PathStrokeType(2.0f));
}

bool GradientMeshEditor::PatchComponent::hitTest(int x, int y)
{
    return outline.contains(juce::Point<int>(x, y).toFloat());
}
