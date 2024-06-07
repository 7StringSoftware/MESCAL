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
    gradientMesh = std::make_unique<GradientMesh>();
    juce::Path path;
    //path.addEllipse({ 0.0f, 0.0f, 1000.0f, 1000.0f });
    //path.addRoundedRectangle(Rectangle<float>{ 100.0f, 100.0f, 500.0f, 500.0f }, 50.0f);

    juce::Rectangle<float> r{ 100.0f, 100.0f, 0.0f, 500.0f };
    for (float pos = 0.0f; pos <= 1.0f; pos += 0.1f)
    {
        float scaledPos = std::cos(pos * juce::MathConstants<float>::pi);
        float width = scaledPos * 20.0f + 20.0f;
        path.clear();
        r.setWidth(width);
        path.addRectangle(r);
        r.translate(width, 0);

        juce::Colour c1, c2;
        c1 = juce::Colours::darkred.interpolatedWith(juce::Colours::red, 0.5f - pos * 0.5f);
        c2 = juce::Colours::darkred.interpolatedWith(juce::Colours::red, 0.5f + (pos + 0.1f) * 0.5f);
        gradientMesh->addPatch(path, { c1.darker(), c1, c2, c2.darker()});
    }

    //
    //gradientMesh->addPatch(path, { juce::Colours::red, juce::Colours::violet, juce::Colours::yellow, juce::Colours::blue });

#if 0
    auto center = path.getBounds().getCentre();
    auto radius = juce::jmin(path.getBounds().getWidth(), path.getBounds().getHeight()) * 0.5f;

    GradientMesh::PathOptions options;
    options.nominalPatchHeight = 250.0f;
    options.nominalPatchWidth = 20.0f;
    options.tolerance = 1.0f;
    options.findVertexColor = [&](std::shared_ptr<GradientMesh::Vertex> v) -> juce::Colour
        {
            //auto angle = center.getAngleToPoint(v->position);
            //auto distance = center.getDistanceFrom(v->position);
            float diff = std::abs(v->position.x - 100.0f) / 200.0f;
            //return //juce::Colour{ 0.75f + 0.25f * std::sin(angle), juce::jlimit(0.0f, 1.0f, distance / radius), 1.0f, 1.0f };
            float angle = diff * juce::MathConstants<float>::twoPi;
            auto level = std::sin(angle) * 0.5f + 0.5f;
            float height = (500.0f - v->position.y) / 500.0f;
            return juce::Colour::fromHSV(level * 0.1f, 1.0f, height * 0.5f + 0.5f, 1.0f);
        };
    gradientMesh = GradientMesh::pathToGridAlt(path, options);
#endif
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
