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
    //path.addRectangle(100.0f, 100.0f, 500.0f, 250.0f);
    path.addRoundedRectangle(Rectangle<float>{ 100.0f, 100.0f, 500.0f, 500.0f }, 100.0f);

    gradientMesh = GradientMesh::pathToGrid(path, {}, 1.0f);
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
