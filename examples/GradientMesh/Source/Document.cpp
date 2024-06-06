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
    //path.addEllipse({ 100.0f, 100.0f, 500.0f, 500.0f });
    path.addRectangle(100.0f, 100.0f, 500.0f, 500.0f);
    //path.addRoundedRectangle(Rectangle<float>{ 100.0f, 350.0f, 500.0f, 200.0f }, 50.0f);

    float xStart = 100.0f;
    float bottom = 600.0f;
    float w = 500.0f;
    float h = 500.0f;
    float centerX = xStart + w * 0.5f;
    float xScale = 1.0f / w;
    float yScale = 1.0f / h;

    GradientMesh::PathOptions options;
    options.nominalPatchHeight = 10.0f;
    options.nominalPatchWidth = 10.0f;
    options.findVertexColor = [&](std::shared_ptr<GradientMesh::Vertex> v) -> juce::Colour
        {
            auto normalizedX = (v->position.x - xStart) * xScale;
            auto red = std::sin(0.5f * normalizedX * juce::MathConstants<float>::twoPi) * 0.75f + 0.25f;
            auto color = juce::Colours::darkgoldenrod.withAlpha(red);
            if (0.1f < normalizedX && normalizedX < 0.2f)
            {
                color = color.interpolatedWith(juce::Colours::white.withAlpha(0.2f), (bottom - v->position.y) * yScale);
            }
            return color;
        };
    gradientMesh = GradientMesh::pathToGrid(path, options);
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
