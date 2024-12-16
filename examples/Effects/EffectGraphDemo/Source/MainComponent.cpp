#include "MainComponent.h"

MainComponent::MainComponent()
{
    setOpaque(true);

    auto& effectGraph = buttonHolder.effectButton.effectGraph;
    effectGraph.paint3DButtonImages();
    effectGraph.create3DButtonEffectGraph();

    //effectGraph.paintSlider();
    //effectGraph.createSliderEffectGraph();

//     effectGraph.paintMetallicKnobImage(0.8f);
//     effectGraph.createMetallicKnobEffectGraph();
//
//     effectGraphComponent.setOutputEffect(effectGraph.outputEffect, effectGraph.sourceImages.front().getWidth(), effectGraph.sourceImages.front().getHeight());
// 	addAndMakeVisible(effectGraphComponent);

    //effectGraph.buildComponentEffectGraph();
    //addAndMakeVisible(widgetsDemo);

    effectGraphComponent = std::make_unique<EffectGraphComponent>();
    addAndMakeVisible(effectGraphComponent.get());
    effectGraphComponent->onEffectPropertyChange = [this]()
        {
            //widgetsDemo.repaint();
        };

    effectGraphComponent->setOutputEffect(effectGraph.outputEffect);

    auto area = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea.reduced(40);
    centreWithSize(area.getWidth(), area.getHeight());

    imageEffectFilter.onApplyEffect = [this](juce::Image const& image)
        {
            snapshot = image;
            effectGraphComponent->setOutputEffect(buttonHolder.effectButton.effectGraph.outputEffect);
            effectGraphComponent->repaint();
        };

    addAndMakeVisible(buttonHolder);

    //imageEffectFilter.setEffect(effectGraph.outputEffect);
    //widgetsDemo.setComponentEffect(&imageEffectFilter);
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
    auto r = getLocalBounds();
    buttonHolder.setBounds(r.removeFromLeft(getWidth() / 3));
    effectGraphComponent->setBounds(r);
}

void MainComponent::animate()
{
#if 0
    auto now = juce::Time::getMillisecondCounterHiRes();
    auto elapsedMsec = now - lastMsec;
    lastMsec = now;

    angle += elapsedMsec * 0.001 * juce::MathConstants<double>::twoPi * 0.25f;
    //effectGraph.paintMetallicKnobImage((float)angle);

    repaint();
#endif
}

void DemoEffectFilter::applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha)
{
    if (onApplyEffect)
    {
        onApplyEffect(sourceImage);
    }

    MescalImageEffectFilter::applyEffect(sourceImage, destContext, scaleFactor, alpha);
}
