#pragma once

#include <JuceHeader.h>

class EffectGraph
{
public:
	EffectGraph();

	juce::Image sourceImage{ juce::Image::ARGB, 1000, 1000, true, juce::NativeImageType{} };
	mescal::Effect::Ptr outputEffect;

    void createMetallicKnobGraph();
    void createSliderGraph();

    mescal::Effect::Ptr addShadow(juce::Image const& sourceImage, juce::Colour const& shadowColor, float shadowSize, juce::AffineTransform transform);
    mescal::Effect::Ptr createInnerShadow(juce::Image const& sourceImage, juce::Colour const& shadowColor, float shadowSize, juce::AffineTransform transform);
};
