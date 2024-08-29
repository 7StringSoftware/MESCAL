#include "EffectGraph.h"

EffectGraph::EffectGraph()
{
    //createMetallicKnobEffectGraph();
    paint3DButtonImages();
    create3DButtonEffectGraph();
}


void EffectGraph::paintMetallicKnobImage(float angle)
{
    if (sourceImages.size() == 0)
    {
        sourceImages.emplace_back(juce::Image{ juce::Image::ARGB, 1000, 1000, true });
    }

    auto sourceImage = sourceImages.front();
    juce::Graphics g{ sourceImage };

    g.setColour(juce::Colours::transparentBlack);
    g.getInternalContext().fillRect(sourceImage.getBounds(), true);

    auto center = sourceImage.getBounds().toFloat().getCentre();

    g.setColour(juce::Colours::black.withAlpha(0.2f));
    g.fillEllipse(sourceImage.getBounds().toFloat().withSizeKeepingCentre(800.0f, 800.0f));

    g.setColour(juce::Colours::black.withAlpha(0.6f));
    juce::Path p;
    p.addStar(center, 10, 360.0f, 400.0f, (float)angle);
    g.fillPath(p);

    juce::Line<float> line{ center, center.getPointOnCircumference(330.0f, (float)angle) };
    g.setColour(juce::Colours::black);
    g.drawLine(line, 30.0f);
    g.setColour(juce::Colours::white);
    g.drawLine(line, 20.0f);
}

void EffectGraph::createMetallicKnobEffectGraph()
{
    /*

        Build the effect graph demonstration

        This graph will take an image drawn with simple geometric objects (rectangles, stars, etc), and add shadow and lighting effects
        to create a pseudo-3D metallic knob.

            The final image will be a composite: the lighting effect output is layered on top of the original image, which in turn is layered on top
            the output of the shadow effect.

        Each layer in the composite is painted by a different chain of effects which are ultimately combined by compositing effects.

        To create the shadow, the source image is fed into a shadow effect. The shadow effect is then chained to a 2D affine transform effect
        to shift the shadow down and to the right.

        To create the lighting effect, the source image is fed into a Gaussian blur effect and then a spot specular lighting effect. The output
        of the spot specular lighting effect is fed into a composite effect.

        To further accentuate the metallic look, the output of that composite effect is combined with the original source image using an arithmetic
        composite effect.

        The final image is created by a composite effect that combines the	output of the 2D affine transform and the output of the arithmetic
        composite effect.

        Note that there's no need to retain any of the upstream effects; you only need to keep the final output effect. Effects are reference-counted,
        so the entire graph will be retained until no longer needed.

    */
    auto& sourceImage = sourceImages.front();

    auto blurEffect = new mescal::Effect{ mescal::Effect::Type::gaussianBlur };
    blurEffect->setPropertyValue(mescal::Effect::GaussianBlur::standardDeviation, 10.0f);
    blurEffect->setInput(0, sourceImage);

    auto lightingEffect = new mescal::Effect{ mescal::Effect::Type::spotSpecularLighting };
    lightingEffect->setPropertyValue(mescal::Effect::SpotSpecularLighting::lightPosition, mescal::Vector3{ sourceImage.getWidth() * 0.1f, sourceImage.getHeight() * 0.1f, 250.0f });
    lightingEffect->setPropertyValue(mescal::Effect::SpotSpecularLighting::pointsAt, mescal::Vector3{ sourceImage.getWidth() * 0.5f, sourceImage.getHeight() * 0.5f, 0.0f });
    lightingEffect->setPropertyValue(mescal::Effect::SpotSpecularLighting::surfaceScale, 30.0f);
    lightingEffect->setPropertyValue(mescal::Effect::SpotSpecularLighting::color, mescal::Vector3{ 0.9f, 0.95f, 1.0f });
    lightingEffect->setInput(0, blurEffect);

    auto shadowEffect = new mescal::Effect{ mescal::Effect::Type::shadow };
    shadowEffect->setInput(0, sourceImage);
    shadowEffect->setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, 10.0f);
    shadowEffect->setPropertyValue(mescal::Effect::Shadow::color, mescal::colourToVector4(juce::Colours::black.withAlpha(0.5f)));

    auto affineTransformEffect = new mescal::Effect{ mescal::Effect::Type::affineTransform2D };
    affineTransformEffect->setInput(0, shadowEffect);
    affineTransformEffect->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, juce::AffineTransform::translation(25.0f, 25.0f));

    auto compositeEffect = new mescal::Effect{ mescal::Effect::Type::composite };
    compositeEffect->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::destinationAtop);
    compositeEffect->setInput(0, lightingEffect);
    compositeEffect->setInput(1, sourceImage);

    auto arithmeticCompositeEffect = new mescal::Effect{ mescal::Effect::Type::arithmeticComposite };
    arithmeticCompositeEffect->setPropertyValue(mescal::Effect::ArithmeticComposite::coefficients, mescal::Vector4{ 0.0f, 2.0f, 1.0f, 0.0f });
    arithmeticCompositeEffect->setInput(0, compositeEffect);
    arithmeticCompositeEffect->setInput(1, sourceImage);

    auto shadowCompositeEffect = new mescal::Effect{ mescal::Effect::Type::composite };
    shadowCompositeEffect->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceOver);
    shadowCompositeEffect->setInput(0, affineTransformEffect);
    shadowCompositeEffect->setInput(1, arithmeticCompositeEffect);

    outputEffect = shadowCompositeEffect;
}


