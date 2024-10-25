#pragma once

class MescalImageEffectFilter : public juce::ImageEffectFilter
{
public:
    MescalImageEffectFilter(mescal::Effect::Ptr effect_);
    ~MescalImageEffectFilter();

    void applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha) override;

protected:
    struct Pimpl;
	std::unique_ptr<Pimpl> pimpl;

    juce::Image outputImage;
};
