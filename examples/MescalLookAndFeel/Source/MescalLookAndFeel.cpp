#include "MescalLookAndFeel.h"

MescalLookAndFeel::MescalLookAndFeel()
{
    setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour{ 0xffd5d5d5 });
    setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);
    setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::black);
    setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colour{ 0x00000000 });
    setColour(juce::Slider::ColourIds::backgroundColourId, juce::Colour::greyLevel(0.9f));
    setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::lightcyan);
    setColour(juce::Slider::ColourIds::trackColourId, juce::Colour{ 0xffB6CFCF });
    setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
}

void MescalLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    juce::Image buttonImage{ juce::Image::ARGB, button.getWidth(), button.getHeight(), true };
    juce::Image outputImage{ juce::Image::ARGB, button.getWidth(), button.getHeight(), true };

    auto r = buttonImage.getBounds().toFloat().reduced(0.0f);

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
        innerShadow.configure(buttonImage,
            innerUpperLeftShadowColor,
            juce::AffineTransform::scale(1.0f, 1.0f).translated(innerShadowSize, innerShadowSize),
            innerLowerRightShadowColor,
            juce::AffineTransform::scale(1.0f, 1.0f).translated(-innerShadowSize, -innerShadowSize),
            innerShadowSize);

        auto innerShadowSourceImageBlend = mescal::Effect::create(mescal::Effect::Type::blend) << buttonImage << innerShadow.getEffect();
        innerShadowSourceImageBlend->setPropertyValue(mescal::Effect::Blend::mode, mescal::Effect::Blend::linearLight);

        outputEffect = innerShadowSourceImageBlend;
    }

#if 0
    if (button.getToggleState() || shouldDrawButtonAsDown)
    {
        auto innerGlow = createInnerShadow(buttonImage, backgroundColour, (float)button.getHeight() * 0.25f, {});
        auto innerGlowBlend = mescal::Effect::Blend::create(mescal::Effect::Blend::multiply) << outputEffect << innerGlow;

        outputEffect = innerGlowBlend;
    }
#endif

    outputEffect->applyEffect(outputImage, {}, false);
    g.drawImageAt(outputImage, 0, 0);
}

void MescalLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    juce::Image sliderImage{ juce::Image::ARGB, label.getWidth(), label.getHeight(), true };

    {
        juce::Graphics imageG{ sliderImage };
        label.findColour(juce::Label::backgroundColourId);
        imageG.fillRect(sliderImage.getBounds());
    }

    auto innerShadowSize = 1.0f;
    innerShadow.configure(sliderImage,
        juce::Colours::black.withAlpha(0.25f),
        juce::AffineTransform::translation(innerShadowSize * 2.0f, innerShadowSize * 2.0f),
        juce::Colours::white,
        juce::AffineTransform::translation(-innerShadowSize, -innerShadowSize),
        innerShadowSize);

    juce::Image outputImage{ juce::Image::ARGB, label.getWidth(), label.getHeight(), true };
    innerShadow.getEffect()->applyEffect(outputImage, {}, false);
    g.drawImageAt(outputImage, 0, 0);

    if (!label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const juce::Font font(getLabelFont(label));

        g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
        g.setFont(font);

        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());

        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
            juce::jmax(1, (int)((float)textArea.getHeight() / font.getHeight())),
            label.getMinimumHorizontalScale());

        g.setColour(label.findColour(juce::Label::outlineColourId).withMultipliedAlpha(alpha));
    }
    else if (label.isEnabled())
    {
        g.setColour(label.findColour(juce::Label::outlineColourId));
    }

    //g.drawRect(label.getLocalBounds().reduced(2));
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

juce::Image MescalLookAndFeel::getImage(int index, juce::Rectangle<int> size)
{
    if (images.size() < index + 1)
    {
        images.emplace_back(juce::Image::ARGB, size.getWidth(), size.getHeight(), true);
        return images.back();
    }

    auto& image = images[index];
    if (image.getWidth() != size.getWidth() || image.getHeight() != size.getHeight())
    {
        image = juce::Image{ juce::Image::ARGB, size.getWidth(), size.getHeight(), true };
    }
    return image;
}

void MescalLookAndFeel::clear(juce::Graphics& g)
{
    g.setColour(juce::Colours::transparentBlack);
    g.getInternalContext().fillRect(g.getClipBounds(), true);
}

void MescalLookAndFeel::InnerShadow::configure(mescal::Effect::Input input, juce::Colour topColor, juce::AffineTransform topShadowTransform, juce::Colour bottomColor, juce::AffineTransform bottomShadowTransform, float shadowSize)
{
    flood->setPropertyValue(mescal::Effect::Flood::color, juce::Colours::blue);

    arithmeticComposite->setPropertyValue(mescal::Effect::ArithmeticComposite::coefficients, mescal::Vector4{ 0.0f, 1.0f, -1.0f, 0.0f });
    arithmeticComposite->setInput(0, flood);
    arithmeticComposite->setInput(1, input);

    upperShadow->setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, shadowSize);
    upperShadow->setPropertyValue(mescal::Effect::Shadow::color, topColor);
    upperShadow->setInput(0, arithmeticComposite);

    lowerShadow->setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, shadowSize);
    lowerShadow->setPropertyValue(mescal::Effect::Shadow::color, bottomColor);
    lowerShadow->setInput(0, arithmeticComposite);

    upperTransform->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, topShadowTransform);
    upperTransform->setInput(0, upperShadow);

    lowerTransform->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, bottomShadowTransform);
    lowerTransform->setInput(0, lowerShadow);

    blend->setPropertyValue(mescal::Effect::Blend::mode, mescal::Effect::Blend::multiply);
    blend->setInput(0, upperTransform);
    blend->setInput(1, lowerTransform);

    alphaMask->setInput(0, blend);
    alphaMask->setInput(1, input);
}

void MescalLookAndFeel::DropShadow::configure(mescal::Effect::Input input, juce::Colour shadowColor, float shadowSize, juce::AffineTransform transform)
{
    shadow->setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, shadowSize);
    shadow->setPropertyValue(mescal::Effect::Shadow::color, shadowColor);
    shadow->setInput(0, input);

    transformEffect->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, transform);
    transformEffect->setInput(0, shadow);
}

void MescalLookAndFeel::Glow::configure(mescal::Effect::Input input, juce::Colour glowColor, float glowSize)
{
    shadow->setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, glowSize);
    shadow->setPropertyValue(mescal::Effect::Shadow::color, glowColor);
    shadow->setInput(0, input);

    blur->setPropertyValue(mescal::Effect::GaussianBlur::standardDeviation, glowSize);
    blur->setInput(0, input);

    blend->setPropertyValue(mescal::Effect::Blend::mode, mescal::Effect::Blend::multiply);
    blend->setInput(0, input);
    blend->setInput(1, blur);

    composite->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);
    composite->setInput(0, input);
    composite->setInput(1, blur);
}
