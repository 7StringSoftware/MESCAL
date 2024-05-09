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

    struct GaussianBlur
    {
        enum Property
        {
            blurAmount,
            borderMode,
            optimization,
            numProperties
        };
    };

    using PropertyValue = std::variant<float, juce::Colour, juce::Point<float>, juce::Rectangle<float>, juce::AffineTransform>;

    static std::unique_ptr<Direct2DEffect> create(EffectType effectType);
    ~Direct2DEffect() override;



    void setProperty(PropertyIndex index, PropertyValue&& value);

    void applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha) override;

    EffectType const effectType;

protected:
    Direct2DEffect(EffectType effectType_);
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
