#include "ContentComponent.h"

ContentComponent::ContentComponent(juce::ApplicationCommandManager& commandManager_, Settings& settings_)
{
    addAndMakeVisible(meshDemo);

    setSize(2048, 1024);
}

void ContentComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void ContentComponent::resized()
{
    meshDemo.setBounds(getLocalBounds());
}
