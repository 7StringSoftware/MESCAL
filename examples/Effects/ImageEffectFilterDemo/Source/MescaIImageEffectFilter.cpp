#include "MescaIImageEffectFilter.h"

void MescalImageEffectFilter::applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha)
{
    if (!effectGraph)
    {
        jassertfalse;
        return;
    }

    if (outputImage.isNull() || outputImage.getWidth() < sourceImage.getWidth() || outputImage.getHeight() < sourceImage.getHeight())
    {
        outputImage = juce::Image{ juce::Image::ARGB, sourceImage.getWidth(), sourceImage.getHeight(), true, juce::NativeImageType{} };
    }

    auto inputs = effectGraph->getGraphImageInputs();
    for (auto& input : inputs)
    {
        input.effect->setInput(input.inputIndex, sourceImage);
    }
    effectGraph->applyEffect(outputImage, {}, true);

    destContext.drawImage(outputImage,
        0, 0, sourceImage.getWidth(), sourceImage.getHeight(),
        0, 0, sourceImage.getWidth(), sourceImage.getHeight());
}
