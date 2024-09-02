#include "MescalLookAndFeel.h"

MescalLookAndFeel::MescalLookAndFeel()
{
    setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour{ 0xffd5d5d5 });
    setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);
    setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::black);
    setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colour{ 0xff959595 });
    setColour(juce::Slider::ColourIds::trackColourId, juce::Colour::greyLevel(0.5f).withAlpha(0.5f));
    setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::lightsteelblue);
    setColour(juce::Slider::ColourIds::backgroundColourId, juce::Colour::greyLevel(0.85f));
}

void MescalLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
#if 0
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.45f;
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = juce::jmin(8.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc(bounds.getCentreX(),
        bounds.getCentreY(),
        arcRadius,
        arcRadius,
        0.0f,
        rotaryStartAngle,
        rotaryEndAngle,
        true);
    juce::PathStrokeType backgroundArcStrokeType{ lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded };
    juce::Path strokedBackgroundArc;
    backgroundArcStrokeType.createStrokedPath(strokedBackgroundArc, backgroundArc);

    auto outline = slider.findColour(juce::Slider::rotarySliderOutlineColourId);
    g.setColour(outline);
    g.fillPath(strokedBackgroundArc);

    mescal::ConicGradient conicGradient;
    conicGradient.setRadiusRange({ arcRadius - lineW, arcRadius + lineW });

    float maxStopAngle = juce::MathConstants<float>::twoPi / 32.0f;
    size_t numStops = (size_t)(std::ceil((toAngle - rotaryStartAngle) / maxStopAngle)) + 1;
    numStops = juce::jmax(numStops, (size_t)2);
    std::vector<mescal::ConicGradient::Stop> stops{ numStops + 1 };
    float angleStep = (toAngle - rotaryStartAngle) / (float)(numStops - 2);
    float angle = rotaryStartAngle - angleStep;
    float color = 0.0f;
    float colorStep = 1.0f / (float)(numStops - 2);
    for (int stop = 0; stop < numStops; ++stop)
    {
        stops[stop] = { angle, mescal::Color128{ juce::Colour::greyLevel(color)} };
        angle += angleStep;
        color += colorStep;
    }
    stops.back().angle = angle;
    stops.back().color128 = mescal::Color128{ juce::Colours::white };

    conicGradient.addStops(stops);

    if (inputScratchpad.getWidth() < width || inputScratchpad.getHeight() < height)
    {
        inputScratchpad = juce::Image(juce::Image::PixelFormat::ARGB, width, height, false);
    }

    if (outputScratchpad.getWidth() < width || outputScratchpad.getHeight() < height)
    {
        outputScratchpad = juce::Image(juce::Image::PixelFormat::ARGB, width, height, false);
    }

    conicGradient.draw(inputScratchpad, juce::AffineTransform::translation(width * 0.5f, height * 0.5f));

    mescal::Effect effect{ mescal::Effect::Type::shadow };
    effect.setPropertyValue(mescal::Effect::Shadow::color, juce::Colour::fromFloatRGBA(0.0f, 0.5f, 0.5f, 1.0f));
    effect.setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, lineW);
    effect.setInput(0, inputScratchpad);
    effect.applyEffect(outputScratchpad, {}, true);
    g.drawImageAt(outputScratchpad, 0, 0);

    {
        juce::Graphics::ScopedSaveState saveState{ g };
        g.reduceClipRegion(strokedBackgroundArc);
        g.drawImageAt(inputScratchpad, x, y);
    }
#endif
}

void MescalLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
#if 0
    images.clear();

    for (int i = 0; i < 4; ++i)
        images.emplace_back(juce::Image{ juce::Image::PixelFormat::ARGB, button.getWidth(), button.getHeight(), true, juce::NativeImageType{} });

    paint3DButtonImages(backgroundColour, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    auto graph = create3DButtonEffectGraph(shouldDrawButtonAsDown, shouldDrawButtonAsHighlighted);
    auto target = juce::Image{ juce::Image::PixelFormat::ARGB, button.getWidth(), button.getHeight(), true, juce::NativeImageType{} };
    graph->applyEffect(target, {}, true);
    g.drawImageAt(target, 0, 0);
#endif
}

