#pragma once

class Direct2DEffect : public juce::ImageEffectFilter
{
public:
    enum class EffectType
    {
        gaussianBlur,
        spotSpecularLighting,
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
    Direct2DEffect(EffectType effectType_);
    ~Direct2DEffect() override;


    void setProperty(int index, PropertyValue&& value);

    void applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha) override;
    void applyEffect(juce::Image& sourceImage, juce::Image& outputImage, float scaleFactor, float alpha);

    EffectType const effectType;

protected:
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
