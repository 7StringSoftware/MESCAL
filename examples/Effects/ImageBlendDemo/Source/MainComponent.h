#pragma once

#include <JuceHeader.h>

class MainComponent  : public juce::Component
{
public:
    MainComponent()
    {
        //
        // The blend effect only has a single property that sets the mode. Query the effect object for a list
        // of available modes and use that to populate the combo box.
        //
        auto propertyInfo = blendEffect->getPropertyInfo(mescal::Effect::Blend::mode);
        for (int index = 0; index < propertyInfo.enumeration.size(); ++index)
        {
            modeCombo.addItem(propertyInfo.enumeration[index], index + 1);
        }

        //
        // Ask the effect for the current mode setting and set the combo box to match
        //
        auto value = blendEffect->getPropertyValue(mescal::Effect::Blend::mode);
        modeCombo.setSelectedItemIndex(std::get<uint8_t>(value), juce::dontSendNotification);

        //
        // Repaint the screen when the combo box changes
        //
        modeCombo.onChange = [this] { repaint(); };
        addAndMakeVisible(modeCombo);

        //
        // Set the size of this component to match the source image
        //
        setSize(sourceImage0.getWidth(), sourceImage0.getHeight());
    }

    void paint(juce::Graphics& g) override
    {
        //
        // Apply the effect: source image 0 -->
        //                                       blend --> output image 
        //                   source image 1 -->
        //
        // Get the current blend mode from the combo box and set it before applying the effect
        //
        blendEffect->setInput(0, sourceImage0);
        blendEffect->setInput(1, sourceImage1);
        blendEffect->setPropertyValue(mescal::Effect::Blend::mode, modeCombo.getSelectedItemIndex());
        blendEffect->applyEffect(outputImage, juce::AffineTransform{}, false);
        
        //
        // Paint the output image
        //
        g.drawImageAt(outputImage, 0, 0);
    }

    void resized() override
    {
        outputImage = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true };

        modeCombo.setBounds(20, 20, 150, 30);
    }

private:
    juce::Image sourceImage0 = juce::ImageFileFormat::loadFrom(BinaryData::VanGoghstarry_night_jpg, BinaryData::VanGoghstarry_night_jpgSize);
    juce::Image sourceImage1 = juce::ImageFileFormat::loadFrom(BinaryData::M31Kennett_jpg, BinaryData::M31Kennett_jpgSize);
    juce::Image outputImage;
    mescal::Effect::Ptr blendEffect = new mescal::Effect{ mescal::Effect::Type::blend };

    juce::ComboBox modeCombo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
