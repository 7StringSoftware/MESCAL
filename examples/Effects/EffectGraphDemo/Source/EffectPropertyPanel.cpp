#include "EffectPropertyPanel.h"

EffectPropertyPanel::EffectPropertyPanel(mescal::Effect::Ptr effect_) :
	effect(effect_)
{
	buildPropertyPanel();
	addAndMakeVisible(propertyPanel);
	setSize(350, propertyPanel.getTotalContentHeight());

#if JUCE_DEBUG
	printProperties(effect_);
#endif
}

void EffectPropertyPanel::resized()
{
	propertyPanel.setBounds(getLocalBounds());
}

void EffectPropertyPanel::buildPropertyPanel()
{
	auto jsonEffectPropertiesObject = [&]()
		{
			juce::var jsonVar;
			juce::String jsonString;
#if JUCE_DEBUG
			juce::File directory = juce::File::getCurrentWorkingDirectory().getParentDirectory();

			bool found = false;
			while (!directory.isRoot() && !found)
			{
				auto files = directory.findChildFiles(juce::File::findFiles, true, "*.json");
				for (auto const& file : files)
				{
					if (file.getFileNameWithoutExtension() == "EffectParameters")
					{
						jsonString = file.loadFileAsString();
						found = true;
						break;
					}
				}

				directory = directory.getParentDirectory();
			}
#else
			jsonString = juce::String{ BinaryData::EffectParameters_json };
#endif
			auto result = juce::JSON::parse(jsonString, jsonVar);
			jassert(result.wasOk());
			return mescal::JSONObject{ jsonVar }
				.getArray("Effects")
				.getObject((int)effect->effectType)
				.getArray("Properties");
		}();

		auto getLabels = [&](int numSliders, int propertyIndex)
			{
				juce::StringArray array;

				auto jsonEffectPropertyObject = jsonEffectPropertiesObject.getObject(propertyIndex);
				if (jsonEffectPropertyObject.hasProperty("Labels"))
				{
					auto labelsArray = jsonEffectPropertyObject.get<mescal::JSONArray>("Labels");
					for (int i = 0; i < numSliders; ++i)
						array.add(labelsArray.get<juce::String>(i));
				}
				else
				{
					for (int i = 0; i < numSliders; ++i)
						array.add(juce::String{ i });
				}

				return array;
			};

		auto getRange = [&](mescal::Effect::PropertyInfo const& propertyInfo, int propertyIndex)
			{
				if (propertyInfo.range.has_value())
					return *(propertyInfo.range);

				auto jsonEffectPropertyObject = jsonEffectPropertiesObject.getObject(propertyIndex);
				if (jsonEffectPropertyObject.hasProperty("Range"))
				{
					auto rangeObject = jsonEffectPropertyObject.get<mescal::JSONObject>("Range");
					return juce::Range<float>
					{
						rangeObject.get<float>("Min"),
							rangeObject.get<float>("Max")
					};
				}

				return juce::Range<float>{ 0.0f, 1.0f };
			};

		auto getSkew = [&](int propertyIndex) -> std::optional<std::tuple<float, bool>>
			{
				auto jsonEffectPropertyObject = jsonEffectPropertiesObject.getObject(propertyIndex);
				if (jsonEffectPropertyObject.hasProperty("Skew"))
				{
					auto skewObject = jsonEffectPropertyObject.get < mescal::JSONObject>("Skew");
					float factor = skewObject.get<float>("Factor");
					bool symmetric = skewObject.get<bool>("Symmetric");
					return std::make_tuple(factor, symmetric);
				}

				return {};
			};

		propertyPanel.clear();
		propertyValueComponents.clear();

		//
		// Effect-specific controls
		//
		{
			juce::Array<juce::PropertyComponent*> sectionComponents;

			auto numProperties = effect->getNumProperties();
			for (auto propertyIndex = 0; propertyIndex < numProperties; ++propertyIndex)
			{
				EffectPropertyValueComponent* propertyComponent = nullptr;
				auto propertyInfo = effect->getPropertyInfo(propertyIndex);

				auto propertyName = effect->getPropertyName((int)propertyIndex);
				auto propertyValue = effect->getPropertyValue((int)propertyIndex);

				if (std::holds_alternative<bool>(propertyValue))
				{
					propertyComponent = new EffectBooleanPropertyComponent{ propertyName,
						propertyIndex,
						propertyValue };
				}
				else if (std::holds_alternative<int>(propertyValue))
				{
					auto defaultIntValue = std::get<int>(propertyValue);
					propertyComponent = new MultiSliderPropertyComponent{ propertyName,
						propertyIndex,
						propertyValue,
						juce::Array<float> { (float)defaultIntValue },
						juce::StringArray{ juce::String{} },
						juce::Range<float>{ 0, 1 },
						{} };
				}
				else if (std::holds_alternative<float>(propertyValue))
				{
					auto defaultFloatValue = std::get<float>(propertyValue);
					auto range = getRange(propertyInfo, propertyIndex);
					auto skew = getSkew(propertyIndex);

					propertyComponent = new MultiSliderPropertyComponent{ propertyName,
						propertyIndex,
						propertyValue,
						juce::Array<float> { defaultFloatValue },
						juce::StringArray{ juce::String{} },
						range,
						skew };
				}
				else if (std::holds_alternative<mescal::Vector2>(propertyValue))
				{
					auto defaultValue = std::get<mescal::Vector2>(propertyValue);
					auto range = getRange(propertyInfo, propertyIndex);
					auto skew = getSkew(propertyIndex);

					propertyComponent = new MultiSliderPropertyComponent{ propertyName,
						propertyIndex,
						propertyValue,
						juce::Array<float> { defaultValue[0], defaultValue[1] },
						getLabels(2, propertyIndex),
						range,
						skew };
				}
				else if (std::holds_alternative<mescal::Vector3>(propertyValue))
				{
					auto defaultValue = std::get<mescal::Vector3>(propertyValue);
					auto range = getRange(propertyInfo, propertyIndex);
					auto skew = getSkew(propertyIndex);

					propertyComponent = new MultiSliderPropertyComponent{ propertyName,
						propertyIndex,
						propertyValue,
						juce::Array<float> { defaultValue[0], defaultValue[1], defaultValue[2] },
						getLabels(3, propertyIndex),
						range, 
						skew };
				}
				else if (std::holds_alternative<mescal::Vector4>(propertyValue))
				{
					auto defaultValue = std::get<mescal::Vector4>(propertyValue);
					auto range = getRange(propertyInfo, propertyIndex);
					auto skew = getSkew(propertyIndex);

					propertyComponent = new MultiSliderPropertyComponent{ propertyName,
						propertyIndex,
						propertyValue,
						juce::Array<float> { defaultValue[0], defaultValue[1], defaultValue[2], defaultValue[3] },
						getLabels(4, propertyIndex),
						range,
						skew };
				}
				else if (std::holds_alternative<uint8_t>(propertyValue))
				{
					propertyComponent = new EnumPropertyComponent{ propertyName,
						propertyIndex,
						propertyValue,
						propertyInfo.enumeration };
				}
				else if (std::holds_alternative<juce::AffineTransform>(propertyValue))
				{
					propertyComponent = new AffineTransformPropertyComponent{ propertyName,
						propertyIndex,
						propertyValue };
				}
				else
				{
					jassertfalse;
				}

				if (propertyComponent)
				{
					propertyValueComponents.emplace_back(propertyComponent);
					sectionComponents.add(propertyComponent);
				}
			}

			propertyPanel.addSection("Properties", sectionComponents);
		}

		for (auto& c : propertyValueComponents)
		{
			c->onChange = [this](size_t propertyIndex, const mescal::Effect::PropertyValue& propertyValue)
				{
					effect->setPropertyValue((int)propertyIndex, propertyValue);

					if (onPropertyChange)
						onPropertyChange();
				};
		}
}

