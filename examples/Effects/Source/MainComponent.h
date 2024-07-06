#pragma once

#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <d2d1_3helper.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#include <initguid.h>
#include <d2d1effectauthor.h>
#include <d2d1effecthelpers.h>
#include <JuceHeader.h>

class MainComponent  : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Image input, output;
    mescal::Effect effect{ mescal::Effect::EffectType::gaussianBlur };
    mescal::CustomEffect customEffect;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
