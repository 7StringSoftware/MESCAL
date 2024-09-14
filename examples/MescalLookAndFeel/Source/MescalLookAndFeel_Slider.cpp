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
        float trackThickness = thumbRadius * 0.9f;
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
                trackG.setColour(trackColor);
                trackG.fillRoundedRectangle(trackRect, trackThickness * 0.5f);
            }


            auto outerLowerDropShadow = mescal::Effect::create(mescal::Effect::Type::shadow) << trackImage;
            outerLowerDropShadow->setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, 2.0f);
            outerLowerDropShadow->setPropertyValue(mescal::Effect::Shadow::color, mescal::colourToVector4(juce::Colours::white));

            auto outerLowerDropShadowTransform = mescal::Effect::create(mescal::Effect::Type::affineTransform2D) << outerLowerDropShadow;
            outerLowerDropShadowTransform->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, juce::AffineTransform::translation(0.0f, 4.0f));

            float innerShadowSize = trackThickness * 0.2f;
            auto innerDarkShadow = createInnerShadow(trackImage, juce::Colours::black.withAlpha(1.0f), innerShadowSize, juce::AffineTransform::scale(1.1f, 2.0f, (float)trackImage.getWidth() * 0.5f, (float)trackImage.getHeight() * 0.5f).translated(0.0f, innerShadowSize * 2.0f));
            auto innerLightShadow = createInnerShadow(trackImage, juce::Colours::white.withAlpha(1.0f), innerShadowSize, juce::AffineTransform::scale(1.1f, 2.0f, (float)trackImage.getWidth() * 0.5f, (float)trackImage.getHeight() * 0.5f).translated(0.0f, -innerShadowSize * 2.0f));

            auto innerShadowComposite = mescal::Effect::create(mescal::Effect::Type::blend) << innerDarkShadow << innerLightShadow;
            innerShadowComposite->setPropertyValue(mescal::Effect::Blend::mode, mescal::Effect::Blend::linearLight);

            auto innerShadowSourceImageComposite = mescal::Effect::create(mescal::Effect::Type::blend) << trackImage << innerShadowComposite;
            innerShadowSourceImageComposite->setPropertyValue(mescal::Effect::Blend::mode, mescal::Effect::Blend::linearLight);

            auto outerShadowComposite = mescal::Effect::create(mescal::Effect::Type::composite) << innerShadowSourceImageComposite << outerLowerDropShadowTransform;
            outerShadowComposite->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::destinationOver);



#if 0
            auto innerUpperShadow = createInnerShadow(trackImage, trackColor.darker(2.0f), trackThickness * 0.05f,
                juce::AffineTransform::scale(1.0f, 1.0f, (float)trackImage.getWidth() * 0.5f, (float)trackImage.getHeight() * 0.5f).translated(0.0f, trackThickness * 0.05f));
            auto innerUpperBlend = mescal::Effect::Blend::create(mescal::Effect::Blend::multiply) << trackImage << innerUpperShadow;

            auto innerLowerShadow = createInnerShadow(trackImage, trackColor.brighter(2.0f).withAlpha(0.5f), trackThickness * 0.1f,
                juce::AffineTransform::scale(1.0f, 1.0f,
                    (float)trackImage.getWidth() * 0.5f, (float)trackImage.getHeight() * 0.5f)
                    .translated(0.0f, trackThickness * -0.1f));
            auto crop = mescal::Effect::Crop::create(trackRect.removeFromBottom(trackThickness * 0.1f)) << innerLowerShadow;

            auto lowerBlend = mescal::Effect::Blend::create(mescal::Effect::Blend::multiply) << innerUpperBlend << crop;
#endif

            outputImage = juce::Image{ juce::Image::ARGB, trackImage.getWidth(), trackImage.getHeight(), true };
            outerShadowComposite->applyEffect(outputImage, {}, false);
            g.drawImageAt(outputImage, imageOrigin.x, imageOrigin.y);
        }
    }
}
