#include "MescalLookAndFeel.h"

using namespace juce;

void MescalLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (slider.isBar())
    {
        auto trackRect = Rectangle<int>{ x, y, width, height }.toFloat();

        auto sliderRect = slider.isHorizontal() ? Rectangle<float>(static_cast<float> (x), (float)y + 0.5f, sliderPos - (float)x, (float)height - 1.0f)
            : Rectangle<float>((float)x + 0.5f, sliderPos, (float)width - 1.0f, (float)y + ((float)height - sliderPos));

        juce::Image sliderImage{ juce::Image::ARGB, (int)trackRect.getWidth(), (int)trackRect.getHeight(), true };
        juce::Image trackImage{ juce::Image::ARGB, (int)trackRect.getWidth(), (int)trackRect.getHeight(), true };

        {
            juce::Graphics imageG{ sliderImage };
            imageG.setColour(slider.findColour(Slider::backgroundColourId));
            imageG.fillRect(trackRect);
        }

        {
            juce::Graphics imageG{ trackImage };
            imageG.setColour(slider.findColour(Slider::thumbColourId));
            imageG.fillRect(sliderRect.reduced(2.0f, 2.0f).translated(0.0f, 0.0f));
        }

        auto innerShadowSize = 1.0f;
        auto innerShadow = create3DInnerShadow(sliderImage,
            juce::Colours::black,
            juce::AffineTransform::translation(innerShadowSize, innerShadowSize),
            juce::Colours::white,
            juce::AffineTransform::translation(-innerShadowSize, -innerShadowSize),
            innerShadowSize);

        auto imageWithInnerShadow = mescal::Effect::create(mescal::Effect::Type::composite) << sliderImage << innerShadow;
        imageWithInnerShadow->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);

        juce::Image outputImage{ juce::Image::ARGB, (int)trackRect.getWidth(), (int)trackRect.getHeight(), true };
        imageWithInnerShadow->applyEffect(outputImage, {}, false);
        g.drawImageAt(outputImage, 0, 0);
        g.drawImageAt(trackImage, 0, 0);

        drawLinearSliderOutline(g, x, y, width, height, style, slider);

        return;
    }

    auto thumbColor = slider.findColour(Slider::thumbColourId);
    auto backgroundColor = slider.findColour(Slider::backgroundColourId);
    auto trackColor = slider.findColour(Slider::trackColourId);

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
    juce::Point<float> imageOrigin;

    juce::Image trackImage, outputImage;
    {
        float trackLength = 0.0f;
        juce::Rectangle<float> trackRect, valueRect;

        if (slider.isHorizontal())
        {
            trackLength = endPoint.x - startPoint.x;
            trackRect = trackRect.withWidth(trackLength).withHeight(trackThickness);
            trackImage = juce::Image(Image::PixelFormat::ARGB, (int)trackRect.getWidth() + thumbRadius * 2, thumbRadius * 2, true);

            valueRect = trackRect.withWidth(sliderPos - minSliderPos);

            imageOrigin = startPoint - Point<float>(thumbRadius, thumbRadius);
        }
        else
        {
            trackLength = std::abs(startPoint.y - endPoint.y);
            trackRect = trackRect.withWidth(trackThickness).withHeight(trackLength);
            trackImage = juce::Image(Image::PixelFormat::ARGB, thumbRadius * 2, (int)trackRect.getHeight() + thumbRadius * 2, true);

            imageOrigin = endPoint - Point<float>(thumbRadius, thumbRadius);
        }

        {
            juce::Graphics trackG{ trackImage };
            auto r = trackRect.withCentre(trackImage.getBounds().toFloat().getCentre());

            if (isTwoVal || isThreeVal)
            {
                if (slider.isHorizontal())
                {
                    juce::Path p;
                    p.addRoundedRectangle(r.reduced(1.0f).translated(0.0f, -1.0f), (r.getHeight() - 1.0f) * 0.5f);

                    juce::PathStrokeType stroke{ 1.0f };
                    juce::Path dashedPath;
                    float dashLength = 2.0f;
                    stroke.createDashedStroke(dashedPath, p, &dashLength, 1);
                    g.setColour(backgroundColor.darker());
                    g.fillPath(dashedPath);

                    //
                    // Shrink r to span minSliderPos to maxSliderPos
                    //
                    r = juce::Rectangle<float>::leftTopRightBottom(minSliderPos - imageOrigin.x,
                        r.getY(),
                        maxSliderPos - imageOrigin.x,
                        r.getBottom());
                }
                else
                {
                    juce::Path p;
                    p.addRoundedRectangle(r.reduced(1.0f).translated(0.0f, 0.0f) + imageOrigin, (r.getWidth() - 1.0f) * 0.5f);

                    juce::PathStrokeType stroke{ 1.0f };
                    juce::Path dashedPath;
                    float dashLength = 2.0f;
                    stroke.createDashedStroke(dashedPath, p, &dashLength, 1);
                    g.setColour(backgroundColor.darker());
                    g.fillPath(dashedPath);

                    //
                    // Shrink r to span minSliderPos to maxSliderPos
                    //
                    r = juce::Rectangle<float>::leftTopRightBottom(r.getX(),
                        maxSliderPos - imageOrigin.y,
                        r.getRight(),
                        minSliderPos - imageOrigin.y);
                }
            }

            auto color = isTwoVal ? thumbColor : backgroundColor;
            trackG.setColour(color);
            trackG.fillRoundedRectangle(r, trackThickness * 0.5f);

            if (!isTwoVal)
            {
                trackG.setColour(trackColor);
                trackG.fillRoundedRectangle(valueRect.withPosition(r.getTopLeft()), trackThickness * 0.5f);
            }
        }

        auto outerLowerDropShadow = mescal::Effect::create(mescal::Effect::Type::shadow) << trackImage;
        outerLowerDropShadow->setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, 2.0f);
        outerLowerDropShadow->setPropertyValue(mescal::Effect::Shadow::color, isTwoVal ? juce::Colours::black.withAlpha(0.25f) : juce::Colours::white);

        auto outerLowerDropShadowTransform = mescal::Effect::create(mescal::Effect::Type::affineTransform2D) << outerLowerDropShadow;
        outerLowerDropShadowTransform->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, juce::AffineTransform::translation(2.0f, 2.0f));

        auto topLeftShadowColor = juce::Colours::black.withAlpha(1.0f);
        auto bottomRightShadowColor = juce::Colours::white;

        if (isTwoVal)
        {
            std::swap(topLeftShadowColor, bottomRightShadowColor);
        }

        float innerShadowSize = 2.0f;
        auto innerShadow = create3DInnerShadow(trackImage,
            topLeftShadowColor,
            juce::AffineTransform::scale(1.0f, 1.0f).translated(innerShadowSize * 0.8f, innerShadowSize * 0.8f),
            bottomRightShadowColor,
            juce::AffineTransform::scale(1.1f, 1.0f,
                (float)trackImage.getWidth() * 0.5f,
                (float)trackImage.getHeight() * 0.5f).translated(-innerShadowSize * 0.5f, -innerShadowSize * 1.0f),
            innerShadowSize);

        auto imageWithInnerShadow = mescal::Effect::create(mescal::Effect::Type::composite) << trackImage << innerShadow;
        imageWithInnerShadow->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);

        auto outerShadowComposite = mescal::Effect::create(mescal::Effect::Type::composite) << imageWithInnerShadow << outerLowerDropShadowTransform;
        outerShadowComposite->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::destinationOver);

        outputImage = juce::Image{ juce::Image::ARGB, trackImage.getWidth(), trackImage.getHeight(), true };
        outerShadowComposite->applyEffect(outputImage, {}, false);
        g.drawImageAt(outputImage, imageOrigin.x, imageOrigin.y);
    }

    //
    // Paint the thumb
    //
    if (!isTwoVal)
    {
        g.setColour(thumbColor);
        juce::Rectangle<float> thumbRect{ thumbRadius * 1.5f, thumbRadius * 1.5f };
        if (slider.isHorizontal())
        {
            thumbRect.reduce(1.0f, 1.0f);
            thumbRect.setCentre(startPoint.withX(sliderPos) - imageOrigin);
        }
        else
        {
            thumbRect.setCentre(startPoint.withY(sliderPos) - imageOrigin);
        }

        {
            juce::Graphics trackG{ trackImage };
            trackG.setColour(juce::Colours::transparentBlack);
            trackG.getInternalContext().fillRect(trackImage.getBounds(), true);

            trackG.setGradientFill(juce::ColourGradient
                {
                    thumbColor,
                    thumbRect.getCentre(),
                    thumbColor.darker(),
                    { 0.0f, 0.0f },
                    true
                });

            trackG.fillEllipse(thumbRect);
        }

        {
            auto outerLowerDropShadow = mescal::Effect::create(mescal::Effect::Type::shadow) << trackImage;
            outerLowerDropShadow->setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, thumbRadius * 0.05f);
            outerLowerDropShadow->setPropertyValue(mescal::Effect::Shadow::color, juce::Colours::black.withAlpha(0.5f));

            auto outerLowerDropShadowTransform = mescal::Effect::create(mescal::Effect::Type::affineTransform2D) << outerLowerDropShadow;
            outerLowerDropShadowTransform->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, juce::AffineTransform::translation(thumbRadius * 0.1f, thumbRadius * 0.1f));

            auto innerShadow = create3DInnerShadow(trackImage,
                juce::Colours::white.withAlpha(0.5f),
                juce::AffineTransform::translation(thumbRadius * 0.5f, thumbRadius * 0.55f),
                juce::Colours::black.withAlpha(0.125f),
                juce::AffineTransform::translation(thumbRadius * -0.125f, thumbRadius * -0.125f),
                thumbRadius * 0.25f);

            auto imageWithInnerShadow = mescal::Effect::create(mescal::Effect::Type::composite) << trackImage << innerShadow;
            imageWithInnerShadow->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);

            outerLowerDropShadowTransform->applyEffect(outputImage, {}, true);
            imageWithInnerShadow->applyEffect(outputImage, {}, false);
        }

        g.setColour(juce::Colours::black.withAlpha(0.25f));
        g.fillEllipse(thumbRect.expanded(1.0f) + imageOrigin);
        g.setColour(juce::Colours::black);
        g.drawImageAt(outputImage, imageOrigin.x, imageOrigin.y);
    }
}

void MescalLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    auto outline = slider.findColour(Slider::rotarySliderOutlineColourId);
    auto fill = slider.findColour(Slider::rotarySliderFillColourId);

    auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(10);

    auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = jmin(8.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;

    juce::Image trackImage{ juce::Image::ARGB, width, height, true };
    juce::Image outputImage{ juce::Image::ARGB, width, height, true };
    auto backgroundColor = slider.findColour(Slider::backgroundColourId);
    Path backgroundArc;

    {
        juce::Graphics imageG{ trackImage };
        backgroundArc.addCentredArc(bounds.getCentreX(),
            bounds.getCentreY(),
            arcRadius,
            arcRadius,
            0.0f,
            rotaryStartAngle,
            rotaryEndAngle,
            true);

        auto center = bounds.getCentre();
        auto midpoint = center.getPointOnCircumference(arcRadius - lineW * 0.5f, rotaryEndAngle);
        backgroundArc.addCentredArc(midpoint.x,
            midpoint.y,
            lineW * 0.5f,
            lineW * 0.5f,
            rotaryEndAngle,
            0.0f,
            juce::MathConstants<float>::pi,
            false);

        backgroundArc.addCentredArc(bounds.getCentreX(),
            bounds.getCentreY(),
            arcRadius - lineW,
            arcRadius - lineW,
            0.0f,
            rotaryEndAngle,
            rotaryStartAngle,
            false);

        midpoint = center.getPointOnCircumference(arcRadius - lineW * 0.5f, rotaryStartAngle);
        backgroundArc.addCentredArc(midpoint.x,
            midpoint.y,
            lineW * 0.5f,
            lineW * 0.5f,
            rotaryStartAngle + juce::MathConstants<float>::pi,
            0.0f,
            juce::MathConstants<float>::pi,
            false);

        imageG.setColour(backgroundColor);
        imageG.fillPath(backgroundArc);
    }

    auto outerLowerDropShadow = mescal::Effect::create(mescal::Effect::Type::shadow) << trackImage;
    outerLowerDropShadow->setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, 2.0f);
    outerLowerDropShadow->setPropertyValue(mescal::Effect::Shadow::color, juce::Colours::white);

    auto outerLowerDropShadowTransform = mescal::Effect::create(mescal::Effect::Type::affineTransform2D) << outerLowerDropShadow;
    outerLowerDropShadowTransform->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, juce::AffineTransform::translation(4.0f, 4.0f));

    float innerShadowSize = arcRadius * 0.1f;