void MescalLookAndFeel::paint3DButtonImages(juce::Colour backgroundColor, bool buttonHighlighted, bool buttonDown)
{
#if 0
    /*

    background gradient: #bdc1cc to #e7ebee linearly vertically

    ellipse 185px: 50% gray (#959595), overlaid with gradient #d7dae1 to #cad0d8, inner shadow #c5c8ce, drop shadow #dde0e7
    ellipse 150px: #5d5d5d, overlaid with a gradient #b0b4bf to #cdd0d7 to #dee1e6 to #f5f6fa

    ellipse 85px: #2c2c2c, overlaid with color #d4d8e3, drop shadow #e4f0ff, inner shadow #b5b5b5

    ellipse 150px copy + clear layer style, then: drop shadow with black

    blank layer above ellipse 185px, clipped to ellipse 185px: upper right quadrant filled with 50%gray (#959

    */
    auto bottomLayerRect = images.front().getBounds().toFloat();
    float cornerProportion = 0.1f;

    //
    // Bottom layer
    //
    auto middleLayerXReduction = bottomLayerRect.getWidth() * 0.03f;
    auto middleLayerYReduction = bottomLayerRect.getHeight() * 0.06f;

    {
        auto bottomImage = images.front();
        juce::Graphics g{ bottomImage };
        auto outlineColor = findColour(juce::ComboBox::ColourIds::outlineColourId);

        if (middleLayerXReduction >= 2.0f && middleLayerYReduction >= 2.0f)
        {
            g.setColour(juce::Colours::transparentBlack);
            g.getInternalContext().fillRect(bottomImage.getBounds(), true);

            //g.setColour(juce::Colour{ 0xff959595 });
            //g.fillRoundedRectangle(bottomLayerRect, cornerProportion * bottomLayerRect.getHeight());

            {
                //auto gradient = ;
                auto topColor = outlineColor.withMultipliedLightness(1.0f);
                auto bottomColor = outlineColor.withMultipliedLightness(1.2f);
                g.setGradientFill(juce::ColourGradient::vertical(topColor, 0, bottomColor, (float)bottomImage.getHeight()));
                //g.fillEllipse(bottomLayerRect);
                g.fillRoundedRectangle(bottomLayerRect, cornerProportion * bottomLayerRect.getHeight());
            }
        }
        else
        {
            g.setColour(outlineColor);
            g.getInternalContext().fillRect(bottomImage.getBounds(), true);

            middleLayerXReduction = 0.0f;
            middleLayerYReduction = 0.0f;
        }
    }

    //
    // Middle layer
    //
    if (buttonDown)
        bottomLayerRect.reduce(bottomLayerRect.proportionOfWidth(0.025f), bottomLayerRect.proportionOfHeight(0.025f));

    {
        auto middleEllipseImage = images[1];
        juce::Graphics g{ middleEllipseImage };
        g.setColour(juce::Colours::transparentBlack);
        g.getInternalContext().fillRect(middleEllipseImage.getBounds(), true);

        auto middleLayerRect = bottomLayerRect.reduced(middleLayerXReduction, middleLayerYReduction);

        auto midColor = backgroundColor;

        g.setColour(backgroundColor);
        g.fillRoundedRectangle(middleLayerRect, cornerProportion * middleLayerRect.getHeight());

        //         auto topColor = juce::Colour{ 0xfff5f6fa };//color.withMultipliedBrightness(3.0f);
        //         auto topMidColor = juce::Colour{ 0xffdee1e6 };
        //         auto bottomMidColor = juce::Colour{ 0xffcdd0d7 };
        //         auto bottomColor = juce::Colour{ 0xffb0b4bf };
                //b0b4bf to #cdd0d7 to #dee1e6 to #f5f6fa

        auto topColor = midColor.withMultipliedBrightness(1.1f);
        auto topMidColor = midColor.withMultipliedBrightness(1.05f);
        auto bottomMidColor = midColor.withMultipliedBrightness(0.95f);
        auto bottomColor = midColor.withMultipliedBrightness(0.9f);

        {
            juce::Graphics::ScopedSaveState saveState{ g };
            auto gradient = juce::ColourGradient::vertical(
                topColor, 0,
                bottomColor, (float)middleEllipseImage.getHeight());

            gradient.addColour(0.33f, topMidColor);
            gradient.addColour(0.66f, bottomMidColor);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(middleLayerRect, cornerProportion * middleLayerRect.getHeight());
        }
    }

    //
    // Top layer
    //
    {
        auto topImage = images[2];
        juce::Graphics g{ topImage };

        auto topLayerRect = bottomLayerRect.reduced(bottomLayerRect.getWidth() * 0.08f);

        {
            auto topColor = juce::Colour{ 0xffdcdae1 };
            auto bottomColor = juce::Colour{ 0xffd0d0d8 };

            auto gradient = juce::ColourGradient::vertical(topColor, 0, bottomColor, (float)topImage.getHeight());
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(topLayerRect, cornerProportion * topLayerRect.getHeight());
        }
    }
#endif
}


