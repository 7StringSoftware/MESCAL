#if 0
#include "SpotSpecularLightingControlComponent.h"

SpotSpecularLightingControlComponent::SpotSpecularLightingControlComponent(mescal::Effect* const effect_) :
    effect(effect_),
    lightPosition("Light Source", mescal::SpotSpecularLightingProperty::lightPosition),
    focusPointPosition("Focus Point ", mescal::SpotSpecularLightingProperty::focusPointPosition)
{
    addAndMakeVisible(focusPointPosition);
    addAndMakeVisible(lightPosition);
    addAndMakeVisible(focusSlider);
    addAndMakeVisible(coneAngleSlider);
    addAndMakeVisible(specularExponentSlider);
    addAndMakeVisible(specularConstantSlider);
    addAndMakeVisible(surfaceScaleSlider);

    lightPosition.onChange = [this](int propertyIndex, mescal::Point3D p3)
    {
        effect->setProperty(propertyIndex, mescal::Effect::PropertyValue{ p3 });

        if (onEffectChange)
            onEffectChange();
    };
    focusPointPosition.onChange = lightPosition.onChange;

    focusSlider.setRange(0.0, 200.0, 0.1);
    focusSlider.setSkewFactor(0.9);
    focusSlider.setTextValueSuffix(" focus");
    focusSlider.onValueChange = [this]
        {
            effect->setProperty(mescal::SpotSpecularLightingProperty::focus, (float)focusSlider.getValue());
            if (onEffectChange)
                onEffectChange();
        };

    coneAngleSlider.setRange(0.0, 90.0, 0.1);
    coneAngleSlider.setTextValueSuffix(" degrees");
    coneAngleSlider.onValueChange = [this]
        {
            effect->setProperty(mescal::SpotSpecularLightingProperty::limitingConeAngle, (float)coneAngleSlider.getValue());
            if (onEffectChange)
                onEffectChange();
        };

    specularExponentSlider.setRange(1.0, 128.0, 0.1);
    specularExponentSlider.setTextValueSuffix(" specular exponent");
    specularExponentSlider.onValueChange = [this]
        {
            effect->setProperty(mescal::SpotSpecularLightingProperty::specularExponent, (float)specularExponentSlider.getValue());
            if (onEffectChange)
                onEffectChange();
        };

    specularConstantSlider.setRange(0.0, 10000.0, 0.1);
    specularConstantSlider.setSkewFactor(0.1);
    specularConstantSlider.setTextValueSuffix(" specular constant");
    specularConstantSlider.onValueChange = [this]
        {
            effect->setProperty(mescal::SpotSpecularLightingProperty::specularConstant, (float)specularConstantSlider.getValue());
            if (onEffectChange)
                onEffectChange();
        };

    surfaceScaleSlider.setRange(0.0, 10000.0, 0.1);
    surfaceScaleSlider.setSkewFactor(0.1);
    surfaceScaleSlider.setTextValueSuffix(" surface scale");
    surfaceScaleSlider.onValueChange = [this]
        {
            effect->setProperty(mescal::SpotSpecularLightingProperty::surfaceScale, (float)surfaceScaleSlider.getValue());
            if (onEffectChange)
                onEffectChange();
        };
}

void SpotSpecularLightingControlComponent::initEffect()
{
    {
        mescal::Point3D position{ 200.0f, 200.0f, 100.0f };
        lightPosition.setPosition3D(position);
    }
    {
        mescal::Point3D position{ 400.0f, 400.0f, 0.0f };
        focusPointPosition.setPosition3D(position);
    }

    focusSlider.setValue(1.0);
    coneAngleSlider.setValue(90.0);
    specularExponentSlider.setValue(1.0);
    specularConstantSlider.setValue(1.0);
    surfaceScaleSlider.setValue(10.0);
}

void SpotSpecularLightingControlComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::deeppink);
    g.drawRect(getLocalBounds());
}

void SpotSpecularLightingControlComponent::resized()
{
    {
        auto position = effect->getProperty(lightPosition.propertyIndex);

        if (std::holds_alternative<mescal::Point3D>(position))
        {
            lightPosition.setSize(250, 250);
            lightPosition.setPosition3D(std::get<mescal::Point3D>(position));
        }
    }
    {
        auto position = effect->getProperty(focusPointPosition.propertyIndex);

        if (std::holds_alternative<mescal::Point3D>(position))
        {
            focusPointPosition.setSize(250, 250);
            focusPointPosition.setPosition3D(std::get<mescal::Point3D>(position));
        }
    }

    std::array<juce::Slider*, 5> sliders{ &focusSlider, &coneAngleSlider, &specularExponentSlider, &specularConstantSlider, &surfaceScaleSlider };
    int w = 200;
    int x = 50;
    int h = 30;
    int y = getHeight() - h - 10;
    int gap = 20;
    for (auto slider : sliders)
    {
        slider->setBounds(x, y, w, h);
        x += w + gap;
    }
}

