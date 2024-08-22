#include "EffectGraph.h"

EffectGraph::EffectGraph()
{
	auto blurEffect = new mescal::Effect{ mescal::Effect::Type::gaussianBlur };
	blurEffect->setPropertyValue(mescal::Effect::GaussianBlur::standardDeviation, 10.0f);
	blurEffect->setInput(0, sourceImage);

	auto lightingEffect = new mescal::Effect{ mescal::Effect::Type::spotSpecularLighting };
	lightingEffect->setPropertyValue(mescal::Effect::SpotSpecular::lightPosition, mescal::Vector3{ sourceImage.getWidth() * 0.1f, sourceImage.getHeight() * 0.1f, 250.0f });
	lightingEffect->setPropertyValue(mescal::Effect::SpotSpecular::pointsAt, mescal::Vector3{ sourceImage.getWidth() * 0.5f, sourceImage.getHeight() * 0.5f, 0.0f });
	lightingEffect->setPropertyValue(mescal::Effect::SpotSpecular::surfaceScale, 30.0f);
	lightingEffect->setPropertyValue(mescal::Effect::SpotSpecular::color, mescal::Vector3{ 0.9f, 0.95f, 1.0f });
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