#if JUCE_DEBUG

void EffectPropertyPanel::printProperties(mescal::Effect::Ptr effect)
{
	juce::StringArray lines;

	auto name = effect->getName().removeCharacters("- ");
	lines.add("struct " + name);
	lines.add("{");
	size_t numProperties = effect->getNumProperties();
	for (auto index = 0; index < numProperties; ++index)
	{
		auto propertyName = effect->getPropertyName(index);
		auto firstChar = propertyName[0];
		firstChar = juce::CharacterFunctions::toLowerCase(firstChar);
		propertyName = firstChar + propertyName.substring(1);
		juce::String line = "    static constexpr int " + propertyName + " = ";
		line += juce::String{ index } + ";";
		lines.add(line);
	}

	for (auto index = 0; index < numProperties; ++index)
	{
		auto info = effect->getPropertyInfo(index);

		if (info.enumeration.size() > 0)
			lines.add({});

		int j = 0;
		for (auto const& s : info.enumeration)
		{
			auto firstChar = s[0];
			firstChar = juce::CharacterFunctions::toLowerCase(firstChar);
			juce::String line = "    static constexpr int ";
			line += firstChar;
			line += s.substring(1) + " = ";
			line += juce::String{ j++ } + ";";
		}
	}

	lines.add("};");

	juce::SystemClipboard::copyTextToClipboard(lines.joinIntoString("\n"));
}

#endif
