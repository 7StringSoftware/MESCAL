#pragma once

class EffectPropertyValueComponent : public juce::PropertyComponent
{
public:
	EffectPropertyValueComponent(juce::String propertyName_, int propertyIndex_, mescal::Effect::PropertyValue propertyValue_) :
		PropertyComponent(propertyName_),
		propertyIndex(propertyIndex_),
		propertyValue(propertyValue_)
	{
	}

	int const propertyIndex;
	mescal::Effect::PropertyValue propertyValue;
	std::function<void(int propertyIndex, const mescal::Effect::PropertyValue& propertyValue)> onChange;

	virtual mescal::Effect::PropertyValue getPropertyValue()
	{
		return propertyValue;
	}
};

class EffectBooleanPropertyComponent : public EffectPropertyValueComponent
{
public:
	EffectBooleanPropertyComponent(juce::String propertyName_, int propertyIndex_, mescal::Effect::PropertyValue propertyValue_) :
		EffectPropertyValueComponent(propertyName_, propertyIndex_, propertyValue_)
	{
		addAndMakeVisible(toggleButton);
		toggleButton.onClick = [this]
			{
				propertyValue = toggleButton.getToggleState();

				if (onChange)
				{
					onChange(propertyIndex, propertyValue);
				}
			};
	}

	void resized() override
	{
		auto contentBounds = getLookAndFeel().getPropertyComponentContentPosition(*this);
		toggleButton.setBounds(contentBounds);
	}

	void refresh() override {}

	juce::ToggleButton toggleButton;
};

class MultiSliderPropertyComponent : public EffectPropertyValueComponent
{
public:
	MultiSliderPropertyComponent(const juce::String& propertyName_,
		int propertyIndex_,
		mescal::Effect::PropertyValue propertyValue_,
		juce::Array<float> const& sliderDefaultValues,
		juce::StringArray const& sliderNames_,
		juce::Range<float> sliderRange_,
		std::optional<std::tuple<float, bool>> skew) :
		EffectPropertyValueComponent(propertyName_, propertyIndex_, propertyValue_)
	{
		int index = 0;

		for (auto const& name : sliderNames_)
		{
			auto slider = std::make_unique<juce::Slider>(juce::Slider::LinearBar, juce::Slider::TextBoxLeft);
            if (sliderRange_.isEmpty())
                continue;

			slider->setRange(sliderRange_.getStart(), sliderRange_.getEnd(), 0.01);
			slider->setValue(sliderDefaultValues[index++], juce::dontSendNotification);

			if (skew.has_value())
			{
				auto [factor, symmetric] = *skew;
				slider->setSkewFactor(factor, symmetric);
			}

			addAndMakeVisible(slider.get());
			slider->onValueChange = [this]
				{
					sliderChanged();
				};

			auto label = std::make_unique<juce::Label>(name, name);
			addAndMakeVisible(label.get());
			label->attachToComponent(slider.get(), true);
			labels.push_back(std::move(label));

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
		else if (std::holds_alternative<mescal::Vector2>(propertyValue))
		{
			propertyValue = mescal::Vector2{ (float)sliders[0]->getValue(), (float)sliders[1]->getValue() };
		}
		else if (std::holds_alternative<mescal::Vector3>(propertyValue))
		{
			propertyValue = mescal::Vector3{ (float)sliders[0]->getValue(), (float)sliders[1]->getValue(), (float)sliders[2]->getValue() };
		}
		else if (std::holds_alternative<mescal::Vector4>(propertyValue))
		{
			propertyValue = mescal::Vector4{ (float)sliders[0]->getValue(), (float)sliders[1]->getValue(), (float)sliders[2]->getValue(), (float)sliders[3]->getValue() };
		}

		if (onChange)
			onChange(propertyIndex, propertyValue);
	}

	std::vector<std::unique_ptr<juce::Label>> labels;
	std::vector<std::unique_ptr<juce::Slider>> sliders;
};

class EnumPropertyComponent : public EffectPropertyValueComponent
{
public:
	EnumPropertyComponent(const juce::String& propertyName,
		int propertyIndex_,
		mescal::Effect::PropertyValue propertyValue_,
		juce::StringArray itemStrings) :
		EffectPropertyValueComponent(propertyName,
			propertyIndex_,
			propertyValue_)
	{
		int id = 1;
		for (auto const& itemString : itemStrings)
		{
			comboBox.addItem(itemString, id++);
		}

		comboBox.setSelectedItemIndex(std::get<mescal::Enumeration>(propertyValue_), juce::dontSendNotification);

		comboBox.onChange = [this]
			{
				propertyValue = comboBox.getSelectedItemIndex();
				if (onChange) onChange(propertyIndex, propertyValue);
			};

		addAndMakeVisible(comboBox);
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

class AffineTransformPropertyComponent : public EffectPropertyValueComponent
{
public:
	AffineTransformPropertyComponent(const juce::String& propertyName,
		int propertyIndex_,
		mescal::Effect::PropertyValue propertyValue_) :
		EffectPropertyValueComponent(propertyName,
			propertyIndex_,
			propertyValue_)
	{
		setPreferredHeight(2 * 30);

		std::array<juce::TextEditor*, 6> editors
		{
			&editor00, &editor01, &editor02,
			&editor10, &editor11, &editor12
		};

		for (auto& editor : editors)
		{
			addAndMakeVisible(editor);
			editor->onReturnKey = [this] { updateTransform(); };
			editor->onFocusLost = editor->onReturnKey;
		}

		auto const& transform = std::get<juce::AffineTransform>(propertyValue_);
		editor00.setText(juce::String{ transform.mat00, 3 });
		editor01.setText(juce::String{ transform.mat01, 3 });
		editor02.setText(juce::String{ transform.mat02, 3 });
		editor10.setText(juce::String{ transform.mat10, 3 });
		editor11.setText(juce::String{ transform.mat11, 3 });
		editor12.setText(juce::String{ transform.mat12, 3 });
	}

	void refresh() override
	{
	}

	void resized() override
	{
		auto contentBounds = getLookAndFeel().getPropertyComponentContentPosition(*this);
		std::array<juce::TextEditor*, 6> editors
		{
			&editor00, &editor01, &editor02,
			&editor10, &editor11, &editor12
		};

		auto it = editors.begin();
		int w = contentBounds.getWidth() / 3;
		int h = contentBounds.getHeight() / 2;
		for (int row = 0; row < 2; ++row)
		{
			for (int column = 0; column < 3; ++column)
			{
				(*it)->setBounds(contentBounds.getX() + column * w,
					contentBounds.getY() + row * h,
					w, h);
				++it;
			}
		}
	}

	void updateTransform()
	{
		juce::AffineTransform transform;
		transform.mat00 = editor00.getText().getFloatValue();
		transform.mat01 = editor01.getText().getFloatValue();
		transform.mat02 = editor02.getText().getFloatValue();
		transform.mat10 = editor10.getText().getFloatValue();
		transform.mat11 = editor11.getText().getFloatValue();
		transform.mat12 = editor12.getText().getFloatValue();
		propertyValue = transform;

		if (onChange)
			onChange(propertyIndex, propertyValue);
	}

	juce::TextEditor editor00, editor01, editor02;
	juce::TextEditor editor10, editor11, editor12;
};