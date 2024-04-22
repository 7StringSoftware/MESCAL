#pragma once

class Direct2DEffect : public juce::ImageEffectFilter
{
public:
    Direct2DEffect();
    ~Direct2DEffect() override;

protected:
    struct Pimpl;

    std::unique_ptr<Pimpl> pimpl;
};
