#include "MescalLookAndFeel.h"

using namespace juce;

#if 0
void MescalLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle style, juce::Slider& slider)
{
    auto thumbColor = slider.findColour(Slider::thumbColourId);
    auto backgroundColor = slider.findColour(Slider::backgroundColourId);
    auto trackColor = slider.findColour(Slider::trackColourId);

    auto isTwoVal = (style == Slider::SliderStyle::TwoValueVertical || style == Slider::SliderStyle::TwoValueHorizontal);
    auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);

    Point<float> startPoint(slider.isHorizontal() ? (float)x : (float)x + (float)width * 0.5f,
        slider.isHorizontal() ? (float)y + (float)height * 0.5f : (float)(height + y));

    Point<float> endPoint(slider.isHorizontal() ? (float)(width + x) : startPoint.x,
        slider.isHorizontal() ? startPoint.y : (float)y);

    float thumbRadius = (float)getSliderThumbRadius(slider);
    float trackThickness = thumbRadius * 0.9f;

    if (slider.isBar())
    {
#if 0
        juce::Image sliderImage{ juce::Image::ARGB, (int)sliderRect.getWidth(), (int)sliderRect.getHeight(), true };
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
        innerShadow.configure(sliderImage,
            juce::Colours::black,
            juce::AffineTransform::translation(innerShadowSize, innerShadowSize),
            juce::Colours::white,
            juce::AffineTransform::translation(-innerShadowSize, -innerShadowSize),
            innerShadowSize);

        auto imageWithInnerShadow = mescal::Effect::create(mescal::Effect::Type::composite) << sliderImage << innerShadow.getEffect();
        imageWithInnerShadow->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);

        juce::Image outputImage{ juce::Image::ARGB, (int)trackRect.getWidth(), (int)trackRect.getHeight(), true };
        imageWithInnerShadow->applyEffect(outputImage, {}, false);
        g.drawImageAt(outputImage, 0, 0);
        g.drawImageAt(trackImage, 0, 0);

        drawLinearSliderOutline(g, x, y, width, height, style, slider);
