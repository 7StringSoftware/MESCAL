/*
  ==============================================================================

    ContentComponent.cpp
    Created: 7 Sep 2024 10:20:46am
    Author:  Matt Gonzalez

  ==============================================================================
*/

#include "ContentComponent.h"

ContentComponent::ContentComponent(mescal::Effect::Ptr effectIn) : effect(effectIn)
{
    //addAndMakeVisible(widgets);
    addAndMakeVisible(rotarySlider);
    //addAndMakeVisible(effectGraphComponent);

    setSize(1024, 800);

    juce::MessageManager::callAsync([this]
        {
            effectGraphComponent.resized();
            repaint();
        });

    effectGraphComponent.onEffectPropertyChange = [this]
        {
            rotarySlider.repaint();
            //widgets.repaint();
        };

    startTimer(200);
}

void ContentComponent::resized()
{
    auto r = getLocalBounds();
    auto widgetsBounds = r;
    //widgets.setBounds(widgetsBounds);
    rotarySlider.setBounds(widgetsBounds);
    effectGraphComponent.setBounds(r);
}
