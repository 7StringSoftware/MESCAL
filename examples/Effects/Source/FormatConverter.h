#pragma once

#include <JuceHeader.h>

class FormatConverter
{
public:
	FormatConverter();
	~FormatConverter();

	juce::Image argb{ juce::Image::ARGB, 16, 16, true };
	juce::Image singleChannel{ juce::Image::SingleChannel, 16, 16, true };

	void convert(const juce::Image& source, juce::Image& destination);

    void print(juce::Image const& image)
        {
            juce::Image::BitmapData bitmapData{ image, juce::Image::BitmapData::readOnly };
            for (int y = 0; y < image.getHeight(); ++y)
            {
                juce::String line;
                for (int x = 0; x < image.getWidth(); ++x)
                {
                    line << bitmapData.getPixelColour(x, y).toString().paddedLeft('0', 8) << " ";
                }

                DBG(line);
            }
        }

	struct Pimpl;
	std::unique_ptr<Pimpl> pimpl;
};