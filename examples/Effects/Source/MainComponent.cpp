#include "MainComponent.h"

MainComponent::MainComponent()
{
    sourceImages.emplace_back(juce::ImageFileFormat::loadFrom(BinaryData::VanGoghstarry_night_jpg, BinaryData::VanGoghstarry_night_jpgSize));

    juce::Rectangle<int> maxBounds;
    for (auto const& sourceImage : sourceImages)
    {
        maxBounds = maxBounds.getUnion(sourceImage.getBounds());
    }

    outputImage = juce::Image{ juce::Image::ARGB, maxBounds.getWidth(), maxBounds.getHeight(), true };

    addAndMakeVisible(propertyPanel);

    setSize(1024, 1024);
    setRepaintsOnMouseActivity(true);

    updateEffect();

    setOpaque(true);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    if (sourceImages.size() == 0)
        return;

    juce::Rectangle<int> imageArea{ 0, 0, propertyPanel.getX(), getHeight() };
    int height = getHeight() / sourceImages.size();
    auto sourceImageArea = imageArea.withWidth(imageArea.getWidth() / 2);
    for (auto& image : sourceImages)
    {
        g.drawImage(image, sourceImageArea.removeFromTop(height).toFloat());
    }

    if (outputImage.isValid())
    {
        juce::Rectangle<int> outputImageArea{ sourceImageArea.getRight(), 0, propertyPanel.getX() - sourceImageArea.getRight(), getHeight() };
        g.drawImage(outputImage, outputImageArea.toFloat());
    }
}

void MainComponent::resized()
{
    propertyPanel.setBounds(getLocalBounds().removeFromRight(300).reduced(10));
}

void MainComponent::buildPropertyPanel()
{
    propertyPanel.clear();
    valueMap.clear();

    juce::Array<juce::PropertyComponent*> propertyComponents;

    for (int propertyIndex = 0; propertyIndex < effect->getNumProperties(); ++propertyIndex)
    {
        auto propertyInfo = effect->getPropertyInfo(propertyIndex);

        if (!propertyInfo.range.has_value())
            continue;

        switch (propertyInfo.range->index())
        {
        case 0: // Range<int>
        {
            break;
        }

        case 1: // Range<float>
        {
            auto range = std::get<juce::Range<float>>(*propertyInfo.range);
            valueMap[propertyIndex] = std::get<float>(propertyInfo.defaultValue);
            propertyComponents.add(new juce::SliderPropertyComponent{ valueMap[propertyIndex], propertyInfo.name, range.getStart(), range.getEnd(), range.getLength() * 0.001f });
            break;
        }

        case 2: // StringArray
        {
            auto strings = std::get<juce::StringArray>(*propertyInfo.range);
            valueMap[propertyIndex] = std::get<int>(propertyInfo.defaultValue);

            juce::Array<juce::var> varArray;
            for (auto const& string : strings)
            {
                varArray.add(varArray.size());
            }
            propertyComponents.add(new juce::ChoicePropertyComponent{ valueMap[propertyIndex], propertyInfo.name, strings, varArray });
            break;
        }

        default:
        {
            valueMap[propertyIndex] = propertyInfo.name;;
            propertyComponents.add(new juce::TextPropertyComponent{ valueMap[propertyIndex], propertyInfo.name, propertyInfo.name.length() , false });
            break;
        }
        }
    }

    propertyPanel.addSection(effect->getName(), propertyComponents);
}

void MainComponent::updateEffect()
{
    buildPropertyPanel();
    applyEffect();
    repaint();
}

void MainComponent::applyEffect()
{
    effect->applyEffect(sourceImages.front(), outputImage, 1.0f, 1.0f, true);
}