#if 0
    auto innerDarkShadow = createInnerShadow(trackImage,
        juce::Colours::black.withAlpha(1.0f),
        innerShadowSize,
        juce::AffineTransform::scale(1.0f, 1.0f).translated(innerShadowSize * 0.5f, innerShadowSize * 0.5f));
    auto innerLightShadow = createInnerShadow(trackImage,
        juce::Colours::white.withAlpha(1.0f),
        innerShadowSize,
        juce::AffineTransform::scale(1.0f, 1.0f,
            (float)trackImage.getWidth() * 0.5f,
            (float)trackImage.getHeight() * 0.5f).translated(-innerShadowSize * 0.5f, -innerShadowSize * 0.25f));

    auto innerShadowBlend = mescal::Effect::create(mescal::Effect::Type::blend) << innerDarkShadow << innerLightShadow;
    innerShadowBlend->setPropertyValue(mescal::Effect::Blend::mode, mescal::Effect::Blend::multiply);
#else
    auto innerShadowBlend = create3DInnerShadow(trackImage,
        juce::Colours::black,
        juce::AffineTransform::scale(1.0f, 1.0f).translated(innerShadowSize * 0.5f, innerShadowSize * 0.5f),
        juce::Colours::white,
        juce::AffineTransform::scale(1.1f, 1.0f,
            (float)trackImage.getWidth() * 0.5f,
            (float)trackImage.getHeight() * 0.5f).translated(-innerShadowSize * 0.5f, -innerShadowSize * 0.25f),
        innerShadowSize);
