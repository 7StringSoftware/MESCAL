#pragma once

class Direct2DEffect : public juce::ImageEffectFilter
{
public:
    Direct2DEffect();
    ~Direct2DEffect() override;

    void applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha) override;

protected:
    struct Pimpl;
    virtual Pimpl* getPimpl() const noexcept = 0;
};