void SpotSpecularLightingControlComponent::Position3DComponent::moved()
{
    if (onChange)
        onChange(propertyIndex, mescal::Point3D{ (float)getBounds().getCentreX(), (float)getBounds().getCentreY(), (float)zSlider.getValue() });
}

SpotSpecularLightingControlComponent::Position3DComponent::Position3DComponent(juce::String name, int propertyIndex_) :
    Component(name),
    propertyIndex(propertyIndex_)
{
    for (int i = 0; i < 4; ++i)
    {
        auto& arrow = arrows.emplace_back(std::make_unique<DragArrow>(dragger, (float)i * juce::MathConstants<float>::halfPi));
        addAndMakeVisible(arrow.get());
    }

    zSlider.setRange(-500.0, 500.0, 1.0);
    zSlider.setTextValueSuffix(" Z");
    zSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    zSlider.setColour(juce::Slider::trackColourId, juce::Colours::darkgrey);
    zSlider.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(zSlider);
    zSlider.onValueChange = [this]
    {
        if (onChange)
            onChange(propertyIndex, mescal::Point3D{ (float)getBounds().getCentreX(), (float)getBounds().getCentreY(), (float)zSlider.getValue() });
    };
}

void SpotSpecularLightingControlComponent::Position3DComponent::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().reduced(40);
    int h = 24;
    auto y = area.getCentreY() - 24 * 2;
    auto pos = getBounds().getCentre();

    {
	    g.setColour(juce::Colours::darkgrey.withAlpha(0.8f));
	    auto rectSize = (float)(h * 4 + 40);
        juce::Rectangle<float> r = getLocalBounds().toFloat().withSizeKeepingCentre(rectSize, rectSize);
        float cornerSize = rectSize * 0.1f;
	    g.fillRoundedRectangle(r, cornerSize);
	    g.setColour(juce::Colours::lightgrey.withAlpha(0.8f));
	    g.drawRoundedRectangle(r, cornerSize, 3.0f);
    }

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions{ (float)h * 0.8f });
    g.drawText(getName(), area.getX(), y, area.getWidth(), h, juce::Justification::centred);
    y += h;
    g.setFont(juce::FontOptions{ (float)h * 0.6f });
    g.drawText(juce::String(pos.x) + " X", area.getX(), y, area.getWidth(), h, juce::Justification::horizontallyCentred);
    y += h;
    g.drawText(juce::String(pos.y) + " Y", area.getX(), y, area.getWidth(), h, juce::Justification::horizontallyCentred);
}

void SpotSpecularLightingControlComponent::Position3DComponent::resized()
{
    auto innerR = getLocalBounds();
    innerR = innerR.reduced(25);

    juce::Rectangle<int> arrowR{ 50, 50 };
    arrows[0]->setBounds(arrowR.withCentre({ innerR.getCentreX(), innerR.getY() }));
    arrows[1]->setBounds(arrowR.withCentre({ innerR.getRight(), innerR.getCentreY() }));
    arrows[2]->setBounds(arrowR.withCentre({ innerR.getCentreX(), innerR.getBottom() }));
    arrows[3]->setBounds(arrowR.withCentre({ innerR.getX(), innerR.getCentreY() }));

    int sliderW = proportionOfWidth(0.4f);
    int sliderX = getWidth() / 2 - sliderW / 2;
    zSlider.setBounds(sliderX, getHeight() / 2 + 24, sliderW, 30);
}

void SpotSpecularLightingControlComponent::Position3DComponent::setPosition3D(mescal::Point3D p3)
{
    auto center = juce::Point<float>(p3.x, p3.y).toInt();
    setCentrePosition(center);
    zSlider.setValue(p3.z);

    if (onChange)
        onChange(propertyIndex, p3);
}

SpotSpecularLightingControlComponent::DragArrow::DragArrow(juce::ComponentDragger& dragger_, float angle_) :
    dragger(dragger_), angle(angle_)
{
}

void SpotSpecularLightingControlComponent::DragArrow::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.addTransform(juce::AffineTransform::rotation(angle, (float)getWidth() * 0.5f, (float)getHeight() * 0.5f));

    juce::Path path;
    path.addArrow({ r.getCentreX(), r.getBottom(), r.getCentreX(), r.getY() }, r.getHeight() * 0.4f, r.getWidth() * 0.75f, r.getWidth() * 0.4f);
    g.setColour(juce::Colours::lightgrey);
    g.fillPath(path);
    g.setColour(juce::Colours::darkgrey);
    g.strokePath(path, juce::PathStrokeType{ 3.0f });
}

void SpotSpecularLightingControlComponent::DragArrow::mouseDown(const juce::MouseEvent& event)
{
    dragger.startDraggingComponent(getParentComponent(), event.getEventRelativeTo(getParentComponent()));
}

void SpotSpecularLightingControlComponent::DragArrow::mouseDrag(const juce::MouseEvent& event)
{
    dragger.dragComponent(getParentComponent(), event.getEventRelativeTo(getParentComponent()), nullptr);
}
#endif
