#pragma once

#include <JuceHeader.h>

class SpotSpecularLightingControlComponent : public juce::Component
{
public:
    SpotSpecularLightingControlComponent(mescal::Effect *const effect_);
    void initEffect();

    void paint(juce::Graphics&) override;
    void resized() override;

    std::function<void()> onEffectChange;

private:
    mescal::Effect* const effect;

    struct DragArrow : public juce::Component
    {
        DragArrow(juce::ComponentDragger& dragger_, float angle_);
        void paint(juce::Graphics&) override;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;

        juce::ComponentDragger& dragger;
        float angle;
    };

    struct Position3DComponent : public juce::Component
    {
        Position3DComponent(juce::String name, int propertyIndex_);

        void paint(juce::Graphics&) override;
        void resized() override;
        void moved() override;

        juce::Slider zSlider{ juce::Slider::LinearBar, juce::Slider::TextBoxLeft };

        mescal::Point3D getPosition3D() const noexcept;
        void setPosition3D(mescal::Point3D p3);

        juce::ComponentDragger dragger;
        std::vector<std::unique_ptr<DragArrow>> arrows;
        int propertyIndex = 0;
        std::function<void(int propertyIndex, mescal::Point3D p3)> onChange;

    } lightPosition, focusPointPosition;

    juce::Slider focusSlider{ juce::Slider::LinearBar, juce::Slider::TextBoxLeft };
    juce::Slider coneAngleSlider{ juce::Slider::LinearBar, juce::Slider::TextBoxLeft };
    juce::Slider specularExponentSlider{ juce::Slider::LinearBar, juce::Slider::TextBoxLeft };
    juce::Slider specularConstantSlider{ juce::Slider::LinearBar, juce::Slider::TextBoxLeft };
    juce::Slider surfaceScaleSlider{ juce::Slider::LinearBar, juce::Slider::TextBoxLeft };
};