mescal::Effect::Ptr  MescalLookAndFeel::create3DButtonEffectGraph(bool buttonDown, bool buttonHighlighted)
{
#if 0
    auto bottomImage = images.front();
    auto middleLayerImage = images[1];
    auto topImage = images[2];

    //
    // Middle layer outer shadow
    //
    auto outlineColor = findColour(juce::ComboBox::ColourIds::outlineColourId);
    auto shadowColor = outlineColor.withMultipliedBrightness(0.1f);
    auto shadowTranslate = (float)middleLayerImage.getHeight() * 0.08f;

    mescal::Effect::Ptr shadowEffect = new mescal::Effect{ mescal::Effect::Type::shadow };
    shadowEffect->setInput(0, middleLayerImage);
    shadowEffect->setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, (float)middleLayerImage.getHeight() * 0.08f);
    shadowEffect->setPropertyValue(mescal::Effect::Shadow::color, mescal::colourToVector4(shadowColor));

    mescal::Effect::Ptr middleLayerOuterShadowTransform = new mescal::Effect{ mescal::Effect::Type::affineTransform2D };
    middleLayerOuterShadowTransform->setInput(0, shadowEffect);
    middleLayerOuterShadowTransform->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, juce::AffineTransform::translation(0.0f, shadowTranslate));

    mescal::Effect::Ptr middleLayerShadowComposite = new mescal::Effect{ mescal::Effect::Type::composite };
    middleLayerShadowComposite->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceOver);
    middleLayerShadowComposite->setInput(0, middleLayerOuterShadowTransform);
    middleLayerShadowComposite->setInput(1, middleLayerImage);

    mescal::Effect::Ptr bottomMiddleComposite = new mescal::Effect{ mescal::Effect::Type::composite };
    bottomMiddleComposite->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);
    bottomMiddleComposite->setInput(0, bottomImage);

    if (buttonHighlighted)
    {
        mescal::Effect::Ptr glow = new mescal::Effect{ mescal::Effect::Type::shadow };
        glow->setInput(0, middleLayerShadowComposite);
        glow->setPropertyValue(mescal::Effect::Shadow::color, outlineColor.contrasting());

        mescal::Effect::Ptr highlightComposite = new mescal::Effect{ mescal::Effect::Type::composite };
        highlightComposite->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);
        highlightComposite->setInput(0, glow);
        highlightComposite->setInput(1, middleLayerShadowComposite);

        bottomMiddleComposite->setInput(1, highlightComposite);
    }
    else
    {
        bottomMiddleComposite->setInput(1, middleLayerShadowComposite);
    }

    auto shadowTransform = juce::AffineTransform::scale(1.1f, 1.f, (float)topImage.getWidth() * 0.5f, (float)topImage.getHeight() * 0.5f);
    auto innerShadow = createInnerShadow(topImage, juce::Colour{ 0xffb5b5b5 }, (float)topImage.getHeight() * 0.15f, shadowTransform);

    mescal::Effect::Ptr topLayerComposite = new mescal::Effect{ mescal::Effect::Type::composite };
    topLayerComposite->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceOver);
    topLayerComposite->setInput(0, topImage);
    topLayerComposite->setInput(1, innerShadow);

    mescal::Effect::Ptr finalComposite = new mescal::Effect{ mescal::Effect::Type::composite };
    finalComposite->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);
    finalComposite->setInput(0, bottomMiddleComposite);
    finalComposite->setInput(1, topLayerComposite);

    return finalComposite;
