#include "MainComponent.h"
#include "PropertyComponents.h"

MainComponent::MainComponent()
{
    //sourceImages.emplace_back(juce::ImageFileFormat::loadFrom(BinaryData::VanGoghstarry_night_jpg, BinaryData::VanGoghstarry_night_jpgSize));
    sourceImages.emplace_back(juce::Image{ juce::Image::ARGB, 1000, 1000, true });
#if 1
    {
        juce::Graphics g{ sourceImages.back() };
        g.setColour(juce::Colours::transparentBlack);
        g.getInternalContext().fillRect(sourceImages.back().getBounds(), true);

        g.fillCheckerBoard(sourceImages.back().getBounds().toFloat(), 50.0f, 50.0f, juce::Colours::transparentBlack, juce::Colours::lightgrey.withAlpha(0.5f));
    }
#endif

    juce::Rectangle<int> maxBounds;
    for (auto const& sourceImage : sourceImages)
    {
        maxBounds = maxBounds.getUnion(sourceImage.getBounds());
    }

    outputImage = juce::Image{ juce::Image::ARGB, maxBounds.getWidth(), maxBounds.getHeight(), true };

    addAndMakeVisible(propertyPanel);

    setSize(1024, 1024);
    setRepaintsOnMouseActivity(true);

    updateEffectType();

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
    int height = getHeight() / (int)sourceImages.size();
    auto sourceImageArea = imageArea.withWidth(imageArea.getWidth() / 2);
    for (auto& image : sourceImages)
    {
        g.drawImage(image, sourceImageArea.removeFromTop(height).toFloat());
    }

    if (outputImage.isValid())
    {
        applyEffect();

        juce::Rectangle<int> outputImageArea{ sourceImageArea.getRight(), 0, propertyPanel.getX() - sourceImageArea.getRight(), getHeight() };
        //g.drawImage(sourceImages.front(), outputImageArea.toFloat());
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
    propertyValueComponents.clear();

    //
    // Effect selection
    //

    //
    // Effect-specific controls
    //
    juce::Array<juce::PropertyComponent*> sectionComponents;

    for (int propertyIndex = 0; propertyIndex < effect->getNumProperties(); ++propertyIndex)
    {
        auto propertyInfo = effect->getPropertyInfo(propertyIndex);

        if (propertyInfo.range.has_value())
        {
            switch (propertyInfo.range->index())
            {
            case 0: // Range<int>
            {
                break;
            }

            case 1: // Range<float>
            {
                auto range = std::get<juce::Range<float>>(*propertyInfo.range);
                auto propertyComponent = new MultiSliderPropertyComponent{ propertyInfo.name,
                    propertyIndex,
                    propertyInfo.defaultValue,
                    { propertyInfo.name },
                    range };
                propertyValueComponents.emplace_back(propertyComponent);
                sectionComponents.add(propertyComponent);;
                break;
            }

            case 2: // StringArray
            {
                auto strings = std::get<juce::StringArray>(*propertyInfo.range);
                auto propertyComponent = new StringArrayPropertyComponent{ propertyInfo.name, propertyIndex, propertyInfo.defaultValue, strings };
                propertyValueComponents.emplace_back(propertyComponent);
                sectionComponents.add(propertyComponent);
                break;
            }

            default:
            {
                jassertfalse;
                break;
            }
            }

            continue;
        }

        juce::StringArray sliderNames;
        juce::Range<float> sliderRange{ -1000.0f, 1000.0f };
        if (std::holds_alternative<mescal::Point3D>(propertyInfo.defaultValue))
        {
            sliderNames = { "x", "y", "z" };
        }
        else if (std::holds_alternative<mescal::Vector3>(propertyInfo.defaultValue))
        {
            sliderNames = { "0", "1", "2" };
        }
        else if (std::holds_alternative<mescal::RGBColor>(propertyInfo.defaultValue))
        {
            sliderNames = { "R", "G", "B" };
        }
        else if (std::holds_alternative<juce::Colour>(propertyInfo.defaultValue))
        {
            sliderNames = { "R", "G", "B", "A" };
        }
        else if (std::holds_alternative<juce::Point<float>>(propertyInfo.defaultValue))
        {
            sliderNames = { "x", "y" };
        }
        else
        {
            jassertfalse;
        }

        if (sliderNames.size() > 0)
        {
            auto propertyComponent = std::make_unique<MultiSliderPropertyComponent>(propertyInfo.name,
                propertyIndex,
                propertyInfo.defaultValue,
                sliderNames,
                sliderRange);
            propertyValueComponents.emplace_back(juce::Component::SafePointer<EffectPropertyValueComponent>{ propertyComponent.get() });
            sectionComponents.add(propertyComponent.release());
        }

    }

    propertyPanel.addSection(effect->getName(), sectionComponents);

    for (auto& c : propertyValueComponents)
    {
        c->onChange = [this]()
            {
                applyEffect();
                repaint();
            };
    }
}

void MainComponent::updateEffectType()
{
    buildPropertyPanel();
    applyEffect();
    repaint();
}

void MainComponent::applyEffect()
{
    for (auto& c : propertyValueComponents)
    {
        effect->setPropertyValue(c->propertyIndex, c->getPropertyValue());
    }

    effect->applyEffect(sourceImages.front(), outputImage, 1.0f, 1.0f, true);
}
