/*
  ==============================================================================

    Document.cpp
    Created: 23 Apr 2024 9:25:14am
    Author:  Matt Gonzalez

  ==============================================================================
*/

#include "Document.h"

Document::Document() :
    juce::FileBasedDocument(".mescal.gradientmesh", "*.mescal.gradientmesh", "Open a gradient mesh file", "Save gradient mesh")
{
    juce::Path path;
    path.addEllipse({ 0.0f, 0.0f, 1000.0f, 1000.0f });
    //path.addRectangle(100.0f, 100.0f, 500.0f, 500.0f);
    //path.addRoundedRectangle(Rectangle<float>{ 100.0f, 100.0f, 500.0f, 500.0f }, 50.0f);

    auto center = path.getBounds().getCentre();
    auto radius = juce::jmin(path.getBounds().getWidth(), path.getBounds().getHeight()) * 0.5f;

    GradientMesh::PathOptions options;
    options.nominalPatchHeight = 100.0f;
    options.nominalPatchWidth = 100.0f;
    options.findVertexColor = [&](std::shared_ptr<GradientMesh::Vertex> v) -> juce::Colour
        {
            auto angle = center.getAngleToPoint(v->position);
            auto distance = center.getDistanceFrom(v->position);
            return juce::Colour{ 0.75f + 0.25f * std::sin(angle), juce::jlimit(0.0f, 1.0f, distance / radius), 1.0f, 1.0f };
        };
    gradientMesh = GradientMesh::pathToGridAlt(path, options);
}

Document::~Document()
{
}

juce::Result Document::loadDocument(const juce::File& file)
{
    return juce::Result::ok();
}

juce::Result Document::saveDocument(const juce::File& file)
{
    return juce::Result::ok();
}

juce::File Document::getLastDocumentOpened()
{
    return {};
}

void Document::setLastDocumentOpened(const File&)
{
}

juce::String Document::getDocumentTitle()
{
    return "Gradient Mesh";
}
