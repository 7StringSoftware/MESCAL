#pragma once

#include <JuceHeader.h>

class FormatConverter
{
public:
	FormatConverter();
	~FormatConverter();

    juce::Image convert(const juce::Image& source, juce::Image::PixelFormat outputFormat);

    void print(juce::Image const& image);

private:
    juce::Image convertToSingleChannel(const juce::Image& source);
    juce::Image singleChannelToARGB(const juce::Image& source);

	struct Pimpl;
	std::unique_ptr<Pimpl> pimpl;
};