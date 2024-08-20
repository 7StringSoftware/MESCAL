#pragma once

#include <JuceHeader.h>
#include "PropertyComponents.h"

class EffectPropertyPanel : public juce::Component
{
public:
	EffectPropertyPanel(mescal::Effect::Ptr effect_);

	void resized() override;

	std::function<void()> onPropertyChange;

private:
	mescal::Effect::Ptr effect;
	juce::PropertyPanel propertyPanel;
	std::vector<juce::Component::SafePointer<EffectPropertyValueComponent>> propertyValueComponents;

	void buildPropertyPanelFromJSON();
	void buildPropertyPanel();

#if JUCE_DEBUG
	static void printProperties(mescal::Effect::Ptr effect);
#endif

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectPropertyPanel)
};
