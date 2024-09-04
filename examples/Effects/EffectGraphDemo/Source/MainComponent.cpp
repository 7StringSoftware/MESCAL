#include "MainComponent.h"

MainComponent::MainComponent()
{
	setOpaque(true);

    paintSourceImage();
    effectGraph.createSliderGraph();
    effectGraphComponent.setOutputEffect(effectGraph.outputEffect, effectGraph.sourceImage.getWidth(), effectGraph.sourceImage.getHeight());
	addAndMakeVisible(effectGraphComponent);

	setBounds(effectGraphComponent.getPreferredSize());
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint(juce::Graphics& g)
{
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
    //effectGraph.paintMetallicKnobImage((float)angle);

	repaint();
}
