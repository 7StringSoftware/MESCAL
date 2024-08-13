/*
  ==============================================================================

    ImageCarousel.h
    Created: 13 Aug 2024 8:50:47am
    Author:  Matt Gonzalez

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class ImageCarousel  : public juce::Component
{
public:
    ImageCarousel();
    ~ImageCarousel() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageCarousel)
};
