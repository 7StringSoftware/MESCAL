#include <JuceHeader.h>
#include "EffectChainComponent.h"

EffectChainComponent::EffectChainComponent()
{
	viewport.setViewedComponent(&viewportContent, false);
	addAndMakeVisible(viewport);
}

EffectChainComponent::~EffectChainComponent()
{
}

void EffectChainComponent::setOutputEffect(mescal::Effect::Ptr outputEffect, int imageWidth, int imageHeight)
{
	viewportContent.buildEffectChainComponents(outputEffect, imageWidth, imageHeight);
}

void EffectChainComponent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::white);
}

void EffectChainComponent::resized()
{
	viewportContent.setBounds(viewportContent.getPreferredSize());
	viewport.setBounds(getLocalBounds());
}

juce::Rectangle<int> EffectChainComponent::ViewportContent::getPreferredSize()
{
	juce::Rectangle<int> bounds;

	for (int index = 0; index < getNumChildComponents(); ++index)
	{
		bounds = bounds.getUnion(getChildComponent(index)->getBounds());
	}

	return bounds;
}

EffectChainComponent::ViewportContent::ViewportContent()
{
}

void EffectChainComponent::ViewportContent::buildEffectChainComponentsRecursive(EffectChainComponent::ViewportContent* parent,
	NodeComponent* downstreamComponent,
	int downstreamInputIndex,
	mescal::Effect::Ptr effect,
	int depth,
	int& maxDepth)
{
	auto addConnection = [&](NodeComponent* inputNode, int inputIndex, NodeComponent* outputNode)
		{
			if (inputNode)
			{
				jassert(inputIndex < inputNode->numInputs);

				auto& connectionComponent = parent->connectionComponents.emplace_back(std::make_unique<ConnectionComponent>());

				parent->addAndMakeVisible(connectionComponent.get());

				connectionComponent->inputConnector = inputNode->inputConnectors[inputIndex].get();
				connectionComponent->outputConnector = &outputNode->outputConnector;

				inputNode->inputConnectors[inputIndex]->connection = connectionComponent.get();
				outputNode->outputConnector.connection = connectionComponent.get();
			}
		};

	depth += 1;
	maxDepth = juce::jmax(depth, maxDepth);

	auto effectComponent = parent->effectComponents.emplace_back(std::make_unique<EffectComponent>(effect)).get();
	parent->addAndMakeVisible(effectComponent);

	addConnection(downstreamComponent, downstreamInputIndex, effectComponent);

	for (int index = 0; index < effectComponent->numInputs; ++index)
	{
		auto const& input = effect->getInputs()[index];

		if (std::holds_alternative<mescal::Effect::Ptr>(input))
		{
			auto chainedEffect = std::get<mescal::Effect::Ptr>(input);
			buildEffectChainComponentsRecursive(parent, effectComponent, index, chainedEffect, depth, maxDepth);
		}
		else if (std::holds_alternative<juce::Image>(input))
		{
			if (auto image = std::get<juce::Image>(input); image.isValid())
			{
				auto& imageComponent = parent->inputImageComponents.emplace_back(std::make_unique<InputImageComponent>(image));
				maxDepth = juce::jmax(depth + 1, maxDepth);

				parent->addAndMakeVisible(imageComponent.get());

				addConnection(effectComponent, index, imageComponent.get());
			}
		}
	}
}

int EffectChainComponent::ViewportContent::positionEffectChainComponentsRecursive(NodeComponent* nodeComponent, int y, int depth)
{
	int size = 150;
	int gap = 50;
	int startY = y;
	int x = (size + gap) * (depth - 1);

	int inputIndex = 0;
	int terminalCount = 0;
	for (auto& inputConnector : nodeComponent->inputConnectors)
	{
		if (auto connection = inputConnector->connection)
		{
			if (connection->outputConnector)
			{
				terminalCount += positionEffectChainComponentsRecursive(&connection->outputConnector->nodeComponent, startY + inputIndex * (size + gap), depth - 1);
			}
		}

		++inputIndex;
	}

	terminalCount = juce::jmax(1, terminalCount);

	if (terminalCount > 1)
		y = startY + ((size + gap) * (terminalCount - 1) - size) / 2;
	nodeComponent->setBounds(x, y, size, size + NodeComponent::textHeight);

	return terminalCount;
}

void EffectChainComponent::ViewportContent::buildEffectChainComponents(mescal::Effect::Ptr newOutputEffect, int imageWidth, int imageHeight)
{
	outputEffect = newOutputEffect;
	if (!outputEffect)
		return;

	inputImageComponents.clear();
	effectComponents.clear();
	connectionComponents.clear();

	int maxDepth = 0;
	buildEffectChainComponentsRecursive(this, nullptr, -1, newOutputEffect, 0, maxDepth);

	positionEffectChainComponentsRecursive(effectComponents.front().get(), 0, maxDepth);

	for (auto& effectComponent : effectComponents)
	{
		effectComponent->image = juce::Image{ juce::Image::ARGB, imageWidth, imageHeight, true, juce::NativeImageType{} };
	}

	resized();
}

