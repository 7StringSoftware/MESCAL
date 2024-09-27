#include "MescalLookAndFeel.h"

using namespace juce;

Slider::SliderLayout MescalLookAndFeel::getSliderLayout(Slider& slider)
{
    auto layout = juce::LookAndFeel_V4::getSliderLayout(slider);
    layout.sliderBounds.reduce(4, 4);
    return layout;
}

void MescalLookAndFeel::drawLinearSlider(Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, Slider::SliderStyle style, Slider& slider)
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

    if (slider.isBar())
    {
        auto sliderRect = slider.getLocalBounds().toFloat();
        auto trackImage = getImage(0, slider.getLocalBounds());
        auto outputImage = getImage(1, slider.getLocalBounds());

        Rectangle<float> valueRect;
        if (slider.isHorizontal())
        {
            valueRect = sliderRect.withWidth(sliderPos - (float)x).reduced(0.0f, 4.0f);
        }
        else
        {
            valueRect = sliderRect.withY(sliderPos).withHeight((float)y + (float)height - sliderPos).reduced(4.0f, 0.0f);
        }

        {
            Graphics valueG{ trackImage };
            clear(valueG);
            valueG.setColour(thumbColor);
            valueG.fillRect(valueRect);
        }

        glow.configure(trackImage,
            trackColor,
            5.0f);

        glow.getEffect()->applyEffect(outputImage, {}, false);

        g.drawImageAt(outputImage, 0, 0);
        return;
    }

    //
    // Paint the slider track
    //
    float thumbRadius = (float)getSliderThumbRadius(slider);
    float trackThickness = thumbRadius * 0.75f;
    Rectangle<float> thumbRect{ thumbRadius * 2.5f, thumbRadius * 2.5f };

    auto trackRect = Rectangle<float>{ startPoint, endPoint };
    if (slider.isHorizontal())
    {
        trackRect = trackRect.withSizeKeepingCentre(trackRect.getWidth() + trackThickness, trackThickness);
        thumbRect.setCentre({ sliderPos, trackRect.getCentreY() });
    }
    else
    {
        trackRect = trackRect.withSizeKeepingCentre(trackThickness, trackRect.getHeight() + trackThickness);
        thumbRect.setCentre({ trackRect.getCentreX(), sliderPos });
    }

    auto trackImage = getImage(0, trackRect);
    auto outputImage = getImage(2, trackImage.getBounds());

    trackRect = trackImage.getBounds().toFloat().withPosition(trackRect.getPosition());

    {
        Graphics trackG{ trackImage };
        clear(trackG);

        auto trackRectWithinImage = trackRect.withZeroOrigin();

        if (isTwoVal || isThreeVal)
        {
            Path p;

            if (slider.isHorizontal())
            {
                auto dashedR = trackRect.reduced(1.0f).translated(0.0f, -1.0f);
                p.addRoundedRectangle(dashedR, dashedR.getHeight() * 0.5f);

                trackRectWithinImage = dashedR.withWidth(trackThickness + maxSliderPos - minSliderPos).withX(minSliderPos - trackThickness * 0.5f) - trackRect.getPosition();
            }
            else
            {
                auto dashedR = trackRect.reduced(1.0f).translated(-1.0f, 0.0f);
                p.addRoundedRectangle(dashedR, dashedR.getWidth() * 0.5f);

                trackRectWithinImage = dashedR.withHeight(trackThickness + minSliderPos - maxSliderPos).withY(maxSliderPos - trackThickness * 0.5f) - trackRect.getPosition();
            }

            PathStrokeType stroke{ 1.0f };
            Path dashedPath;
            float dashLength = 2.0f;
            stroke.createDashedStroke(dashedPath, p, &dashLength, 1);
            g.setColour(backgroundColor.darker());
            g.fillPath(dashedPath);
        }

        auto color = isTwoVal ? thumbColor : backgroundColor;
        trackG.setColour(color);
        trackG.fillRoundedRectangle(trackRectWithinImage, trackThickness * 0.5f);

        if (!isTwoVal)
        {
            juce::Rectangle<float> valueRect = slider.isHorizontal() ?
                trackRectWithinImage.withRight(sliderPos - (float)x) :
                trackRectWithinImage.withY(sliderPos - (float)y).withBottom(trackRectWithinImage.getBottom());

            trackG.setColour(trackColor);

            trackG.fillRoundedRectangle(valueRect, trackThickness * 0.5f);
        }
    }

    {
        auto topLeftShadowColor = Colours::black;
        auto bottomRightShadowColor = Colours::white;

        if (isTwoVal)
        {
            std::swap(topLeftShadowColor, bottomRightShadowColor);
        }

        float innerShadowSize = 2.0f;
        innerShadow.configure(trackImage,
            topLeftShadowColor,
            AffineTransform::scale(1.0f, 1.0f).translated(innerShadowSize * 1.0f, innerShadowSize * 1.0f),
            bottomRightShadowColor,
            AffineTransform::scale(1.0f, 1.0f,
                (float)trackImage.getWidth() * 0.5f,
                (float)trackImage.getHeight() * 0.5f).translated(innerShadowSize * -1.0f, innerShadowSize * -1.0f),
            innerShadowSize);

        auto imageWithInnerShadow = mescal::Effect::create(mescal::Effect::Type::composite) << trackImage << innerShadow.getEffect();
        imageWithInnerShadow->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);

        imageWithInnerShadow->applyEffect(outputImage, {}, true);
    }

    g.drawImage(outputImage, outputImage.getBounds().toFloat() + trackRect.getTopLeft(), juce::RectanglePlacement::doNotResize);

    //
    // Paint the thumb
    //
    if (!isTwoVal)
    {
        g.setColour(thumbColor);

        auto thumbImage = getImage(1, thumbRect);
        auto thumbOutput = getImage(3, thumbRect);

        {
            Graphics thumbG{ thumbImage };
            clear(thumbG);

            auto ellipseR = thumbImage.getBounds().toFloat().withSizeKeepingCentre(thumbRadius * 2.0f, thumbRadius * 2.0f);

            thumbG.setGradientFill(ColourGradient
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
                Colours::black.withAlpha(0.5f),
                thumbRadius * 0.05f,
                AffineTransform::translation(thumbRadius * 0.1f, thumbRadius * 0.1f));

            innerShadow.configure(thumbImage,
                Colours::white.withAlpha(0.5f),
                AffineTransform::translation(thumbRadius * 0.5f, thumbRadius * 0.55f),
                Colours::black.withAlpha(0.125f),
                AffineTransform::translation(thumbRadius * -0.125f, thumbRadius * -0.125f),
                thumbRadius * 0.25f);

            auto imageWithInnerShadow = mescal::Effect::create(mescal::Effect::Type::composite) << thumbImage << innerShadow.getEffect();
            imageWithInnerShadow->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);

            dropShadow.getEffect()->applyEffect(thumbOutput, {}, true);
            imageWithInnerShadow->applyEffect(thumbOutput, {}, false);
        }

        g.drawImage(thumbOutput, thumbOutput.getBounds().toFloat() + thumbRect.getTopLeft(), RectanglePlacement::doNotResize);
    }
}

void MescalLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, Slider& slider)
{
    return;

    auto outline = slider.findColour(Slider::rotarySliderOutlineColourId);
    auto fill = slider.findColour(Slider::rotarySliderFillColourId);

    auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(10);

    auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = jmin(8.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;

    Image trackImage{ Image::ARGB, width, height, true };
    Image outputImage{ Image::ARGB, width, height, true };
    auto backgroundColor = slider.findColour(Slider::backgroundColourId);
    Path backgroundArc;

    {
        Graphics imageG{ trackImage };
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
            MathConstants<float>::pi,
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
            rotaryStartAngle + MathConstants<float>::pi,
            0.0f,
            MathConstants<float>::pi,
            false);

        imageG.setColour(backgroundColor);
        imageG.fillPath(backgroundArc);
    }

    {
        Graphics::ScopedSaveState save{ g };
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

    Rectangle<float> thumbRect{ thumbRadius * 2.0f, thumbRadius * 2.0f };
    thumbRect.setCentre(thumbPoint);

    {
        Graphics trackG{ trackImage };
        trackG.setColour(Colours::transparentBlack);
        trackG.getInternalContext().fillRect(trackImage.getBounds(), true);

        trackG.setGradientFill(ColourGradient
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
        outerLowerDropShadow->setPropertyValue(mescal::Effect::Shadow::color, Colours::black.withAlpha(0.5f));

        auto outerLowerDropShadowTransform = mescal::Effect::create(mescal::Effect::Type::affineTransform2D) << outerLowerDropShadow;
        outerLowerDropShadowTransform->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, AffineTransform::translation(thumbRadius * 0.1f, thumbRadius * 0.1f));

        innerShadow.configure(trackImage,
            Colours::white.withAlpha(0.5f),
            AffineTransform::translation(thumbRadius * 0.5f, thumbRadius * 0.5f),
            Colours::black.withAlpha(0.5f),
            AffineTransform::translation(thumbRadius * -0.5f, thumbRadius * -0.5f),
            thumbRadius * 0.25f);

        auto imageWithUpperShadow = mescal::Effect::create(mescal::Effect::Type::composite) << trackImage << innerShadow.getEffect();
        imageWithUpperShadow->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);

        //             auto buttonComposite = mescal::Effect::create(mescal::Effect::Type::composite) << outerLowerDropShadowTransform << imageWithUpperShadow;
        //             buttonComposite->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);

                    //auto buttonBlend = mescal::Effect::Blend::create(mescal::Effect::Blend::overlay) << imageWithUpperShadow << outerLowerDropShadowTransform;

                    //buttonBlend->applyEffect(outputImage, {}, true);
        outerLowerDropShadowTransform->applyEffect(outputImage, {}, true);
        imageWithUpperShadow->applyEffect(outputImage, {}, false);
    }

    g.setColour(Colours::black.withAlpha(0.25f));
    g.fillEllipse(thumbRect.expanded(1.0f));
    g.setColour(Colours::black);
    g.drawImageAt(outputImage, 0, 0);
}