#endif

    outputImage = juce::Image{ juce::Image::ARGB, trackImage.getWidth(), trackImage.getHeight(), true };
    innerShadowBlend->applyEffect(outputImage, {}, false);

    {
        juce::Graphics::ScopedSaveState save{ g };
        g.setColour(backgroundColor);
        g.fillPath(backgroundArc);

        g.reduceClipRegion(backgroundArc);
        g.drawImageAt(outputImage, 0, 0);
    }

#if 0
    if (slider.isEnabled())
    {
        Path valueArc;
        valueArc.addCentredArc(bounds.getCentreX(),
            bounds.getCentreY(),
            arcRadius,
            arcRadius,
            0.0f,
            rotaryStartAngle,
            toAngle,
            true);

        g.setColour(fill);
        g.strokePath(valueArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));
    }

    auto thumbWidth = lineW * 2.0f;
    Point<float> thumbPoint(bounds.getCentreX() + arcRadius * std::cos(toAngle - MathConstants<float>::halfPi),
        bounds.getCentreY() + arcRadius * std::sin(toAngle - MathConstants<float>::halfPi));

    g.setColour(slider.findColour(Slider::thumbColourId));
    g.fillEllipse(Rectangle<float>(thumbWidth, thumbWidth).withCentre(thumbPoint));
#endif

    //
    // Paint the thumb
    //
    auto thumbColor = slider.findColour(Slider::thumbColourId);
    auto thumbRadius = lineW;

    Point<float> thumbPoint(bounds.getCentreX() + (arcRadius - lineW * 0.5f) * std::cos(toAngle - MathConstants<float>::halfPi),
        bounds.getCentreY() + (arcRadius - lineW * 0.5f) * std::sin(toAngle - MathConstants<float>::halfPi));

    juce::Rectangle<float> thumbRect{ thumbRadius * 2.0f, thumbRadius * 2.0f };
    thumbRect.setCentre(thumbPoint);

    {
        juce::Graphics trackG{ trackImage };
        trackG.setColour(juce::Colours::transparentBlack);
        trackG.getInternalContext().fillRect(trackImage.getBounds(), true);

        trackG.setGradientFill(juce::ColourGradient
            {
                thumbColor,
                thumbRect.getCentre(),
                thumbColor.darker(),
                { 0.0f, 0.0f },
                true
            });

        trackG.fillEllipse(thumbRect);
    }

    {
        auto outerLowerDropShadow = mescal::Effect::create(mescal::Effect::Type::shadow) << trackImage;
        outerLowerDropShadow->setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, thumbRadius * 0.05f);
        outerLowerDropShadow->setPropertyValue(mescal::Effect::Shadow::color, juce::Colours::black.withAlpha(0.5f));

        auto outerLowerDropShadowTransform = mescal::Effect::create(mescal::Effect::Type::affineTransform2D) << outerLowerDropShadow;
        outerLowerDropShadowTransform->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, juce::AffineTransform::translation(thumbRadius * 0.1f, thumbRadius * 0.1f));

        auto upperInnerShadow = createInnerShadow(trackImage,
            //thumbColor.brighter(),
            juce::Colours::white.withAlpha(0.5f),
            thumbRadius * 0.25f,
            juce::AffineTransform::translation(thumbRadius * 0.5f, thumbRadius * 0.5f));

        auto imageWithUpperShadow = mescal::Effect::create(mescal::Effect::Type::composite) << trackImage << upperInnerShadow;
        imageWithUpperShadow->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);

        //             auto buttonComposite = mescal::Effect::create(mescal::Effect::Type::composite) << outerLowerDropShadowTransform << imageWithUpperShadow;
        //             buttonComposite->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);

                    //auto buttonBlend = mescal::Effect::Blend::create(mescal::Effect::Blend::overlay) << imageWithUpperShadow << outerLowerDropShadowTransform;

                    //buttonBlend->applyEffect(outputImage, {}, true);
        outerLowerDropShadowTransform->applyEffect(outputImage, {}, true);
        imageWithUpperShadow->applyEffect(outputImage, {}, false);
    }

    g.setColour(juce::Colours::black.withAlpha(0.25f));
    g.fillEllipse(thumbRect.expanded(1.0f));
    g.setColour(juce::Colours::black);
    g.drawImageAt(outputImage, 0, 0);
}