#endif
    return nullptr;
}


mescal::Effect::Ptr MescalLookAndFeel::addShadow(juce::Image const& sourceImage, juce::Colour const& shadowColor, float shadowSize, juce::AffineTransform transform)
{
    auto shadow = new mescal::Effect{ mescal::Effect::Type::shadow };
    shadow->setInput(0, sourceImage);
    shadow->setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, shadowSize);
    shadow->setPropertyValue(mescal::Effect::Shadow::color, mescal::colourToVector4(shadowColor));

    auto shadowTransform = new mescal::Effect{ mescal::Effect::Type::affineTransform2D };
    shadowTransform->setInput(0, shadow);
    shadowTransform->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, transform);

    auto composite = new mescal::Effect{ mescal::Effect::Type::composite };
    composite->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceOver);
    composite->setInput(0, shadowTransform);
    composite->setInput(1, sourceImage);

    return composite;
}

mescal::Effect::Ptr MescalLookAndFeel::createInnerShadow(juce::Image const& sourceImage, juce::Colour const& shadowColor, float shadowSize, juce::AffineTransform transform)
{
    auto floodEffect = new mescal::Effect{ mescal::Effect::Type::flood };
    floodEffect->setPropertyValue(mescal::Effect::Flood::color, juce::Colours::blue);

    auto arithmeticComposite = new mescal::Effect{ mescal::Effect::Type::arithmeticComposite };
    arithmeticComposite->setPropertyValue(mescal::Effect::ArithmeticComposite::coefficients, mescal::Vector4{ 0.0f, 1.0f, -1.0f, 0.0f });
    arithmeticComposite->setInput(0, floodEffect);
    arithmeticComposite->setInput(1, sourceImage);

    auto innerShadow = new mescal::Effect{ mescal::Effect::Type::shadow };
    innerShadow->setInput(0, arithmeticComposite);
    innerShadow->setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, shadowSize);
    innerShadow->setPropertyValue(mescal::Effect::Shadow::color, mescal::colourToVector4(shadowColor));

    auto alphaMaskEffect = new mescal::Effect{ mescal::Effect::Type::alphaMask };
    alphaMaskEffect->setInput(0, innerShadow);
    alphaMaskEffect->setInput(1, sourceImage);

    auto alphaMaskTransform = new mescal::Effect{ mescal::Effect::Type::affineTransform2D };
    alphaMaskTransform->setInput(0, alphaMaskEffect);
    alphaMaskTransform->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, transform);

    auto alphaMaskEffect2 = new mescal::Effect{ mescal::Effect::Type::alphaMask };
    alphaMaskEffect2->setInput(0, alphaMaskTransform);
    alphaMaskEffect2->setInput(1, sourceImage);

    return alphaMaskEffect2;
}

