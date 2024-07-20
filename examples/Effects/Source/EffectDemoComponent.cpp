#include "EffectDemoComponent.h"

EffectDemoComponent::EffectDemoComponent()
{
    sourceImages.emplace_back(juce::ImageFileFormat::loadFrom(BinaryData::VanGoghstarry_night_jpg, BinaryData::VanGoghstarry_night_jpgSize));

    addAndMakeVisible(propertyPanel);
}

void EffectDemoComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    if (sourceImages.size() == 0)
        return;

    juce::Rectangle<int> imageArea{ 0, 0, propertyPanel.getX(), getHeight() };
    int height = getHeight() / sourceImages.size();
    auto sourceImageArea = imageArea.withWidth(imageArea.getWidth() / 2);
    for (auto& image : sourceImages)
    {
        g.drawImage(image, sourceImageArea.removeFromTop(height).toFloat());
    }
}

void EffectDemoComponent::resized()
{
    propertyPanel.setBounds(getLocalBounds().removeFromRight(250));
}

