/*
  ==============================================================================

    ImageCarousel.cpp
    Created: 13 Aug 2024 8:50:47am
    Author:  Matt Gonzalez

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ImageCarousel.h"

//==============================================================================
ImageCarousel::ImageCarousel()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

ImageCarousel::~ImageCarousel()
{
}

void ImageCarousel::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("ImageCarousel", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void ImageCarousel::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
