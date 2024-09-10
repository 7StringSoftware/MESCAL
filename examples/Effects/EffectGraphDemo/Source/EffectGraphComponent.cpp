#include <JuceHeader.h>
#include "EffectGraphComponent.h"

EffectGraphComponent::EffectGraphComponent()
{
	viewport.setViewedComponent(&viewportContent, false);
	addAndMakeVisible(viewport);


    setTransform(juce::AffineTransform::scale(1.0f));
}

EffectGraphComponent::~EffectGraphComponent()
{
}

static void setCallbackRecursive(mescal::Effect::Ptr effect, std::function<void()> callback)
{
    effect->onPropertyChange = [callback](mescal::Effect*, int, mescal::Effect::PropertyValue)
    {
        if (callback)
        {
            callback();
        }
    };

    for (auto const& input : effect->getInputs())
    {
        if (std::holds_alternative<mescal::Effect::Ptr>(input))
        {
            setCallbackRecursive(std::get<mescal::Effect::Ptr>(input), callback);
        }
    }
}

void EffectGraphComponent::setOutputEffect(mescal::Effect::Ptr outputEffect, int imageWidth, int imageHeight)
{
	viewportContent.buildEffectGraphComponents(outputEffect, imageWidth, imageHeight);

    setCallbackRecursive(outputEffect, [this]()
        {
            if (onPropertyChange)
            {
                onPropertyChange();
            }
        });
}

void EffectGraphComponent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::white);
}

void EffectGraphComponent::resized()
{
    auto contentSize = getPreferredSize().getUnion(getLocalBounds());
	viewportContent.setBounds(contentSize);
	viewport.setBounds(getLocalBounds());
}

juce::Rectangle<int> EffectGraphComponent::getPreferredSize()
{
    return viewportContent.getPreferredSize();
}

juce::Rectangle<int> EffectGraphComponent::ViewportContent::getPreferredSize()
{
	juce::Rectangle<int> bounds;

	for (int index = 0; index < getNumChildComponents(); ++index)
	{
		bounds = bounds.getUnion(getChildComponent(index)->getBounds());
	}

	return bounds;
}

EffectGraphComponent::ViewportContent::ViewportContent()
{
}

