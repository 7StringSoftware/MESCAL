#pragma once

#include <JuceHeader.h>
using namespace juce;
#define PIP_JUCE_EXAMPLES_DIRECTORY_STRING "c:/JUCE-fork/examples"
#include "c:\JUCE-fork\examples\Assets\DemoUtilities.h"
#include "C:\JUCE-fork\examples\GUI\WidgetsDemo.h"
#include "..\..\Effects\EffectGraphDemo\Source\EffectGraphComponent.h"

class ContentComponent : public juce::Component, public juce::Timer
{
public:
    ContentComponent(mescal::Effect::Ptr effectIn);

    void resized() override;

    mescal::Effect::Ptr effect;
    WidgetsDemo widgets;
    juce::Slider rotarySlider{ juce::Slider::Rotary, juce::Slider::NoTextBox };
    EffectGraphComponent effectGraphComponent;

    void timerCallback()  override
    {
    }

    juce::VBlankAttachment vblank{ this, [this] { rotarySlider.repaint(); } };
};
