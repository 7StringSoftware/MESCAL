#include "GradientMeshEditor.h"

GradientMeshEditor::GradientMeshEditor()
{

}

void GradientMeshEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    paintMesh(g);
}

void GradientMeshEditor::resized()
{
    auto localBounds = getLocalBounds();
    int size = juce::jmin(localBounds.getWidth(), localBounds.getHeight());
    meshImageBounds = juce::Rectangle<int>{ size, size }.reduced(100).withCentre(localBounds.getCentre()).toFloat();
}

void GradientMeshEditor::createMesh()
{
    mesh = std::make_unique<mescal::GradientMesh>(2, 2);

    auto meshBounds = meshImageBounds.withZeroOrigin().reduced(100);

    auto northwest = mesh->getVertex(0, 0);
    northwest->position = meshBounds.getTopLeft();
    northwest->setColors(juce::Colours::red);

    auto northeast = mesh->getVertex(1, 0);
    northeast->position = meshBounds.getTopRight();
    northeast->setColors(juce::Colours::green);

    auto southwest = mesh->getVertex(0, 1);
    southwest->position = meshBounds.getBottomLeft();
    southwest->setColors(juce::Colours::blue);

    auto southeast = mesh->getVertex(1, 1);
    southeast->position = meshBounds.getBottomRight();
    southeast->setColors(juce::Colours::yellow);
}

void GradientMeshEditor::paintMesh(juce::Graphics& g)
{
    if (meshImageBounds.isEmpty())
        return;

    if (meshImage.isNull() || meshImage.getWidth() != (int)meshImageBounds.getWidth() || meshImage.getHeight() != (int)meshImageBounds.getHeight())
        meshImage = juce::Image(juce::Image::ARGB, meshImageBounds.getWidth(), meshImageBounds.getHeight(), true);

    if (!mesh)
        createMesh();

    mesh->draw(meshImage, {}, juce::Colours::transparentBlack);
    g.drawImage(meshImage, meshImageBounds.toFloat(), juce::RectanglePlacement::centred);
}
