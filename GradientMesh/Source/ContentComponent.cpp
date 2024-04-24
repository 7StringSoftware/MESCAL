#include "ContentComponent.h"

ContentComponent::ContentComponent()
{
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&meshEditor);

    setSize(1024, 1024);
}

void ContentComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void ContentComponent::resized()
{
    meshEditor.setBounds(getLocalBounds().getUnion(meshEditor.getPreferredSize()));
    viewport.setBounds(getLocalBounds());
}
