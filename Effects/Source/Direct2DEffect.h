#pragma once

class Direct2DEffect : public juce::ImageEffectFilter
{
public:
    enum class EffectType
    {
        gaussianBlur,
        spotDiffuseLighting,
        numEffectTypes
    };

    Direct2DEffect(EffectType effectType_);
    ~Direct2DEffect() override;

    void applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha) override;

    EffectType const effectType;

protected:
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
