#include "mescal_Effects_windows.h"

namespace mescal
{
    struct MescalImageEffectFilter::Pimpl
    {
        Pimpl(Effect::Ptr effect_) :
            effect(effect_)
        {
        }

        static void setInputRecursive(Effect::Ptr effect, juce::Image& sourceImage)
        {
            for (auto it = effect->getInputs().begin() + 1; it != effect->getInputs().end(); ++it)
            {
                if (std::holds_alternative<mescal::Effect::Ptr>(*it))
                {
                    if (auto upstreamEffect = std::get<mescal::Effect::Ptr>(*it))
                    {
                        setInputRecursive(upstreamEffect, sourceImage);
                    }
                }
            }

            effect->setInput(0, sourceImage);
        }

        Effect::Ptr effect;
    };

    MescalImageEffectFilter::MescalImageEffectFilter(Effect::Ptr effect_) :
        pimpl(std::make_unique<Pimpl>(effect_))
    {
    }

    void MescalImageEffectFilter::applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha)
    {
        if (outputImage.isNull() || outputImage.getWidth() != sourceImage.getWidth() || outputImage.getHeight() != sourceImage.getHeight())
        {
            outputImage = juce::Image(juce::Image::ARGB, sourceImage.getWidth(), sourceImage.getHeight(), true, juce::NativeImageType{});
        }

        Pimpl::setInputRecursive(pimpl->effect, sourceImage);
        pimpl->effect->applyEffect(outputImage, juce::AffineTransform::scale(scaleFactor), true);
        destContext.setColour(juce::Colours::black);
        destContext.drawImageAt(outputImage, 0, 0);
    }
}
