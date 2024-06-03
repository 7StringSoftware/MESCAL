#include "ContentComponent.h"

ContentComponent::ContentComponent(juce::ApplicationCommandManager& commandManager_) :
    meshEditor(commandManager_)
{
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&meshEditor);

    setSize(2048, 1024);
}

void ContentComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void ContentComponent::resized()
{
    //meshEditor.setBounds(getLocalBounds().getUnion(meshEditor.getPreferredSize()));
    meshEditor.setBounds(getLocalBounds());
    viewport.setBounds(getLocalBounds());
}
