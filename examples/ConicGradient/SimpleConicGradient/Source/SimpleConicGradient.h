#pragma once

class SimpleConicGradient  : public juce::Component
{
public:
    SimpleConicGradient()
    {
		std::array<mescal::ConicGradient::Stop, 5> stops
		{
			mescal::ConicGradient::Stop{ 0.0f, mescal::Color128{ juce::Colours::red }},
			{ juce::MathConstants<float>::halfPi, mescal::Color128{ juce::Colours::orange }},
			{ juce::MathConstants<float>::pi, mescal::Color128{ juce::Colours::yellow }},
			{ juce::MathConstants<float>::halfPi * 3.0f, mescal::Color128{ juce::Colours::green }},
			{ juce::MathConstants<float>::twoPi, mescal::Color128{ juce::Colours::blue }}
		};
		conicGradient.addStops(stops);

        setSize(768, 768);
    }

    void paint(juce::Graphics& g) override
    {
		//
		// Paint the conic gradient
		//
		conicGradient.draw(outputImage, juce::AffineTransform::translation((float)getWidth() * 0.5f, (float)getHeight() * 0.5f));
        
        //
        // Paint the output image
        //
        g.drawImageAt(outputImage, 0, 0);
    }

    void resized() override
    {
        outputImage = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true };
        conicGradient.setRadiusRange({ 0.0f, getWidth() * 0.4f });
    }

private:
	juce::Image outputImage;
	mescal::ConicGradient conicGradient;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleConicGradient)
};