#endif

        return;
    }

    //
    // Paint the slider track
    //
    auto trackRect = juce::Rectangle<float>{ startPoint, endPoint };
    juce::Rectangle<float> thumbRect{ thumbRadius * 2.5f, thumbRadius * 2.5f };

    if (slider.isHorizontal())
    {
        trackRect = trackRect.withSizeKeepingCentre(trackRect.getWidth() + trackThickness, trackThickness);
        thumbRect.setPosition(sliderPos - thumbRadius, trackRect.getCentreY() - thumbRect.getHeight() * 0.5f);
    }
    else
    {
        trackRect = trackRect.withSizeKeepingCentre(trackThickness, trackRect.getHeight() + trackThickness);
        thumbRect.setPosition(trackRect.getCentreX() - thumbRect.getWidth() * 0.5f, sliderPos - thumbRadius);
    }

    auto& trackImage = getImage(0, trackRect);
    auto outputImage = getImage(2, trackRect);

    {
        juce::Graphics trackG{ trackImage };
        clear(trackG);

        auto r = trackRect.withZeroOrigin();

        if (isTwoVal || isThreeVal)
        {
            juce::Path p;

            if (slider.isHorizontal())
            {
                auto dashedR = trackRect.reduced(1.0f).translated(0.0f, -1.0f);
                p.addRoundedRectangle(dashedR, dashedR.getHeight() * 0.5f);

                r = r.withWidth(maxSliderPos - minSliderPos).withPosition(minSliderPos - startPoint.x, r.getY() - 2.0f);
            }
            else
            {
                auto dashedR = trackRect.reduced(1.0f).translated(-1.0f, 0.0f);
                p.addRoundedRectangle(dashedR, dashedR.getWidth() * 0.5f);

                r = r.withHeight(minSliderPos - maxSliderPos).withY(maxSliderPos);

            }

            juce::PathStrokeType stroke{ 1.0f };
            juce::Path dashedPath;
            float dashLength = 2.0f;
            stroke.createDashedStroke(dashedPath, p, &dashLength, 1);
            g.setColour(backgroundColor.darker());
            g.fillPath(dashedPath);
        }

        auto color = isTwoVal ? thumbColor : backgroundColor;
        trackG.setColour(color);
        trackG.fillRoundedRectangle(r, trackThickness * 0.5f);

//         if (!isTwoVal)
//         {
//             trackG.setColour(trackColor);
//             trackG.fillRoundedRectangle(valueRect.withPosition(r.getTopLeft()), trackThickness * 0.5f);
//         }
    }

    {
        auto topLeftShadowColor = juce::Colours::black.withAlpha(1.0f);
        auto bottomRightShadowColor = juce::Colours::white;

        if (isTwoVal)
        {
            std::swap(topLeftShadowColor, bottomRightShadowColor);
        }

        float innerShadowSize = 2.0f;
        innerShadow.configure(trackImage,
            topLeftShadowColor,
            juce::AffineTransform::scale(1.0f, 1.0f).translated(innerShadowSize * 0.8f, innerShadowSize * 0.8f),
            bottomRightShadowColor,
            juce::AffineTransform::scale(1.1f, 1.0f,
                (float)trackImage.getWidth() * 0.5f,
                (float)trackImage.getHeight() * 0.5f).translated(-innerShadowSize * 0.5f, -innerShadowSize * 1.0f),
            innerShadowSize);

        auto imageWithInnerShadow = mescal::Effect::create(mescal::Effect::Type::composite) << trackImage << innerShadow.getEffect();
        imageWithInnerShadow->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);

        imageWithInnerShadow->applyEffect(outputImage, {}, true);

    }

    g.drawImageAt(outputImage, trackRect.getX(), trackRect.getY());

    //
    // Paint the thumb
    //
    if (!isTwoVal)
    {
        g.setColour(thumbColor);

        auto thumbImage = getImage(1, thumbRect);
        auto thumbOutput = getImage(3, thumbRect);

        {
            juce::Graphics thumbG{ thumbImage };
            clear(thumbG);

            auto ellipseR = thumbImage.getBounds().toFloat().withSizeKeepingCentre(thumbRadius * 2.0f, thumbRadius * 2.0f);

            thumbG.setGradientFill(juce::ColourGradient
                {
                    thumbColor,
                    ellipseR.getCentre(),
                    thumbColor.darker(),
                    { 0.0f, 0.0f },
                    true
                });

            thumbG.fillEllipse(ellipseR);
        }

        {
            dropShadow.configure(thumbImage,
                juce::Colours::black.withAlpha(0.5f),
                thumbRadius * 0.05f,
                juce::AffineTransform::translation(thumbRadius * 0.1f, thumbRadius * 0.1f));

            innerShadow.configure(thumbImage,
                juce::Colours::white.withAlpha(0.5f),
                juce::AffineTransform::translation(thumbRadius * 0.5f, thumbRadius * 0.55f),
                juce::Colours::black.withAlpha(0.125f),
                juce::AffineTransform::translation(thumbRadius * -0.125f, thumbRadius * -0.125f),
                thumbRadius * 0.25f);

            auto imageWithInnerShadow = mescal::Effect::create(mescal::Effect::Type::composite) << thumbImage << innerShadow.getEffect();
            imageWithInnerShadow->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);

            dropShadow.getEffect()->applyEffect(thumbOutput, {}, true);
            imageWithInnerShadow->applyEffect(thumbOutput, {}, false);
        }
        g.drawImage(thumbOutput, thumbRect - juce::Point<float>{ 2.0f, 0.0f }, juce::RectanglePlacement::doNotResize);


#if 0
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
            dropShadow.configure(trackImage,
                juce::Colours::black.withAlpha(0.5f),
                thumbRadius * 0.05f,
                juce::AffineTransform::translation(thumbRadius * 0.1f, thumbRadius * 0.1f));

            innerShadow.configure(trackImage,
                juce::Colours::white.withAlpha(0.5f),
                juce::AffineTransform::translation(thumbRadius * 0.5f, thumbRadius * 0.55f),
                juce::Colours::black.withAlpha(0.125f),
                juce::AffineTransform::translation(thumbRadius * -0.125f, thumbRadius * -0.125f),
                thumbRadius * 0.25f);

            auto imageWithInnerShadow = mescal::Effect::create(mescal::Effect::Type::composite) << trackImage << innerShadow.getEffect();
            imageWithInnerShadow->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);

            dropShadow.getEffect()->applyEffect(outputImage, {}, true);
            imageWithInnerShadow->applyEffect(outputImage, {}, false);
        }

        g.setColour(juce::Colours::black.withAlpha(0.25f));
        g.fillEllipse(thumbRect.expanded(1.0f) + imageOrigin);
        g.setColour(juce::Colours::black);
        g.drawImage(outputImage, outputImage.getBounds().toFloat().withPosition(imageOrigin), juce::RectanglePlacement::doNotResize);
#endif
    }
}
#endif

void MescalLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    //LookAndFeel_V4::drawRotarySlider(g, x, y, width, height, sliderPosProportional, rotaryStartAngle, rotaryEndAngle, slider);
    drawRotarySliderConicGradient(g, x, y, width, height, sliderPosProportional, rotaryStartAngle, rotaryEndAngle, slider);
    return;

    auto outline = slider.findColour(Slider::rotarySliderOutlineColourId);
    auto fill = slider.findColour(Slider::rotarySliderFillColourId);

    auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(10);

    auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = jmin(8.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;

    drawRotarySliderConicGradient(g, x, y, width, height, sliderPosProportional, rotaryStartAngle, rotaryEndAngle, slider);
    return;

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

#if 0
    {
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
    }
#endif

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
                thumbColor.darker(0.2f),
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

        auto imageWithUpperShadow = mescal::Effect::create(mescal::Effect::Type::composite) << trackImage << innerShadow.getEffect();
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

void MescalLookAndFeel::drawRotarySliderConicGradient(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    auto outline = slider.findColour(Slider::rotarySliderOutlineColourId);
    auto fill = slider.findColour(Slider::rotarySliderFillColourId);

    auto bounds = Rectangle<int>(x, y, width, height).toFloat();

    auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = jmin(32.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;

    //
    // Draw the conic gradient
    //
    juce::Image sourceImage{ juce::Image::ARGB, slider.getWidth(), slider.getHeight(), true};
    juce::Image trackShadowImage{ juce::Image::ARGB, slider.getWidth(), slider.getHeight(), true };
    juce::Image conicGradientArcImage{ juce::Image::ARGB, slider.getWidth(), slider.getHeight(), true };
    juce::Image glowImage{ juce::Image::ARGB, slider.getWidth(), slider.getHeight(), true };
    juce::Image outputImage{ juce::Image::ARGB, slider.getWidth(), slider.getHeight(), true };
    juce::Range<float> radiusRange{ arcRadius - lineW * 0.4f, arcRadius + lineW * 0.4f };

    Path backgroundArc;
    {
        backgroundArc.addCentredArc(bounds.getCentreX(),
            bounds.getCentreY(),
            arcRadius,
            arcRadius,
            0.0f,
            rotaryStartAngle,
            rotaryEndAngle,
            true);

        {
            juce::Graphics trackG{ sourceImage };
	        trackG.setColour(outline.withAlpha(1.0f));
	        trackG.strokePath(backgroundArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));
        }

        innerShadow.configure(sourceImage,
            juce::Colours::black,
            juce::AffineTransform::translation(lineW * 0.25f, lineW * 0.25f),
            juce::Colours::white,
            juce::AffineTransform::translation(lineW * -0.25f, lineW * -0.25f),
            lineW * 0.5f);
        innerShadow.getEffect()->applyEffect(trackShadowImage, {}, true);
    }

    {
        mescal::ConicGradient thumbConicGradient;

        thumbConicGradient.setRadiusRange(radiusRange);

        float stopInterval = 1.0f / 32.0f;
        float angleStep = sliderPosProportional * (rotaryEndAngle - rotaryStartAngle) * stopInterval;
        if (angleStep > 0.0f)
        {
            auto endColor128 = mescal::Color128::fromHSV(0.75f, 1.0f, 1.0f, 0.5f);
            for (float angle = rotaryStartAngle; angle <= toAngle; angle += angleStep)
            {
                auto color128 = mescal::Color128::fromHSV(0.25f + 0.5f * (angle - rotaryStartAngle) / (toAngle - rotaryStartAngle) , 1.0f, 1.0f, 0.5f);
                thumbConicGradient.addStop(angle, color128, color128);
            }

            thumbConicGradient.addStop(toAngle, endColor128, endColor128);

            thumbConicGradient.draw(conicGradientArcImage, juce::AffineTransform::translation(bounds.toFloat().getCentre()), juce::Colours::transparentBlack, true);
        }

        {
            auto blur = mescal::Effect::GaussianBlur::create(20.0f);
            blur << conicGradientArcImage;

            auto glowComposite = mescal::Effect::ArithmeticComposite::create(0.0f, 2.0f, 0.5f, 0.0f);
            glowComposite << blur;
            glowComposite << trackShadowImage;

            glowComposite->applyEffect(glowImage, {}, false);
        }
    }

    g.drawImageAt(glowImage, 0, 0);
    g.setColour(juce::Colour::greyLevel(0.8f));
    g.strokePath(backgroundArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));
    g.drawImageAt(trackShadowImage, 0, 0);
    g.drawImageAt(conicGradientArcImage, 0, 0);
}
