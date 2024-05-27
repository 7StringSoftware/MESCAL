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
    gradientMesh.addPatch(GradientMesh::Patch::create());
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
