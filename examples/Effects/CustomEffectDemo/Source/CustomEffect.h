#pragma once

class CustomEffect
{
public:
    CustomEffect();
    ~CustomEffect();

    void applyEffect(juce::Image& outputImage, const juce::AffineTransform& transform, bool clearDestination);

private:
    class Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
