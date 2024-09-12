#include "MescalLookAndFeel.h"

using namespace juce;

void MescalLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (slider.isBar())
    {
        g.setColour(slider.findColour(Slider::trackColourId));
        g.fillRect(slider.isHorizontal() ? Rectangle<float>(static_cast<float> (x), (float)y + 0.5f, sliderPos - (float)x, (float)height - 1.0f)
            : Rectangle<float>((float)x + 0.5f, sliderPos, (float)width - 1.0f, (float)y + ((float)height - sliderPos)));

        drawLinearSliderOutline(g, x, y, width, height, style, slider);
    }
    else
    {
        auto isTwoVal = (style == Slider::SliderStyle::TwoValueVertical || style == Slider::SliderStyle::TwoValueHorizontal);
        auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);

        Point<float> startPoint(slider.isHorizontal() ? (float)x : (float)x + (float)width * 0.5f,
            slider.isHorizontal() ? (float)y + (float)height * 0.5f : (float)(height + y));

        Point<float> endPoint(slider.isHorizontal() ? (float)(width + x) : startPoint.x,
            slider.isHorizontal() ? startPoint.y : (float)y);

        //
        // Paint the slider track
        //
        float thumbRadius = (float)getSliderThumbRadius(slider);
        float trackThickness = thumbRadius * 2.0f;
        juce::Image trackImage, outputImage;
        {
            float trackLength = 0.0f;
            juce::Rectangle<float> trackRect;
            juce::Point<float> imageOrigin;

            if (slider.isHorizontal())
            {
                trackLength = endPoint.x - startPoint.x;
                trackRect = trackRect.withWidth(trackLength).withHeight(trackThickness);
                trackImage = juce::Image(Image::PixelFormat::ARGB, (int)trackRect.getWidth() + thumbRadius, thumbRadius * 2, true);
                imageOrigin = startPoint - Point<float>(thumbRadius, thumbRadius);
            }
            else
            {
                trackLength = std::abs(startPoint.y - endPoint.y);
                trackRect = trackRect.withWidth(trackThickness).withHeight(trackLength);
                trackImage = juce::Image(Image::PixelFormat::ARGB, thumbRadius * 2, (int)trackRect.getHeight() + thumbRadius, true);
                imageOrigin = endPoint - Point<float>(thumbRadius, thumbRadius);
            }

            trackRect.setCentre(trackImage.getBounds().toFloat().getCentre());
            auto trackColor = slider.findColour(Slider::trackColourId);

            {
                juce::Graphics trackG{ trackImage };
                //trackG.setColour();
                auto gradient = juce::ColourGradient::vertical(trackColor,
                    0.0f,
                    trackColor.brighter(),
                    trackRect.getHeight());
                trackG.setGradientFill(gradient);
                trackG.fillRoundedRectangle(trackRect, trackThickness * 0.5f);
            }

            auto innerUpperShadow = createInnerShadow(trackImage, trackColor.darker(2.0f), trackThickness * 0.05f,
                juce::AffineTransform::scale(1.0f, 1.0f, (float)trackImage.getWidth() * 0.5f, (float)trackImage.getHeight() * 0.5f).translated(0.0f, trackThickness * 0.05f));
            auto innerUpperBlend = mescal::Effect::Blend::create(mescal::Effect::Blend::multiply) << trackImage << innerUpperShadow;

            auto innerLowerShadow = createInnerShadow(trackImage, trackColor.brighter(2.0f).withAlpha(0.5f), trackThickness * 0.1f,
                juce::AffineTransform::scale(1.0f, 1.0f,
                    (float)trackImage.getWidth() * 0.5f, (float)trackImage.getHeight() * 0.5f)
                    .translated(0.0f, trackThickness * -0.1f));
            auto crop = mescal::Effect::Crop::create(trackRect.removeFromBottom(trackThickness * 0.1f)) << innerLowerShadow;

            auto lowerBlend = mescal::Effect::Blend::create(mescal::Effect::Blend::multiply) << innerUpperBlend << crop;

            outputImage = juce::Image{ juce::Image::ARGB, trackImage.getWidth(), trackImage.getHeight(), true };
            lowerBlend->applyEffect(outputImage, {}, false);
            g.drawImageAt(outputImage, imageOrigin.x, imageOrigin.y);

            g.setColour(trackColor);
            g.drawRoundedRectangle(trackRect + imageOrigin.toFloat(), trackThickness * 0.5f, 1.0f);
        }
    }
}
