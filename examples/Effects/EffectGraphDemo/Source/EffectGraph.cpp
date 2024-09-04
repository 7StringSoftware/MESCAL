#include "EffectGraph.h"

EffectGraph::EffectGraph()
{
}

void EffectGraph::createMetallicKnobGraph()
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

void EffectGraph::createSliderGraph()
{
    auto innerShadow = createInnerShadow(sourceImage, juce::Colours::black, 30.0f, {});

    auto blend = new mescal::Effect{ mescal::Effect::Type::blend };
    blend->setPropertyValue(mescal::Effect::Blend::mode, mescal::Effect::Blend::multiply);
    blend->setInput(0, innerShadow);
    blend->setInput(1, sourceImage);

    outputEffect = blend;
}

mescal::Effect::Ptr EffectGraph::addShadow(juce::Image const& shadowSourceImage, juce::Colour const& shadowColor, float shadowSize, juce::AffineTransform transform)
{
    auto shadow = new mescal::Effect{ mescal::Effect::Type::shadow };
    shadow->setInput(0, shadowSourceImage);
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

mescal::Effect::Ptr EffectGraph::createInnerShadow(juce::Image const& shadowSourceImage, juce::Colour const& shadowColor, float shadowSize, juce::AffineTransform transform)
{
#if 0
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
#endif

    return nullptr;
}
