/*
  ==============================================================================

    GradientMesh.cpp
    Created: 22 Apr 2024 12:03:36am
    Author:  Matt Gonzalez

  ==============================================================================
*/

#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#include <JuceHeader.h>
#include <juce_graphics/native/juce_DirectX_windows.h>
#include <juce_graphics/native/juce_Direct2DImage_windows.h>
#include "GradientMesh.h"

struct GradientMeshTest::Pimpl
{
    static constexpr int numColumns = 4;
    static constexpr int numRows = 4;

    Pimpl(GradientMeshTest& owner_) : owner(owner_)
    {
        std::array<std::pair<int, int>, 4> colorPoints = { { { 0, 0 }, { 0, 3 }, { 3, 3 }, { 3, 0 } } };

        colorValues[colorPoints[0]] = (int)juce::Colours::red.getARGB();
        colorValues[colorPoints[1]] = (int)juce::Colours::green.getARGB();
        colorValues[colorPoints[2]] = (int)juce::Colours::white.getARGB();
        colorValues[colorPoints[3]] = (int)juce::Colours::blue.getARGB();

        int xStep = owner.getWidth() / (numColumns + 1);
        int yStep = owner.getHeight() / (numRows + 1);
        int x = xStep;
        int y = yStep;

        for (int row = 0; row < numRows; ++row)
        {
            for (int column = 0; column < numColumns; ++column)
            {
                auto& c = controlPointComponents[row][column];
                c = std::make_unique<ControlPointComponent>(row, column, colorValues[{ row, column }]);

                juce::String name;
                name << row << column;
                c->setName(name);
                c->setCentrePosition(x, y);
                owner.addAndMakeVisible(c.get());

                x += xStep;
            }

            x = xStep;
            y += yStep;
        }

        for (int row = 0; row < numRows; ++row)
        {
            for (int column = 0; column < numColumns; ++column)
            {
                auto& c = controlPointComponents[row][column];

                juce::Component::SafePointer<ControlPointComponent> controlPointSafePointer(c.get());
                c->onMove = [=]()
                    {
                        makeGradient();
                    };

                c->onMouseEnter = [=]()
                    {
                    };
            }
        }

        for (auto const colorPoint : colorPoints)
        {
            auto& c = controlPointComponents[colorPoint.first][colorPoint.second];
            c->colorSelector = std::make_unique<ColorSelectorButton>();
            owner.addAndMakeVisible(c->colorSelector.get());
            c->moved();
            c->colorSelector->colorValue.referTo(c->colorValue);
        }

        makeGradient();
    }

    void createResources()
    {
        if (image.isNull())
        {
            image = juce::Image(juce::Image::ARGB, 1024, 1024, true);
        }

        if (!deviceContext)
        {
            if (auto pixelData = dynamic_cast<juce::Direct2DPixelData*>(image.getPixelData()))
            {
                if (auto adapter = pixelData->getAdapter())
                {
                    winrt::com_ptr<ID2D1DeviceContext1> deviceContext1;
                    if (const auto hr = adapter->direct2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
                        deviceContext1.put());
                        FAILED(hr))
                    {
                        jassertfalse;
                        return;
                    }

                    deviceContext = deviceContext1.as<ID2D1DeviceContext2>();
                }
            }
        }
    }

    void makeGradient()
    {
        createResources();

        auto getCenter = [&](int row, int column)
            {
                auto p = controlPointComponents[row][column]->getBounds().getCentre().toFloat();
                return D2D1_POINT_2F{ p.getX(), p.getY() };
            };

        D2D1_GRADIENT_MESH_PATCH patch;
        patch.point00 = getCenter(0, 0);
        patch.point01 = getCenter(0, 1);
        patch.point02 = getCenter(0, 2);
        patch.point03 = getCenter(0, 3);
        patch.point10 = getCenter(1, 0);
        patch.point11 = getCenter(1, 1);
        patch.point12 = getCenter(1, 2);
        patch.point13 = getCenter(1, 3);
        patch.point20 = getCenter(2, 0);
        patch.point21 = getCenter(2, 1);
        patch.point22 = getCenter(2, 2);
        patch.point23 = getCenter(2, 3);
        patch.point30 = getCenter(3, 0);
        patch.point31 = getCenter(3, 1);
        patch.point32 = getCenter(3, 2);
        patch.point33 = getCenter(3, 3);

        patch.color00 = juce::D2DUtilities::toCOLOR_F(getColor(0, 0));
        patch.color03 = juce::D2DUtilities::toCOLOR_F(getColor(0, 3));
        patch.color30 = juce::D2DUtilities::toCOLOR_F(getColor(3, 0));
        patch.color33 = juce::D2DUtilities::toCOLOR_F(getColor(3, 3));

        patch.leftEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
        patch.topEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
        patch.rightEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
        patch.bottomEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;

        if (deviceContext)
        {
            auto hr = deviceContext->CreateGradientMesh(&patch, 1, gradientMesh.put());
            jassert(SUCCEEDED(hr));
        }

        owner.repaint();
    }

    void paintGradient(juce::Graphics& g)
    {
        if (deviceContext && image.isValid())
        {
            if (auto pixelData = dynamic_cast<juce::Direct2DPixelData*>(image.getPixelData()))
            {
                if (auto bitmap = pixelData->getAdapterD2D1Bitmap())
                {
                    deviceContext->SetTarget(bitmap);
                    deviceContext->BeginDraw();
                    deviceContext->Clear({ 0.0f, 0.0f, 0.0f, 1.0f });

                    if (gradientMesh)
                    {
                        deviceContext->DrawGradientMesh(gradientMesh.get());
                    }

                    deviceContext->EndDraw();
                    deviceContext->SetTarget(nullptr);
                }
            }

            g.drawImageAt(image, 0, 0);
        }
    }

    bool hasColor(int row, int column)
    {
        return colorValues.find({ row, column }) != colorValues.end();
    }

    juce::Colour getColor(int row, int column)
    {
        if (hasColor(row, column))
            return juce::Colour{ (uint32)(int)colorValues[{ row, column }].getValue() };

        return juce::Colours::white;
    }

    GradientMeshTest& owner;
    winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
    winrt::com_ptr<ID2D1GradientMesh> gradientMesh;
    juce::Image image;

    std::unique_ptr<ControlPointComponent> controlPointComponents[4][4];
    std::map<std::pair<int, int>, juce::Value> colorValues;
};

