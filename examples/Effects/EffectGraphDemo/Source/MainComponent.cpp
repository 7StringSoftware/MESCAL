#include "MainComponent.h"

MainComponent::MainComponent()
{
	setOpaque(true);

	effectGraphComponent.setOutputEffect(effectGraph.outputEffect, effectGraph.sourceImage.getWidth(), effectGraph.sourceImage.getHeight());
	addAndMakeVisible(effectGraphComponent);

	setBounds(effectGraphComponent.getPreferredSize());
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint(juce::Graphics& g)
{
	paintSourceImage();
	g.fillAll(juce::Colours::white);
}

void MainComponent::resized()
{
	effectGraphComponent.setBounds(getLocalBounds());
}

void MainComponent::animate()
{
	auto now = juce::Time::getMillisecondCounterHiRes();
	auto elapsedMsec = now - lastMsec;
	lastMsec = now;

	angle += elapsedMsec * 0.001 * juce::MathConstants<double>::twoPi * 0.25f;
	repaint();
}

void MainComponent::paintSourceImage()
{
	juce::Graphics g{ effectGraph.sourceImage };

	g.setColour(juce::Colours::transparentBlack);
	g.getInternalContext().fillRect(effectGraph.sourceImage.getBounds(), true);

	auto center = effectGraph.sourceImage.getBounds().toFloat().getCentre();

	g.setColour(juce::Colours::black.withAlpha(0.2f));
	g.fillEllipse(effectGraph.sourceImage.getBounds().toFloat().withSizeKeepingCentre(800.0f, 800.0f));

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
