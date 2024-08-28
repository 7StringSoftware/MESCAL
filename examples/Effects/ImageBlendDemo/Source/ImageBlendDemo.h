#pragma once

class ImageBlendDemo  : public juce::Component
{
public:
    static constexpr int firstSourceImageID = -1;
    static constexpr int secondSourceImageID = -2;

    ImageBlendDemo()
    {
        //
        // The blend effect only has a single property that sets the mode. Query the effect object for a list
        // of available modes and use that to populate the combo box.
        //
        modeCombo.addItem("First source image", firstSourceImageID);
        modeCombo.addItem("Second source image", secondSourceImageID);
        modeCombo.addSeparator();

        auto propertyInfo = blendEffect->getPropertyInfo(mescal::Effect::Blend::mode);
        for (int index = 0; index < propertyInfo.enumeration.size(); ++index)
        {
            modeCombo.addItem(propertyInfo.enumeration[index], index + 1);
        }

        modeCombo.setSelectedId(mescal::Effect::Blend::overlay + 1, juce::dontSendNotification);

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
        auto id = modeCombo.getSelectedId();

        switch (id)
        {
        case firstSourceImageID:
            g.drawImageAt(sourceImage0, 0, 0);
            break;

        case secondSourceImageID:
            g.drawImageAt(sourceImage1, 0, 0);
            break;

        default:
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
            blendEffect->setPropertyValue(mescal::Effect::Blend::mode, id - 1);
            blendEffect->applyEffect(outputImage, juce::AffineTransform{}, false);

            //
            // Paint the output image
            //
            g.drawImageAt(outputImage, 0, 0);
            break;
        }
        }
    }

    void resized() override
    {
        outputImage = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true };

        modeCombo.setBounds(20, 20, 250, 30);
    }

private:
    juce::Image sourceImage0 = juce::ImageFileFormat::loadFrom(BinaryData::VanGoghstarry_night_jpg, BinaryData::VanGoghstarry_night_jpgSize);
    juce::Image sourceImage1 = juce::ImageFileFormat::loadFrom(BinaryData::M31Kennett_jpg, BinaryData::M31Kennett_jpgSize);
    juce::Image outputImage;
    mescal::Effect::Ptr blendEffect = new mescal::Effect{ mescal::Effect::Type::blend };

    juce::ComboBox modeCombo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageBlendDemo)
};
