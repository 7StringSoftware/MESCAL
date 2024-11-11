/*
  ==============================================================================

    ContentComponent.cpp
    Created: 7 Sep 2024 10:20:46am
    Author:  Matt Gonzalez

  ==============================================================================
*/

#include "ContentComponent.h"

ContentComponent::ContentComponent()
{
    addAndMakeVisible(widgets);
    addAndMakeVisible(effectGraph);

    setSize(1024, 800);

    juce::MessageManager::callAsync([this]
        {
            effectGraph.resized();
            repaint();
        });

    effectGraph.onPropertyChange = [this]
        {
            widgets.repaint();
        };
}

void ContentComponent::resized()
{
    auto r = getLocalBounds();
    auto widgetsBounds = r.removeFromLeft(r.getWidth() / 2);
    widgets.setBounds(widgetsBounds);
    effectGraph.setBounds(r);
}