void EffectChainComponent::EffectComponent::mouseUp(juce::MouseEvent const&)
{
	if (isMouseOver(false))
	{
		showCallout();
	}
}

void EffectChainComponent::EffectComponent::showCallout()
{
	auto content = std::make_unique<EffectPropertyPanel>(effect);
	content->onPropertyChange = [this]()
		{
			getParentComponent()->repaint();
		};
	juce::CallOutBox::launchAsynchronously(std::move(content),
		getScreenBounds(),
		nullptr);
}

void EffectChainComponent::ViewportContent::resized()
{
	for (auto& connectionComponent : connectionComponents)
	{
		connectionComponent->setBounds(getLocalBounds());
	}
}

void EffectChainComponent::ViewportContent::paint(juce::Graphics&)
{
}

EffectChainComponent::EffectComponent::EffectComponent(mescal::Effect::Ptr effect_) :
	NodeComponent(effect_->getInputs().size()),
	effect(effect_)
{
	setName(effect_->getName());
}

void EffectChainComponent::EffectComponent::paint(juce::Graphics& g)
{
	NodeComponent::paint(g);

	effect->applyEffect(image, 1.0f, 1.0f, true);
	g.drawImage(image, getLocalBounds().withTrimmedBottom(textHeight).toFloat());

	g.setColour(juce::Colours::white.withAlpha(0.8f));
	auto r = getLocalBounds().removeFromBottom(textHeight);
	g.fillRect(r);
	g.setColour(juce::Colours::black.withAlpha(0.8f));
	g.setFont(juce::FontOptions{ textHeight * 0.43f, juce::Font::bold });
	g.drawMultiLineText(effect->getName(), 0, getHeight() - textHeight / 2, getWidth(), juce::Justification::centred);
}

void EffectChainComponent::EffectComponent::resized()
{
	NodeComponent::resized();
}

EffectChainComponent::InputImageComponent::InputImageComponent(juce::Image image_) :
	NodeComponent(0)
{
	setName("Image");

	image = image_;
	numOutputs = 1;
}

void EffectChainComponent::InputImageComponent::paint(juce::Graphics& g)
{
	g.drawImage(image, getLocalBounds().withTrimmedBottom(textHeight).toFloat());
}

EffectChainComponent::NodeComponent::NodeComponent(size_t numInputs_) :
	numInputs(numInputs_),
	outputConnector(*this, 0)
{
	addAndMakeVisible(outputConnector);

	for (int inputIndex = 0; inputIndex < numInputs_; ++inputIndex)
	{
		auto& inputConnector = inputConnectors.emplace_back(std::make_unique<InputConnectorComponent>(*this, inputIndex));
		addAndMakeVisible(inputConnector.get());
	}
}

void EffectChainComponent::NodeComponent::paint(juce::Graphics&)
{
}

void EffectChainComponent::NodeComponent::resized()
{
	int size = 20;
	outputConnector.setBounds(getWidth() - size, (getHeight() - size) / 2, size, size);

	int yGap = getHeight() / (int)(inputConnectors.size() + 1);
	int y = yGap - size / 2;
	for (auto& inputConnector : inputConnectors)
	{
		inputConnector->setBounds(0, y, size, size);
		y += yGap;
	}
}

EffectChainComponent::OutputConnectorComponent::OutputConnectorComponent(NodeComponent& nodeComponent_, int index_) :
	nodeComponent(nodeComponent_),
	index(index_)
{
}

void EffectChainComponent::OutputConnectorComponent::paint(juce::Graphics& g)
{
	g.setColour(juce::Colours::lightgrey);
	auto r = getLocalBounds().toFloat().reduced(6.0, 6.0f);
	g.drawEllipse(r, 4.0f);
}

EffectChainComponent::InputConnectorComponent::InputConnectorComponent(NodeComponent& nodeComponent_, int index_) :
	nodeComponent(nodeComponent_),
	index(index_)
{
}

void EffectChainComponent::InputConnectorComponent::paint(juce::Graphics& g)
{
	g.setColour(juce::Colours::lightgrey);
	auto r = getLocalBounds().toFloat().reduced(6.0, 6.0f);
	g.drawEllipse(r, 4.0f);
}

void EffectChainComponent::ConnectionComponent::paint(juce::Graphics& g)
{
	if (outputConnector && inputConnector)
	{
		g.setColour(juce::Colours::darkgrey);
		auto begin = getLocalPoint(outputConnector, outputConnector->getLocalBounds().getCentre());
		auto end = getLocalPoint(inputConnector, inputConnector->getLocalBounds().getCentre());
		juce::Line<float> line{ begin.toFloat(), end.toFloat() };
		g.drawArrow(line, 4.0f, 12.0f, 12.0f);
	}
}

bool EffectChainComponent::ConnectionComponent::hitTest(int, int)
{
	return false;
}
