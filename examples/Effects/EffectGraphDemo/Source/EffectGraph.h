#pragma once

#include <JuceHeader.h>

class EffectGraph
{
public:
	EffectGraph();

    void paintMetallicKnobImage(float angle);
    void createMetallicKnobEffectGraph();
    void paint3DButtonImages();
    void create3DButtonEffectGraph();

    mescal::Effect::Ptr createInnerShadow(juce::Image const& sourceImage, juce::Colour const& shadowColour, float shadowSize);

    std::vector<juce::Image> sourceImages;
	mescal::Effect::Ptr outputEffect;
};
