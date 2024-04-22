/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

  name:             GradientMesh

  dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
  exporters:        VS2022

  moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
  defines:

  type:             Component
  mainClass:        GradientMeshTest

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once




/*

    D2D1_GRADIENT_MESH_PATCH


    P00  Top left corner
    P03  Top right corner
    P30  Bottom left corner
    P33  Bottom right corner

    P01  Top edge control point #1
    P02  Top edge control point #1

    P10  Left edge control point #1
    P20  Left edge control point #1

    P13  Right edge control point #1
    P23  Right edge control point #1

    P31  Bottom edge control point #1
    P32  Bottom edge control point #1

    P11  Top left corner (inner)
    P12  Top right corner (inner)
    P21  Bottom left corner (inner)
    P22  Bottom right corner (inner)


         P01
        /
     P00--------------------P03
    / |                    / | \
 P10  |                  P02 |  P13
      |                      |
      |     P11     P12      |
      |                      |
      |     P21     P22      |
      |                      |
      | P20                  |
      | /   P31          P23 |
      |/   /               \ |
     P30--------------------P33
                          /
                       P32

*/

class ColorSelectorButton : public juce::Button
{
public:
    ColorSelectorButton();
    ~ColorSelectorButton() override = default;

    void clicked(const ModifierKeys& modifiers) override;
    void paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    juce::Value colorValue;
};

class ControlPointComponent : public juce::Component
{
public:
    ControlPointComponent(int row_, int column_, juce::Value colorValue_);
    ~ControlPointComponent() override = default;

    void paint(juce::Graphics& g) override;

    void mouseEnter(const MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

    void moved() override;

    std::function<void()> onMouseEnter;
    std::function<void()> onMove;

    std::unique_ptr<ColorSelectorButton> colorSelector;
    juce::Value colorValue;

private:
    juce::ComponentDragger dragger;
    int column, row;
};

class GradientMeshTest : public juce::Component
{
public:
    GradientMeshTest();
    ~GradientMeshTest() override = default;

    void resized() override;

    void paint(juce::Graphics& g) override;

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GradientMeshTest)
};

