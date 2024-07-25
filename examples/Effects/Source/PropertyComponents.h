#pragma once

class EffectPropertyValueComponent : public juce::PropertyComponent
{
public:
    EffectPropertyValueComponent(juce::String propertyName_, size_t propertyIndex_, mescal::Effect::PropertyValue propertyValue_) :
        PropertyComponent(propertyName_),
        propertyIndex(propertyIndex_),
        propertyValue(propertyValue_)
    {
    }

    size_t const propertyIndex;
    mescal::Effect::PropertyValue propertyValue;
    std::function<void()> onChange;

    virtual mescal::Effect::PropertyValue getPropertyValue()
    {
        return propertyValue;
    }
};

class MultiSliderPropertyComponent : public EffectPropertyValueComponent
{
public:
    MultiSliderPropertyComponent(const juce::String& propertyName_,
        size_t propertyIndex_,
        mescal::Effect::PropertyValue propertyValue_,
        juce::StringArray const& sliderNames_,
        juce::Range<float> sliderRange_) :
        EffectPropertyValueComponent(propertyName_, propertyIndex_, propertyValue_)
    {
        for (auto const& name : sliderNames_)
        {
            auto slider = std::make_unique<juce::Slider>(juce::Slider::LinearBar, juce::Slider::TextBoxLeft);
            slider->setRange(sliderRange_.getStart(), sliderRange_.getEnd(), 0.01);
            slider->setValue(0.0f);
            addAndMakeVisible(slider.get());
            slider->onValueChange = [this]
                {
                    sliderChanged();
                };

            sliders.push_back(std::move(slider));
        }

        setPreferredHeight(sliderNames_.size() * 30);
    }

    void resized() override
    {
        auto contentBounds = getLookAndFeel().getPropertyComponentContentPosition(*this);
        for (auto& slider : sliders)
        {
            slider->setBounds(contentBounds.removeFromTop(getHeight() / (int)sliders.size()));
        }
    }

    void refresh() override
    {
    }

    void sliderChanged()
    {
        if (std::holds_alternative<int>(propertyValue))
        {
            propertyValue = juce::roundToInt(sliders.front()->getValue());
        }
        else if (std::holds_alternative<float>(propertyValue))
        {
            propertyValue = (float)sliders[0]->getValue();
        }
        else if (std::holds_alternative<mescal::Point3D>(propertyValue))
        {
            propertyValue = mescal::Point3D{ (float)sliders[0]->getValue(), (float)sliders[1]->getValue(), (float)sliders[2]->getValue() };
        }
        else if (std::holds_alternative<mescal::Vector3>(propertyValue))
        {
            propertyValue = mescal::Vector3{ (float)sliders[0]->getValue(), (float)sliders[1]->getValue(), (float)sliders[2]->getValue() };
        }
        else if (std::holds_alternative<mescal::RGBColor>(propertyValue))
        {
            auto r = juce::jlimit(0.0f, 1.0f, (float)sliders[0]->getValue());
            auto g = juce::jlimit(0.0f, 1.0f, (float)sliders[1]->getValue());
            auto b = juce::jlimit(0.0f, 1.0f, (float)sliders[2]->getValue());
            propertyValue = mescal::RGBColor{ r, g, b };
        }
        else if (std::holds_alternative<juce::Colour>(propertyValue))
        {
            auto r = juce::jlimit(0.0f, 1.0f, (float)sliders[0]->getValue());
            auto g = juce::jlimit(0.0f, 1.0f, (float)sliders[1]->getValue());
            auto b = juce::jlimit(0.0f, 1.0f, (float)sliders[2]->getValue());
            auto a = juce::jlimit(0.0f, 1.0f, (float)sliders[3]->getValue());
            propertyValue = juce::Colour::fromFloatRGBA((float)r, (float)g, (float)b, (float)a);
        }
        else if (std::holds_alternative<juce::Point<float>>(propertyValue))
        {
            propertyValue = juce::Point<float>{ (float)sliders[0]->getValue(), (float)sliders[1]->getValue() };
        }

        if (onChange)
            onChange();
    }

    std::vector<std::unique_ptr<juce::Slider>> sliders;
};

class StringArrayPropertyComponent : public EffectPropertyValueComponent
{
public:
    StringArrayPropertyComponent(const juce::String& propertyName,
        int propertyIndex_,
        mescal::Effect::PropertyValue propertyValue_,
        juce::StringArray& strings) :
        EffectPropertyValueComponent(propertyName,
            propertyIndex_,
            propertyValue_)
    {
        int id = 1;
        for (auto const& string : strings)
        {
            comboBox.addItem(string, id++);
        }

        comboBox.setSelectedId(std::get<int>(propertyValue_));

        comboBox.onChange = [this]
            {
                propertyValue = comboBox.getSelectedItemIndex();
                if (onChange) onChange();
            };
    }

    void refresh() override
    {
    }

    void resized() override
    {
        auto contentBounds = getLookAndFeel().getPropertyComponentContentPosition(*this);
        comboBox.setBounds(contentBounds);
    }

    juce::ComboBox comboBox;
};
