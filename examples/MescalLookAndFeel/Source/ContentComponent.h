#pragma once

#include <JuceHeader.h>
using namespace juce;
#define PIP_JUCE_EXAMPLES_DIRECTORY_STRING "c:/JUCE/examples"
#include "c:\JUCE\examples\Assets\DemoUtilities.h"
#include "C:\JUCE\examples\GUI\WidgetsDemo.h"
#include "..\..\Effects\EffectGraphDemo\Source\EffectGraphComponent.h"

class ContentComponent : public juce::Component
{
public:
    ContentComponent();

    void resized() override;

    WidgetsDemo widgets;
    EffectGraphComponent effectGraph;
};
