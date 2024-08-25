#pragma once

class SimpleBlurDemo  : public juce::Component
{
public:
    SimpleBlurDemo()
    {
        addAndMakeVisible(blurAmountSlider);
        blurAmountSlider.setRange({ 0.0, 50.0 }, 0.1);
        blurAmountSlider.setValue(3.0, juce::dontSendNotification);
        blurAmountSlider.onValueChange = [this] { repaint(); };

        setSize(sourceImage.getWidth(), sourceImage.getHeight());
    }

    void paint(juce::Graphics& g) override
    {
        //
        // Apply the effect: source image -> blur -> output image
        //
        blurEffect->setInput(0, sourceImage);
        blurEffect->setPropertyValue(mescal::Effect::GaussianBlur::standardDeviation, (float)blurAmountSlider.getValue());
        blurEffect->applyEffect(outputImage, juce::AffineTransform{}, false);
        
        //
        // Paint the output image
        //
        g.drawImageAt(outputImage, 0, 0);

        //
        // Paint a rectangle behind the slider
        //
        g.setColour(juce::Colour{ 0xff222222 });
        g.fillRect(blurAmountSlider.getBounds().expanded(4, 0));
    }

    void resized() override
    {
        outputImage = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true };

        blurAmountSlider.setBounds(20, getHeight() - 50, 300, 30);
    }

private:
    juce::Image sourceImage = juce::ImageFileFormat::loadFrom(BinaryData::VanGoghstarry_night_jpg, BinaryData::VanGoghstarry_night_jpgSize);
    juce::Image outputImage;
    mescal::Effect::Ptr blurEffect = new mescal::Effect{ mescal::Effect::Type::gaussianBlur };
    
    juce::Slider blurAmountSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxLeft };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleBlurDemo)
};
