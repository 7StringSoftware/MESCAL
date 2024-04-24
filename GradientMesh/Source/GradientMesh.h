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

#include <JuceHeader.h>

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

struct GridPosition
{
    int row = 0, column = 0;
};

class GradientMesh
{
public:
    GradientMesh();
    ~GradientMesh();

    class Patch : public juce::ReferenceCountedObject
    {
    public:
        Patch(juce::Rectangle<float> area);
        Patch(Patch const& other) = default;
        ~Patch();

        enum
        {
            numRows = 4,
            numColumns = 4,
            numControlPoints = numRows * numColumns,
            numColors = 4
        };

        static GridPosition indexToGridPosition(int index)
        {
            return GridPosition{ index / numRows, index % numColumns };
        }

        juce::Rectangle<float> getBounds() const noexcept;

        juce::Point<float> getControlPointPosition(GridPosition gridPosition) const;
        std::optional<juce::Colour> getControlPointColor(GridPosition gridPosition) const;

        using Ptr = juce::ReferenceCountedObjectPtr<Patch>;

    private:
        friend class GradientMesh;

        struct PatchPimpl;
        std::unique_ptr<PatchPimpl> pimpl;

        juce::WeakReference<Patch> leftNeighbor;
        juce::WeakReference<Patch> rightNeighbor;
        juce::WeakReference<Patch> topNeighbor;
        juce::WeakReference<Patch> bottomNeighbor;

        void setControlPointPosition(GridPosition gridPosition, juce::Point<float> pos);
        void setControlPointColor(GridPosition gridPosition, juce::Colour color);

        JUCE_LEAK_DETECTOR(Patch)
        JUCE_DECLARE_WEAK_REFERENCEABLE(Patch)
    };

    juce::Rectangle<float> getBounds() const noexcept;

    void setControlPointPosition(Patch::Ptr patch, GridPosition gridPosition, juce::Point<float> pos);
    void setControlPointColor(Patch::Ptr patch, GridPosition gridPosition, juce::Colour color);
    void draw(juce::Image image, juce::AffineTransform transform);

    void addPatch(juce::Rectangle<float> area);
    auto const& getPatches() const
    {
        return patches;
    }

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    juce::ReferenceCountedArray<Patch> patches;

    void updateMesh();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GradientMesh)
};