void EffectGraphComponent::ViewportContent::buildEffectGraphComponentsRecursive(EffectGraphComponent::ViewportContent* parent,
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
			buildEffectGraphComponentsRecursive(parent, effectComponent, index, chainedEffect, depth, maxDepth);
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

void EffectGraphComponent::ViewportContent::positionEffectGraphComponentsRecursive(NodeComponent* nodeComponent, int& y, int depth, std::vector<std::vector<juce::Component*>>& columns,
    int& sequence)
{
	int size = 150;
	int gap = 80;
	int top = y;
	int x = (size + gap) * (depth - 1);

	auto column = std::abs(depth);
	columns[column].push_back(nodeComponent);

	int inputIndex = 0;
	int inputY = y;
	int numConnectedInputs = 0;
	for (auto& inputConnector : nodeComponent->inputConnectors)
	{
		if (auto connection = inputConnector->connection)
		{
			if (connection->outputConnector)
			{
				positionEffectGraphComponentsRecursive(&connection->outputConnector->nodeComponent, inputY, depth - 1, columns, sequence);
				inputY += size + gap;
				++numConnectedInputs;
			}
		}

		++inputIndex;
	}

	y = top;
	if (numConnectedInputs > 1)
	{
		auto height = inputY - top;
		y = top + (height - size) / 2;
	}

	nodeComponent->setBounds(x, y, size, size + NodeComponent::textHeight);
}

void EffectGraphComponent::ViewportContent::buildEffectGraphComponents(mescal::Effect::Ptr newOutputEffect, int imageWidth, int imageHeight)
{
	outputEffect = newOutputEffect;
	if (!outputEffect)
		return;

	inputImageComponents.clear();
	effectComponents.clear();
	connectionComponents.clear();

	int maxDepth = 0;
	buildEffectGraphComponentsRecursive(this, nullptr, -1, newOutputEffect, 0, maxDepth);

	int y = 0;
	std::vector<std::vector<juce::Component*>> columns{ (size_t)maxDepth + 1 };
	int sequence = 0;
	positionEffectGraphComponentsRecursive(effectComponents.front().get(), y, maxDepth, columns, sequence);

	for (auto& effectComponent : effectComponents)
	{
		effectComponent->image = juce::Image{ juce::Image::ARGB, imageWidth, imageHeight, true, juce::NativeImageType{} };
	}

	for (auto& column : columns)
	{
		int bottom = 0;
		for (auto component : column)
		{
			if (component->getY() < bottom)
			{
				component->setTopLeftPosition(component->getX(), bottom + 10);
			}

			bottom = component->getBottom();
		}
	}

	resized();
}

void EffectGraphComponent::EffectComponent::mouseUp(juce::MouseEvent const&)
{
	if (isMouseOver(false))
	{
		showCallout();
	}
}

void EffectGraphComponent::EffectComponent::showCallout()
{
	auto content = std::make_unique<EffectPropertyPanel>(effect);
	content->onPropertyChange = [this]()
		{
			getParentComponent()->repaint();
		};
	juce::CallOutBox::launchAsynchronously(std::move(content),
		getBoundsInParent(),
		getParentComponent());
}

void EffectGraphComponent::ViewportContent::resized()
{
	for (auto& connectionComponent : connectionComponents)
	{
		connectionComponent->setBounds(getLocalBounds());
	}
}

void EffectGraphComponent::ViewportContent::paint(juce::Graphics&)
{
}

EffectGraphComponent::EffectComponent::EffectComponent(mescal::Effect::Ptr effect_) :
	NodeComponent(effect_->getInputs().size()),
	effect(effect_)
{
	setName(effect_->getName());
}

void EffectGraphComponent::EffectComponent::paint(juce::Graphics& g)
{
	NodeComponent::paint(g);

	//
	// Process the entire effect graph. Note that only the final effect needs to be applied.
	//
	effect->applyEffect(image, {}, true);
	g.drawImage(image, getLocalBounds().withTrimmedBottom(textHeight).toFloat());

	g.setColour(juce::Colours::white.withAlpha(0.8f));
	auto r = getLocalBounds().removeFromBottom(textHeight);
	g.fillRect(r);
	g.setColour(juce::Colours::black.withAlpha(0.8f));
	g.setFont(juce::FontOptions{ textHeight * 0.43f, juce::Font::bold });
	g.drawMultiLineText(effect->getName(), 0, getHeight() - textHeight / 2, getWidth(), juce::Justification::centred);
}

void EffectGraphComponent::EffectComponent::resized()
{
	NodeComponent::resized();
}

EffectGraphComponent::InputImageComponent::InputImageComponent(juce::Image image_) :
	NodeComponent(0)
{
	setName("Image");

	image = image_;
	numOutputs = 1;
}

void EffectGraphComponent::InputImageComponent::paint(juce::Graphics& g)
{
	g.drawImage(image, getLocalBounds().withTrimmedBottom(textHeight).toFloat());
}

EffectGraphComponent::NodeComponent::NodeComponent(size_t numInputs_) :
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

void EffectGraphComponent::NodeComponent::resized()
{
	int size = 20;
	outputConnector.setBounds(getWidth() - size, (getHeight() - size - textHeight) / 2, size, size);

	int yGap = (getHeight() - textHeight) / (int)(inputConnectors.size() + 1);
	int y = yGap - size / 2;
	for (auto& inputConnector : inputConnectors)
	{
		inputConnector->setBounds(0, y, size, size);
		y += yGap;
	}
}

EffectGraphComponent::OutputConnectorComponent::OutputConnectorComponent(NodeComponent& nodeComponent_, int index_) :
	nodeComponent(nodeComponent_),
	index(index_)
{
}

void EffectGraphComponent::OutputConnectorComponent::paint(juce::Graphics& g)
{
	g.setColour(juce::Colours::lightgrey);
	auto r = getLocalBounds().toFloat().reduced(6.0, 6.0f);
	g.drawEllipse(r, 4.0f);
}

EffectGraphComponent::InputConnectorComponent::InputConnectorComponent(NodeComponent& nodeComponent_, int index_) :
	nodeComponent(nodeComponent_),
	index(index_)
{
}

void EffectGraphComponent::InputConnectorComponent::paint(juce::Graphics& g)
{
	g.setColour(juce::Colours::lightgrey);
	auto r = getLocalBounds().toFloat().reduced(6.0, 6.0f);
	g.drawEllipse(r, 4.0f);
}

void EffectGraphComponent::ConnectionComponent::paint(juce::Graphics& g)
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

bool EffectGraphComponent::ConnectionComponent::hitTest(int, int)
{
	return false;
}
