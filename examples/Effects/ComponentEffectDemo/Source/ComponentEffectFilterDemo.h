#pragma once

class ImageEffectFilterDemo  : public juce::Component
{
public:
    ImageEffectFilterDemo()
    {
        addAndMakeVisible(blurAmountSlider);
        blurAmountSlider.setRange({ 0.0, 50.0 }, 0.1);
        blurAmountSlider.setValue(3.0, juce::dontSendNotification);
        blurAmountSlider.onValueChange = [this] 
            { 
                blurEffect->setPropertyValue(mescal::Effect::GaussianBlur::standardDeviation, (float)blurAmountSlider.getValue());
                repaint(); 
            };

        setComponentEffect(effect.get());

        setSize(image.getWidth(), image.getHeight());
    }

    void paint(juce::Graphics& g) override
    {
        //
        // Paint the image
        //
        g.drawImageAt(image, 0, 0);

        //
        // Paint a rectangle behind the slider
        //
        g.setColour(juce::Colour{ 0xff222222 });
        g.fillRect(blurAmountSlider.getBounds().expanded(4, 0));
    }

    void resized() override
    {
        blurAmountSlider.setBounds(20, getHeight() - 50, 300, 30);
    }

private:
    juce::Image image = juce::ImageFileFormat::loadFrom(BinaryData::VanGoghstarry_night_jpg, BinaryData::VanGoghstarry_night_jpgSize);
    mescal::Effect::Ptr blurEffect = new mescal::Effect{ mescal::Effect::Type::gaussianBlur };
    std::unique_ptr<mescal::MescalImageEffectFilter> effect = std::make_unique<mescal::MescalImageEffectFilter>(blurEffect);
    
    juce::Slider blurAmountSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxLeft };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageEffectFilterDemo)
};