juce::Rectangle<float> MescalLookAndFeel::paintSliderTrack(juce::Graphics& g,
    juce::Point<float> start, juce::Point<float> end,
    juce::Colour backgroundColor, juce::Colour trackColor,
    float trackThickness, bool horizontal)
{
    juce::Rectangle<float> trackArea{ (end.x - start.x) + trackThickness, trackThickness };
    auto paintOffset = start - juce::Point<float>{ trackThickness * 0.5f, trackThickness * 0.5f };

    if (!horizontal)
    {
        trackArea = { trackThickness, std::abs(end.y - start.y) + trackThickness };
        paintOffset = end - juce::Point<float>{ trackThickness * 0.5f, trackThickness * 0.5f };
    }

    auto imageBounds = trackArea.toNearestIntEdges();
    if (imageBounds.isEmpty())
        return {};

    juce::Image trackImage{ juce::Image::ARGB, imageBounds.getWidth(), imageBounds.getHeight(), true };

    {
        juce::Graphics trackG{ trackImage };

        trackColor = juce::Colour::greyLevel(0.95f);
        auto startColor = trackColor.withMultipliedLightness(1.1f);
        auto stopColor = trackColor.withMultipliedLightness(horizontal ? 1.0f : 0.8f);

        trackG.setColour(juce::Colour{ 0xff959595 });
        //trackG.fillRoundedRectangle(r, 0.5f * trackThickness);

        auto gradient = horizontal ? juce::ColourGradient::vertical(startColor, 0.0f, stopColor, trackThickness) :
            juce::ColourGradient::vertical(startColor, trackThickness, stopColor, 0.0f);
        trackG.setGradientFill(gradient);
        trackG.fillRoundedRectangle(trackArea, 0.5f * trackThickness);
    }

    auto shadowTransform = juce::AffineTransform::scale(2.0f, 2.0f, trackArea.getCentreX(), trackArea.getCentreY()).translated(0.0f, trackThickness * 0.5f);
    float shadowSize = horizontal ? trackThickness * 0.1f : trackThickness * 0.2f;
    auto shadow = createInnerShadow(trackImage, juce::Colours::black, shadowSize, shadowTransform);

    mescal::Effect::Ptr shadowComposite = new mescal::Effect{ mescal::Effect::Type::composite };
    shadowComposite->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);
    shadowComposite->setInput(0, trackImage);
    shadowComposite->setInput(1, shadow);

    juce::Image outputImage{ juce::Image::ARGB, (int)trackArea.getWidth(), (int)trackArea.getHeight(), true };
    shadowComposite->applyEffect(outputImage, {}, true);

    g.drawImageAt(outputImage, (int)paintOffset.x, (int)paintOffset.y);

    g.setColour(juce::Colour::greyLevel(0.1f));
    float outlineThickness = trackThickness * 0.05f;
    auto reducedTrackR = trackArea.reduced(outlineThickness, outlineThickness);
    g.drawRoundedRectangle(reducedTrackR + paintOffset, (trackThickness - outlineThickness) * 0.5f, outlineThickness);

    return trackArea + paintOffset;
}

void MescalLookAndFeel::paintSliderThumb(juce::Graphics& g, juce::Rectangle<float> trackArea, juce::Colour thumbColor, float sliderPos, float thumbSize, float sliderThickness, bool horizontal)
{
    if (trackArea.isEmpty())
        return;

    juce::Image thumbImage{ juce::Image::ARGB, (int)sliderThickness, (int)sliderThickness, true };

    {
        juce::Graphics thumbG{ thumbImage };
        //thumbG.setColour(thumbColor);
        auto thumbR = juce::Rectangle<float>{ thumbSize, thumbSize }.withCentre({ sliderThickness * 0.5f, sliderThickness * 0.5f });
        thumbG.setGradientFill(juce::ColourGradient{ thumbColor, thumbR.getCentre(), thumbColor.darker(), thumbR.getTopLeft(), true });
        thumbG.fillEllipse(thumbR);
        //         thumbG.setColour(thumbColor.contrasting());
        //         thumbG.drawEllipse(thumbR, thumbSize * 0.1f);
    }

    juce::Image outputImage{ juce::Image::ARGB, (int)sliderThickness, (int)sliderThickness, true };

    auto shadow = addShadow(thumbImage, juce::Colours::black, thumbSize * 0.12f, juce::AffineTransform::scale(1.0f, 1.0f, sliderThickness * 0.5f, sliderThickness * 0.5f).translated(0.0f, thumbSize * 0.1f));
    shadow->applyEffect(outputImage, {}, true);

    if (horizontal)
    {
        g.drawImage(outputImage,
            juce::Rectangle<float>{sliderPos - sliderThickness * 0.5f, trackArea.getY(), sliderThickness, sliderThickness},
            juce::RectanglePlacement::doNotResize);
    }
    else
    {
        g.drawImage(outputImage,
            juce::Rectangle<float>{trackArea.getX(), sliderPos - sliderThickness * 0.5f, sliderThickness, sliderThickness},
            juce::RectanglePlacement::doNotResize);
    }
}