GradientMeshTest::GradientMeshTest()
{
    setSize(1024, 1024);

    pimpl = std::make_unique<Pimpl>(*this);
}

void GradientMeshTest::resized()
{
}

void GradientMeshTest::paint(juce::Graphics& g)
{
    pimpl->createResources();

    g.fillAll(juce::Colour{ 0xff111111 });

    pimpl->paintGradient(g);

    auto drawLine = [&](int column1, int row1, int column2, int row2)
        {
            auto comp1 = pimpl->controlPointComponents[column1][row1].get();
            auto comp2 = pimpl->controlPointComponents[column2][row2].get();

            g.setColour(juce::Colours::black);
            g.drawLine(juce::Line<float>{ comp1->getBounds().getCentre().toFloat(),
                comp2->getBounds().getCentre().toFloat() }, 6.0f);
            g.setColour(juce::Colours::white);
            g.drawLine(juce::Line<float>{ comp1->getBounds().getCentre().toFloat(),
                comp2->getBounds().getCentre().toFloat() }, 3.0f);
        };

    drawLine(0, 0, 0, 1);
    drawLine(0, 0, 1, 0);

    drawLine(0, 3, 0, 2);
    drawLine(0, 3, 1, 3);

    drawLine(3, 3, 2, 3);
    drawLine(3, 3, 3, 2);

    drawLine(3, 0, 2, 0);
    drawLine(3, 0, 3, 1);
}

ControlPointComponent::ControlPointComponent(int row_, int column_, juce::Value colorValue_)
    : column(column_), row(row_), colorValue(colorValue_)
{
    setSize(32, 32);
}

void ControlPointComponent::paint(juce::Graphics& g)
{
    auto c = juce::Colour{ (uint32)(int)colorValue.getValue() };
    g.setColour(c);
    g.fillEllipse(getLocalBounds().toFloat());
    g.setColour(c.contrasting());
    g.drawText(getName(), getLocalBounds(), juce::Justification::centred);
    g.drawEllipse(getLocalBounds().toFloat().reduced(0.5f), 1.0f);
}

void ControlPointComponent::mouseEnter(const MouseEvent&)
{
    if (onMouseEnter)
        onMouseEnter();
}

void ControlPointComponent::mouseDown(const juce::MouseEvent& e)
{
    dragger.startDraggingComponent(this, e);
}

void ControlPointComponent::mouseDrag(const juce::MouseEvent& e)
{
    dragger.dragComponent(this, e, nullptr);
}

void ControlPointComponent::moved()
{
    if (onMove)
    {
        onMove();
    }

    if (colorSelector)
    {
        colorSelector->setBounds(getBounds().translated(5 + getWidth(), 0));
    }
}

ColorSelectorButton::ColorSelectorButton()
    : juce::Button("ColorSelectorComponent")
{
    setSize(32, 32);
    setOpaque(false);
}

void ColorSelectorButton::clicked(const ModifierKeys& modifiers)
{
    auto content = std::make_unique<juce::ColourSelector>();
    content->setCurrentColour(juce::Colour{ (uint32)(int)colorValue.getValue() }, juce::dontSendNotification);
    content->setSize(300, 300);

    juce::CallOutBox::launchAsynchronously(std::move(content),
        getScreenBounds(),
        nullptr);
}

void ColorSelectorButton::paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    const std::array<juce::Colour, 7> colors = { juce::Colours::red, juce::Colours::orange, juce::Colours::yellow,
    juce::Colours::green, juce::Colours::blue, juce::Colours::indigo, juce::Colours::violet };

    juce::Path p;
    float radius = juce::jmin(getWidth() * 0.45f, getHeight() * 0.45f);

    if (shouldDrawButtonAsDown)
        radius *= 0.9f;

    float angleStep = juce::MathConstants<float>::twoPi / (float)colors.size();
    auto center = getLocalBounds().getCentre().toFloat();
    p.startNewSubPath(center);
    p.lineTo(center - juce::Point<float>{ 0.0f, radius });
    p.addCentredArc(getWidth() * 0.5f, getHeight() * 0.5f, radius, radius, 0.0f, 0.0f, angleStep, false);
    p.closeSubPath();

    float angle = 0.0f;
    for (auto const& color : colors)
    {
        g.setColour(color);
        g.fillPath(p, juce::AffineTransform::rotation(angle, center.x, center.y));
        angle += angleStep;
    }

    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(juce::Colours::white);
        g.drawEllipse(juce::Rectangle<float>{ radius * 2.0f, radius * 2.0f}.withCentre(center), 2.0f);
    }
}
