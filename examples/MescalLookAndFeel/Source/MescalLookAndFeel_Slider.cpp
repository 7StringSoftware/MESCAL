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

        //auto trackWidth = getSliderThumbRadius(slider) * 1.1f;

        Point<float> startPoint(slider.isHorizontal() ? (float)x : (float)x + (float)width * 0.5f,
            slider.isHorizontal() ? (float)y + (float)height * 0.5f : (float)(height + y));

        Point<float> endPoint(slider.isHorizontal() ? (float)(width + x) : startPoint.x,
            slider.isHorizontal() ? startPoint.y : (float)y);


#if 0
        Path backgroundTrack;
        backgroundTrack.startNewSubPath(startPoint);
        backgroundTrack.lineTo(endPoint);
        g.setColour(slider.findColour(Slider::backgroundColourId));
        g.strokePath(backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

        g.setColour(juce::Colours::red);
        trackWidth = getSliderThumbRadius(slider) * 1.1f;
        juce::Rectangle<float> r{ startPoint.x, startPoint.y - trackWidth * 0.5f, endPoint.x - startPoint.x, trackWidth };
        //g.drawRect(r);
        float cornerSize = 0.5f * trackWidth;
        g.fillRoundedRectangle(r.expanded(cornerSize * 2.0f, 0.0f), cornerSize);
#else
        auto trackWidth = getSliderThumbRadius(slider) * 1.8f;
        auto trackArea = paintSliderTrack(g,
            startPoint, endPoint,
            slider.findColour(Slider::backgroundColourId), slider.findColour(Slider::trackColourId),
            trackWidth, slider.isHorizontal());
#endif

        Path valueTrack;
        Point<float> minPoint, maxPoint, thumbPoint;

        if (isTwoVal || isThreeVal)
        {
            minPoint = { slider.isHorizontal() ? minSliderPos : (float)width * 0.5f,
                         slider.isHorizontal() ? (float)height * 0.5f : minSliderPos };

            if (isThreeVal)
                thumbPoint = { slider.isHorizontal() ? sliderPos : (float)width * 0.5f,
                               slider.isHorizontal() ? (float)height * 0.5f : sliderPos };

            maxPoint = { slider.isHorizontal() ? maxSliderPos : (float)width * 0.5f,
                         slider.isHorizontal() ? (float)height * 0.5f : maxSliderPos };
        }
        else
        {
            auto kx = slider.isHorizontal() ? sliderPos : ((float)x + (float)width * 0.5f);
            auto ky = slider.isHorizontal() ? ((float)y + (float)height * 0.5f) : sliderPos;

            minPoint = startPoint;
            maxPoint = { kx, ky };
        }

#if 0
        auto thumbWidth = getSliderThumbRadius(slider);

        valueTrack.startNewSubPath(minPoint);
        valueTrack.lineTo(isThreeVal ? thumbPoint : maxPoint);
        g.setColour(slider.findColour(Slider::trackColourId));
        g.strokePath(valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

        if (!isTwoVal)
        {
            g.setColour(slider.findColour(Slider::thumbColourId));
            g.fillEllipse(Rectangle<float>(static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre(isThreeVal ? thumbPoint : maxPoint));
        }

        if (isTwoVal || isThreeVal)
        {
            auto sr = jmin(trackWidth, (slider.isHorizontal() ? (float)height : (float)width) * 0.4f);
            auto pointerColour = slider.findColour(Slider::thumbColourId);

            if (slider.isHorizontal())
            {
                drawPointer(g, minSliderPos - sr,
                    jmax(0.0f, (float)y + (float)height * 0.5f - trackWidth * 2.0f),
                    trackWidth * 2.0f, pointerColour, 2);

                drawPointer(g, maxSliderPos - trackWidth,
                    jmin((float)(y + height) - trackWidth * 2.0f, (float)y + (float)height * 0.5f),
                    trackWidth * 2.0f, pointerColour, 4);
            }
            else
            {
                drawPointer(g, jmax(0.0f, (float)x + (float)width * 0.5f - trackWidth * 2.0f),
                    minSliderPos - trackWidth,
                    trackWidth * 2.0f, pointerColour, 1);

                drawPointer(g, jmin((float)(x + width) - trackWidth * 2.0f, (float)x + (float)width * 0.5f), maxSliderPos - sr,
                    trackWidth * 2.0f, pointerColour, 3);
            }
        }

        if (slider.isBar())
            drawLinearSliderOutline(g, x, y, width, height, style, slider);
#else
        if (!isTwoVal)
        {
            //g.setColour(slider.findColour(Slider::thumbColourId));
            //auto thumbWidth = getSliderThumbRadius(slider);
            //g.fillEllipse(Rectangle<float>(static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre(isThreeVal ? thumbPoint : maxPoint));

            paintSliderThumb(g, trackArea, slider.findColour(Slider::thumbColourId), sliderPos, getSliderThumbRadius(slider), trackWidth, slider.isHorizontal());
        }

        if (isTwoVal || isThreeVal)
        {
            auto sr = jmin(trackWidth, (slider.isHorizontal() ? (float)height : (float)width) * 0.4f);
            auto pointerColour = slider.findColour(Slider::thumbColourId);

            if (slider.isHorizontal())
            {
                drawPointer(g, minSliderPos - sr,
                    jmax(0.0f, (float)y + (float)height * 0.5f - trackWidth * 2.0f),
                    trackWidth * 2.0f, pointerColour, 2);

                drawPointer(g, maxSliderPos - trackWidth,
                    jmin((float)(y + height) - trackWidth * 2.0f, (float)y + (float)height * 0.5f),
                    trackWidth * 2.0f, pointerColour, 4);
            }
            else
            {
                drawPointer(g, jmax(0.0f, (float)x + (float)width * 0.5f - trackWidth * 2.0f),
                    minSliderPos - trackWidth,
                    trackWidth * 2.0f, pointerColour, 1);

                drawPointer(g, jmin((float)(x + width) - trackWidth * 2.0f, (float)x + (float)width * 0.5f), maxSliderPos - sr,
                    trackWidth * 2.0f, pointerColour, 3);
            }
        }

        if (slider.isBar())
            drawLinearSliderOutline(g, x, y, width, height, style, slider);
#endif
    }
}

void MescalLookAndFeel::LinearHorizontalSlider::paintImages()
{
    auto outlineColor = slider.findColour(juce::ComboBox::ColourIds::outlineColourId);

    {
        juce::Graphics g{ trackImage };
        g.setColour(juce::Colours::transparentBlack);
        g.getInternalContext().fillRect(trackImage.getBounds(), true);

        {
            auto startColor = outlineColor.withMultipliedLightness(1.0f);
            auto stopColor = outlineColor.withMultipliedLightness(1.2f);
            g.setGradientFill(juce::ColourGradient::vertical(startColor, expandedPaintArea.getY(), stopColor, expandedPaintArea.getHeight()));
            auto cornerSize = cornerProportion * expandedPaintArea.getWidth();
            g.fillRoundedRectangle(trackImage.getBounds().toFloat().reduced(0.0f, outlineThickness), cornerSize);
        }
    }

    {
        juce::Graphics g{ thumbImage };
        g.setColour(juce::Colours::transparentBlack);
        g.getInternalContext().fillRect(thumbImage.getBounds(), true);

        auto thumbColor = slider.findColour(juce::Slider::ColourIds::thumbColourId);
        g.setColour(thumbColor);
        g.fillEllipse(juce::Rectangle<float>{ thumbSize, thumbSize }.withCentre(thumbImage.getBounds().toFloat().getCentre()));
    }
}

void MescalLookAndFeel::LinearHorizontalSlider::createGraph()
{
    auto shadowTransform = juce::AffineTransform::scale(2.0f, 2.0f, expandedPaintArea.getCentreX(), expandedPaintArea.getCentreY()).translated(0.0f, expandedPaintArea.getHeight() * 0.5f);
    auto shadow = createInnerShadow(trackImage, juce::Colours::black, expandedPaintArea.getHeight() * 0.2f, shadowTransform);

    mescal::Effect::Ptr blend = new mescal::Effect{ mescal::Effect::Type::blend };
    blend->setInput(0, trackImage);
    blend->setInput(1, shadow);
    blend->setPropertyValue(mescal::Effect::Blend::mode, mescal::Effect::Blend::overlay);

    mescal::Effect::Ptr thumbWithShadow = addShadow(thumbImage, juce::Colours::black, thumbSize * 0.1f, juce::AffineTransform::translation(0.0f, thumbSize * 0.15f));

    effectGraph = blend;
    thumbGraph = thumbWithShadow;
}

void MescalLookAndFeel::LinearHorizontalSlider::paint(juce::Graphics& g)
{
    paintImages();

    effectGraph->applyEffect(outputImage, {}, true);
    g.drawImage(outputImage, expandedPaintArea, juce::RectanglePlacement::doNotResize);

    g.reduceClipRegion(expandedPaintArea.toNearestIntEdges());
    thumbGraph->applyEffect(thumbOutputImage, {}, false);
    g.drawImage(thumbOutputImage,
        juce::Rectangle<float>{ sliderPos - (float)thumbOutputImage.getWidth() * 0.5f, expandedPaintArea.getY(), (float)thumbOutputImage.getWidth(), (float)thumbOutputImage.getHeight() },
        juce::RectanglePlacement::doNotResize);

    g.setColour(juce::Colours::white);
    g.drawRoundedRectangle(expandedPaintArea.reduced(0.0f, outlineThickness),
        cornerProportion * expandedPaintArea.getWidth(),
        outlineThickness);
}

void MescalLookAndFeel::LinearVerticalSlider::paintImages()
{
    auto outlineColor = slider.findColour(juce::ComboBox::ColourIds::outlineColourId);

    {
        juce::Graphics g{ trackImage };
        g.setColour(juce::Colours::transparentBlack);
        g.getInternalContext().fillRect(trackImage.getBounds(), true);

        {
            auto startColor = outlineColor.withMultipliedLightness(1.0f);
            auto stopColor = outlineColor.withMultipliedLightness(1.2f);
            g.setGradientFill(juce::ColourGradient::vertical(startColor, expandedPaintArea.getY(), stopColor, expandedPaintArea.getHeight() * 0.2f));
            auto cornerSize = cornerProportion * expandedPaintArea.getHeight();
            g.fillRoundedRectangle(trackImage.getBounds().toFloat().reduced(outlineThickness, 0.0f), cornerSize);
        }
    }

    {
        juce::Graphics g{ thumbImage };
        g.setColour(juce::Colours::transparentBlack);
        g.getInternalContext().fillRect(thumbImage.getBounds(), true);

        auto thumbColor = slider.findColour(juce::Slider::ColourIds::thumbColourId);
        //g.setColour(thumbColor);
        g.setGradientFill(juce::ColourGradient{ thumbColor, thumbImage.getBounds().toFloat().getCentre(), thumbColor.darker(), { 0.0f, 0.0f }, true });
        //g.fillEllipse(juce::Rectangle<float>{ thumbSize, thumbSize }.withCentre(thumbImage.getBounds().toFloat().getCentre()));
//         g.fillEllipse(juce::Rectangle<float>{ thumbSize, thumbSize }.withCentre(thumbImage.getBounds().toFloat().getCentre()));
//         g.setColour(thumbColor.contrasting());
//         g.drawEllipse(juce::Rectangle<float>{ thumbSize, thumbSize }.withCentre(thumbImage.getBounds().toFloat().getCentre()), thumbSize * 0.1f);
    }
}

void MescalLookAndFeel::LinearVerticalSlider::createGraph()
{
    auto shadowTransform = juce::AffineTransform::scale(2.0f, 2.0f, expandedPaintArea.getCentreX(), expandedPaintArea.getCentreY()).translated(0.0f, expandedPaintArea.getHeight() * 0.5f);
    auto shadow = createInnerShadow(trackImage, juce::Colours::black, expandedPaintArea.getHeight() * 0.02f, shadowTransform);

    mescal::Effect::Ptr blend = new mescal::Effect{ mescal::Effect::Type::blend };
    blend->setInput(0, trackImage);
    blend->setInput(1, shadow);
    blend->setPropertyValue(mescal::Effect::Blend::mode, mescal::Effect::Blend::overlay);

    mescal::Effect::Ptr thumbWithShadow = addShadow(thumbImage, juce::Colours::black, thumbSize * 0.08f, juce::AffineTransform::translation(0.0f, thumbSize * 0.12f));

    effectGraph = blend;
    thumbGraph = thumbWithShadow;
}

void MescalLookAndFeel::LinearVerticalSlider::paint(juce::Graphics& g)
{
    paintImages();

    effectGraph->applyEffect(outputImage, {}, true);
    g.drawImage(outputImage, expandedPaintArea, juce::RectanglePlacement::doNotResize);

    g.reduceClipRegion(expandedPaintArea.toNearestIntEdges());
    thumbGraph->applyEffect(thumbOutputImage, {}, false);
    g.drawImage(thumbOutputImage,
        juce::Rectangle<float>{ expandedPaintArea.getX(),
        sliderPos - (float)thumbOutputImage.getHeight() * 0.5f,
        (float)thumbOutputImage.getWidth(),
        (float)thumbOutputImage.getHeight() },
        juce::RectanglePlacement::doNotResize);

    g.setColour(juce::Colours::white);
    g.drawRoundedRectangle(expandedPaintArea.reduced(outlineThickness, 0.0f),
        cornerProportion * expandedPaintArea.getHeight(),
        outlineThickness);
}
