#pragma once

class ScatterEffect : public juce::ImageEffectFilter
{
public:
    ScatterEffect();
    ~ScatterEffect() override = default;

    void setScatterDistanceMultiplier(float multiplier);
    void setSize(int width, int height);

    void setParticleSize(int size);
    void applyEffect(juce::Image& sourceImage,
        juce::Graphics& destContext,
        float scaleFactor,
        float alpha) override;

private:
    float scatterMultiplier = 1.0f;
    int particleSize = 8;
    int width = 100, height = 100;
    juce::Image outputImage;
    mescal::SpriteBatch spriteBatch;
};
