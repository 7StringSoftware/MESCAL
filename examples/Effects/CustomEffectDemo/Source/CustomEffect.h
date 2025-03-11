#pragma once

class CustomEffect : public juce::ComBaseClassHelper<ID2D1EffectImpl>
{
public:

private:
    class Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
