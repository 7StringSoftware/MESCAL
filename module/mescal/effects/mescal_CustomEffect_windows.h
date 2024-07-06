#pragma once

class CustomEffect : public juce::ImageEffectFilter
{
public:
    CustomEffect();
	~CustomEffect() override;

	void applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha) override;
	void applyEffect(juce::Image& sourceImage, juce::Image& outputImage, float scaleFactor, float alpha);

protected:
	struct Pimpl;
	std::unique_ptr<Pimpl> pimpl;
};
