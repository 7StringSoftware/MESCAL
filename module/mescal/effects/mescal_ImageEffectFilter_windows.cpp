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
            auto inputs = effect->getInputs();
            for (int index = 0; index < inputs.size(); ++index)
            {
                auto& input = inputs[index];
                if (std::holds_alternative<mescal::Effect::Ptr>(input))
                {
                    if (auto upstreamEffect = std::get<mescal::Effect::Ptr>(input))
                    {
                        setInputRecursive(upstreamEffect, sourceImage);
                    }
                }
                else if (std::holds_alternative <std::monostate>(input))
                {
                    effect->setInput(index, sourceImage);
                }
            }
        }

        Effect::Ptr effect;
    };

    MescalImageEffectFilter::MescalImageEffectFilter(Effect::Ptr effect_) :
        pimpl(std::make_unique<Pimpl>(effect_))
    {
    }

    MescalImageEffectFilter::~MescalImageEffectFilter()
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
        destContext.setColour(juce::Colours::black.withAlpha(alpha));
        destContext.drawImageAt(outputImage, 0, 0);
    }
}
