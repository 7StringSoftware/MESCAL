#pragma once

#include <JuceHeader.h>

class EffectGraph
{
public:
	EffectGraph();

	juce::Image sourceImage{ juce::Image::ARGB, 1000, 1000, true, juce::NativeImageType{} };
	mescal::Effect::Ptr outputEffect;
};
