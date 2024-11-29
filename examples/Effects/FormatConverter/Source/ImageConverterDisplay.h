#pragma once

#include "FormatConverter.h"

struct ImageConverterDisplay : public juce::Component
{
    ImageConverterDisplay()
    {
        setSize(800, 600);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillCheckerBoard(getLocalBounds().toFloat(), 100.0f, 100.0f, juce::Colours::lightgrey, juce::Colours::darkgrey);

        int x = 0, y = 0;
        for (auto const& image : images)
        {
            g.drawImageAt(image, x, y);

            g.setColour(juce::Colours::black);
            g.drawText(image.getProperties()->getWithDefault("Format", {}).toString(), x, y, image.getWidth(), image.getHeight(), juce::Justification::centred);
            y += image.getHeight();
            if (y >= getHeight())
            {
                y = 0;
                x += image.getWidth();
            }
        }
    }

    void resized() override
    {
        softwareSourceImage = juce::Image{ juce::Image::ARGB, getWidth() / 3, getHeight() / 3, true, juce::NativeImageType{} };

        {
            juce::Graphics g{ softwareSourceImage };
            auto gradient = juce::ColourGradient{ juce::Colours::orange, softwareSourceImage.getBounds().toFloat().getCentre(), juce::Colours::cyan.withAlpha(0.5f), { 0.0f, (float)softwareSourceImage.getHeight() * 0.5f}, true};
            g.setGradientFill(gradient);
            g.fillEllipse(softwareSourceImage.getBounds().toFloat());
        }

        std::array<juce::Image::PixelFormat, 3> constexpr formats{ juce::Image::SingleChannel, juce::Image::RGB, juce::Image::ARGB };
        juce::StringArray const formatNames{ "Unknown", "RGB", "ARGB", "A" };
        std::array<juce::Image, 3> sourceImages;

        {
            auto sourceImageIt = sourceImages.begin();
            for (auto const sourceFormat : formats)
            {
                *sourceImageIt = softwareSourceImage.convertedToFormat(sourceFormat);
                ++sourceImageIt;
            }
        }

        images.clear();

        auto sourceImageIt = sourceImages.begin();
        for (auto const sourceFormat : formats)
        {
            for (auto const destFormat : formats)
            {
                //auto image = sourceImageIt->convertedToFormat(destFormat);
                auto image = converter.convert(*sourceImageIt, destFormat);
                if (image.isValid())
                {
                    image.getProperties()->set("Format", formatNames[(int)sourceFormat] + "->" + formatNames[(int)destFormat]);
                }
                else
                {
                    image = juce::Image{ juce::Image::ARGB, sourceImageIt->getWidth(), sourceImageIt->getHeight(), true };
                }

                images.push_back(image);
            }

            sourceImageIt++;
        }
    }

    FormatConverter converter;
    juce::Image softwareSourceImage;
    std::vector<juce::Image> images;

};