void EffectGraph::paint3DButtonImages()
{
    /*

    background gradient: #bdc1cc to #e7ebee linearly vertically

    ellipse 185px: 50% gray (#959595), overlaid with gradient #d7dae1 to #cad0d8, inner shadow #c5c8ce, drop shadow #dde0e7
    ellipse 150px: #5d5d5d, overlaid with a gradient #b0b4bf to #cdd0d7 to #dee1e6 to #f5f6fa

    ellipse 85px: #2c2c2c, overlaid with color #d4d8e3, drop shadow #e4f0ff, inner shadow #b5b5b5

    ellipse 150px copy + clear layer style, then: drop shadow with black

    blank layer above ellipse 185px, clipped to ellipse 185px: upper right quadrant filled with 50%gray (#959

    */

    sourceImages.emplace_back(juce::Image{ juce::Image::ARGB, 300, 300, true });
    sourceImages.emplace_back(juce::Image{ juce::Image::ARGB, 300, 300, true });
    sourceImages.emplace_back(juce::Image{ juce::Image::ARGB, 300, 300, true });

    auto bottomImage = sourceImages.front();
    auto middleEllipseImage = sourceImages[1];

    {
        juce::Graphics g{ middleEllipseImage };

        g.setColour(juce::Colour{ 0xff5d5d5d });
        auto middleEllipseBounds = middleEllipseImage.getBounds().toFloat().withSizeKeepingCentre(150.0f, 150.0f);
        g.fillEllipse(middleEllipseBounds);

        {
            juce::Graphics::ScopedSaveState saveState{ g };
            auto gradient = juce::ColourGradient::vertical(juce::Colour{ 0xffb0b4bf },
                middleEllipseBounds.getBottom(),
                juce::Colour{ 0xfff5f6fa },
                middleEllipseBounds.getY());
            gradient.addColour(0.33f, juce::Colour{ 0xffcdd0d7 });
            gradient.addColour(0.66f, juce::Colour{ 0xffdee1e6 });

            g.setGradientFill(gradient);
            g.fillEllipse(middleEllipseBounds);
        }
    }

    {
        juce::Graphics g{ bottomImage };

        {
            auto gradient = juce::ColourGradient::vertical(juce::Colour{ 0xffbdc1cc }, 0, juce::Colour{ 0xffe7ebee }, (float)bottomImage.getHeight());
            g.setGradientFill(gradient);
            g.fillRect(bottomImage.getBounds());
        }

        g.setColour(juce::Colour{ 0xff959595 });
        g.fillEllipse(bottomImage.getBounds().toFloat().withSizeKeepingCentre(185.0f, 185.0f));
    }

}

void EffectGraph::create3DButtonEffectGraph()
{
    auto bottomImage = sourceImages.front();
    auto middleEllipseImage = sourceImages[1];

    auto shadowEffect = new mescal::Effect{ mescal::Effect::Type::shadow };
    shadowEffect->setInput(0, middleEllipseImage);
    shadowEffect->setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, 10.0f);
    shadowEffect->setPropertyValue(mescal::Effect::Shadow::color, mescal::colourToVector4(juce::Colours::black));

    auto affineTransformEffect = new mescal::Effect{ mescal::Effect::Type::affineTransform2D };
    affineTransformEffect->setInput(0, shadowEffect);
    affineTransformEffect->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, juce::AffineTransform::translation(0.0f, 12.0f));

    auto shadowCompositeEffect = new mescal::Effect{ mescal::Effect::Type::composite };
    shadowCompositeEffect->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceOver);
    shadowCompositeEffect->setInput(0, affineTransformEffect);
    shadowCompositeEffect->setInput(1, middleEllipseImage);


    auto floodEffect = new mescal::Effect{ mescal::Effect::Type::flood };
    floodEffect->setPropertyValue(mescal::Effect::Flood::color, juce::Colours::blue);

    auto arithmeticComposite = new mescal::Effect{ mescal::Effect::Type::arithmeticComposite };
    arithmeticComposite->setPropertyValue(mescal::Effect::ArithmeticComposite::coefficients, mescal::Vector4{ 0.0f, 1.0f, -1.0f, 0.0f });
    arithmeticComposite->setInput(0, floodEffect);
    arithmeticComposite->setInput(1, middleEllipseImage);

    auto innerShadow = new mescal::Effect{ mescal::Effect::Type::shadow };
    innerShadow->setInput(0, arithmeticComposite);
    innerShadow->setPropertyValue(mescal::Effect::Shadow::blurStandardDeviation, 10.0f);
    innerShadow->setPropertyValue(mescal::Effect::Shadow::color, mescal::colourToVector4(juce::Colours::black));

    auto alphaMaskEffect = new mescal::Effect{ mescal::Effect::Type::alphaMask };
    alphaMaskEffect->setInput(0, innerShadow);
    alphaMaskEffect->setInput(1, middleEllipseImage);

    auto alphaMaskTransform = new mescal::Effect{ mescal::Effect::Type::affineTransform2D };
    alphaMaskTransform->setInput(0, alphaMaskEffect);
    alphaMaskTransform->setPropertyValue(mescal::Effect::AffineTransform2D::transformMatrix, juce::AffineTransform::scale(0.8f).translated(30.0f, 30.0f));

    auto compositeEffect = new mescal::Effect{ mescal::Effect::Type::composite };
    compositeEffect->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);
    compositeEffect->setInput(0, bottomImage);
    compositeEffect->setInput(1, shadowCompositeEffect);

    auto innerShadowComposite = new mescal::Effect{ mescal::Effect::Type::composite };
    innerShadowComposite->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceAtop);
    innerShadowComposite->setInput(0, compositeEffect);
    innerShadowComposite->setInput(1, alphaMaskTransform);

    outputEffect = innerShadowComposite;
}
