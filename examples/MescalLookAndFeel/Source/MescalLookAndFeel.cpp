#include "MescalLookAndFeel.h"

MescalLookAndFeel::MescalLookAndFeel()
{
    setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour{ 0xffd5d5d5 });
    setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);
    setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::black);
    setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colour{ 0x00000000 });
    setColour(juce::Slider::ColourIds::backgroundColourId, juce::Colour::greyLevel(0.9f));
    setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::skyblue);
        //juce::Colours::lightgrey);
    setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::orange.withAlpha(1.0f));
        //juce::Colour::greyLevel(0.85f));
    setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
}

void MescalLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    juce::Image buttonImage{ juce::Image::ARGB, button.getWidth(), button.getHeight(), true };
    juce::Image outputImage{ juce::Image::ARGB, button.getWidth(), button.getHeight(), true };

#if 1
    auto r = buttonImage.getBounds().toFloat().reduced(2.0f);

    auto cornerProportion = 0.2f;

    {
        juce::Graphics imageG{ buttonImage };
        auto topColor = juce::Colour{ 0xffb0b4bf };
        auto c1 = juce::Colour{ 0xffcdd0d7 };
        auto c2 = juce::Colour{ 0xffdee1e6 };
        auto bottomColor = juce::Colour{ 0xfff5f6fa };

        float start = r.getBottom();
        float end = r.getY();

        if (shouldDrawButtonAsDown || button.getToggleState())
        {
            std::swap(topColor, bottomColor);
            std::swap(c1, c2);
        }

        auto gradient = juce::ColourGradient::vertical(topColor,
            start,
            bottomColor,
            end);

        gradient.addColour(0.33f, c1);
        gradient.addColour(0.66f, c2);

        imageG.setGradientFill(gradient);
        imageG.fillRoundedRectangle(r, cornerProportion * r.getHeight());
    }

    mescal::Effect::Ptr outputEffect;

    juce::Colour innerUpperLeftShadowColor = juce::Colours::white.withAlpha(0.5f);
    juce::Colour innerLowerRightShadowColor = juce::Colours::black.withAlpha(0.5f);
    float innerShadowSize = (float)button.getHeight() * 0.025f;

    if (button.getToggleState() || shouldDrawButtonAsDown)
    {
        std::swap(innerUpperLeftShadowColor, innerLowerRightShadowColor);
        innerShadowSize *= 2.0f;
    }

    {
        auto innerUpperLeftShadow = createInnerShadow(buttonImage,
            innerUpperLeftShadowColor,
            innerShadowSize,
            juce::AffineTransform::scale(1.0f, 1.0f).translated(innerShadowSize, innerShadowSize));
        auto innerLowerRightShadow = createInnerShadow(buttonImage,
            innerLowerRightShadowColor,
            innerShadowSize,
            juce::AffineTransform::scale(1.0f, 1.0f).translated(-innerShadowSize, -innerShadowSize));

        auto innerShadowBlend = mescal::Effect::create(mescal::Effect::Type::blend) << innerLowerRightShadow << innerUpperLeftShadow;
        innerShadowBlend->setPropertyValue(mescal::Effect::Blend::mode, mescal::Effect::Blend::multiply);

        auto innerShadowSourceImageBlend = mescal::Effect::create(mescal::Effect::Type::blend) << buttonImage << innerShadowBlend;
        innerShadowSourceImageBlend->setPropertyValue(mescal::Effect::Blend::mode, mescal::Effect::Blend::linearLight);

        outputEffect = innerShadowSourceImageBlend;
    }

    if (button.getToggleState() || shouldDrawButtonAsDown)
    {
        auto innerGlow = createInnerShadow(buttonImage, backgroundColour, (float)button.getHeight() * 0.25f, {});
        auto innerGlowBlend = mescal::Effect::Blend::create(mescal::Effect::Blend::multiply) << outputEffect << innerGlow;

        outputEffect = innerGlowBlend;
    }

    outputEffect->applyEffect(outputImage, {}, false);
    g.drawImageAt(outputImage, 0, 0);
#endif

#if 0
    auto emboss = new mescal::Effect{ mescal::Effect::Type::emboss };
    emboss->setInput(0, topImage);

    mescal::Effect::Ptr chromaKey = new mescal::Effect{ mescal::Effect::Type::chromaKey };
    chromaKey->setInput(0, emboss);

    mescal::Effect::Ptr blend = new mescal::Effect{ mescal::Effect::Type::blend };
    blend->setPropertyValue(mescal::Effect::Blend::mode, mescal::Effect::Blend::subtract);
    blend->setInput(0, middleComposite);
    blend->setInput(1, chromaKey);
#endif

#if 0
    auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
        .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColour = baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);

    {
        juce::Graphics buttonGraphics{ buttonImage };

        auto cornerSize = 6.0f;
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

        buttonGraphics.setColour(baseColour);

        auto flatOnLeft = button.isConnectedOnLeft();
        auto flatOnRight = button.isConnectedOnRight();
        auto flatOnTop = button.isConnectedOnTop();
        auto flatOnBottom = button.isConnectedOnBottom();

        if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
        {
            juce::Path path;
            path.addRoundedRectangle(bounds.getX(), bounds.getY(),
                bounds.getWidth(), bounds.getHeight(),
                cornerSize, cornerSize,
                !(flatOnLeft || flatOnTop),
                !(flatOnRight || flatOnTop),
                !(flatOnLeft || flatOnBottom),
                !(flatOnRight || flatOnBottom));

            buttonGraphics.fillPath(path);
        }
        else
        {
            buttonGraphics.fillRoundedRectangle(bounds, cornerSize);
        }
    }

    float innerShadowSize = (float)button.getHeight() * 0.2f;

    juce::Colour topShadowColor = baseColour.brighter();
    juce::Colour bottomShadowColor = baseColour.darker();
    if (shouldDrawButtonAsDown)
    {
        std::swap(topShadowColor, bottomShadowColor);
    }

    auto innerTopShadow = createInnerShadow(buttonImage, topShadowColor, innerShadowSize,
        juce::AffineTransform::scale(2.0f, 2.0f, (float)buttonImage.getWidth() * 0.5f, (float)buttonImage.getHeight() * 0.5f).translated(0.0f, innerShadowSize * 2.0f));

    auto innerBottomShadow = createInnerShadow(buttonImage, bottomShadowColor, innerShadowSize,
        juce::AffineTransform::scale(2.0f, 2.0f, (float)buttonImage.getWidth() * 0.5f, (float)buttonImage.getHeight() * 0.5f).translated(0.0f, -innerShadowSize * 2.0f));

    auto innerShadowComposite = mescal::Effect::create(mescal::Effect::Type::blend) << innerBottomShadow << innerTopShadow;
    innerShadowComposite->setPropertyValue(mescal::Effect::Blend::mode, mescal::Effect::Blend::linearLight);

    auto innerShadowSourceImageComposite = mescal::Effect::create(mescal::Effect::Type::blend) << buttonImage << innerShadowComposite;
    innerShadowSourceImageComposite->setPropertyValue(mescal::Effect::Blend::mode, mescal::Effect::Blend::multiply);

    innerShadowSourceImageComposite->applyEffect(outputImage, {}, false);
    g.drawImageAt(outputImage, 0, 0);

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

    auto alphaMaskTransform = new mescal::Effect{ mescal::Effect::Type::affineTransform2D };
    alphaMaskTransform->setInput(0, innerShadow);
    alphaMaskTransform->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, transform);

    auto alphaMaskEffect = new mescal::Effect{ mescal::Effect::Type::alphaMask };
    alphaMaskEffect->setInput(0, alphaMaskTransform);
    alphaMaskEffect->setInput(1, sourceImage);

    return alphaMaskEffect;
}

mescal::Effect::Ptr MescalLookAndFeel::create3DInnerShadow(juce::Image const& sourceImage,
    juce::Colour topColor,
    juce::AffineTransform topShadowTransform,
    juce::Colour bottomColor,
    juce::AffineTransform bottomShadowTransform,
    float shadowSize)
{
    auto flood = mescal::Effect::Flood::create(juce::Colours::blue) << sourceImage;

    auto arithmeticComposite = mescal::Effect::ArithmeticComposite::create(0.0f, 1.0f, -1.0f, 0.0f) << flood << sourceImage;

    auto upperShadow = mescal::Effect::Shadow::create(shadowSize, topColor) << arithmeticComposite;
    auto upperTransform = mescal::Effect::AffineTransform2D::create(topShadowTransform) << upperShadow;

    auto lowerShadow = mescal::Effect::Shadow::create(shadowSize, bottomColor) << arithmeticComposite;
    auto lowerTransform = mescal::Effect::AffineTransform2D::create(bottomShadowTransform) << lowerShadow;

    auto blend = mescal::Effect::Blend::create(mescal::Effect::Blend::multiply) << upperTransform << lowerTransform;

    auto alphaMask = mescal::Effect::AlphaMask::create() << blend << sourceImage;

    return alphaMask;
}
