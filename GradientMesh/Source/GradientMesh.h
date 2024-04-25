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

class GradientMesh
{
public:
    GradientMesh(std::array<juce::Point<float>, 4> cornerPositions);
    GradientMesh();
    ~GradientMesh();

    enum class Corner : size_t
    {
        topLeft, topRight, bottomRight, bottomLeft
    };

    enum class Direction : size_t
    {
        north, east, south, west
    };

    enum class Orientation : size_t
    {
        horizontal,
        vertical
    };

    struct GridCoordinates
    {
        int x = 0, y = 0;

        GridCoordinates translated(Direction direction) const
        {
            switch (direction)
            {
            case Direction::north:
                return { x, y - 1 };
            case Direction::east:
                return { x + 1, y };
            case Direction::south:
                return { x, y + 1 };
            case Direction::west:
                return { x - 1, y };
            default:
                return *this;
            }
        }
    };

    struct CornerOptions
    {
        juce::Point<float> position;
        juce::Colour color;
    };

    struct EdgeOptions
    {
        std::array<juce::Point<float>, 2> controlPointPositions;
    };


    struct PatchOptions
    {
        GridCoordinates gridCoordinates;

        CornerOptions upperLeftCorner;
        CornerOptions upperRightCorner;
        CornerOptions lowerLeftCorner;
        CornerOptions lowerRightCorner;

        struct
        {
            juce::Point<float> upperControlPoint;
            juce::Point<float> lowerControlPoint;
        } leftEdge, rightEdge;

        struct
        {
            juce::Point<float> leftControlPoint;
            juce::Point<float> rightControlPoint;
        } topEdge, bottomEdge;
    };

    class Patch : public juce::ReferenceCountedObject
    {
    public:
        Patch(PatchOptions& options_, GradientMesh& mesh_);
        Patch(Patch const& other);
        ~Patch();

        enum
        {
            numRows = 4,
            numColumns = 4,
            numControlPoints = numRows * numColumns,
            numColors = 4
        };

        juce::Rectangle<float> getBounds() const noexcept;
        void translate(float x, float y);
        void flipControlPointsHorizontally();
        void flipColorsHorizontally();
        void flipControlPointsVertically();
        void flipColorsVertically();

        using Ptr = juce::ReferenceCountedObjectPtr<Patch>;

    private:
        friend class GradientMesh;

        struct PatchPimpl;
        std::unique_ptr<PatchPimpl> pimpl;

        GradientMesh& mesh;
        PatchOptions options;
        static const std::array<juce::Colour, 4> defaultColors;

        JUCE_LEAK_DETECTOR(Patch)
    };

    juce::Rectangle<float> getBounds() const noexcept;

    void draw(juce::Image image, juce::AffineTransform transform);

    Patch::Ptr addConnectedPatch(Patch::Ptr existingPatch, Direction direction, std::array<juce::Colour, 4> colors);

    Patch::Ptr addPatch(PatchOptions& options);

    Patch::Ptr clonePatch(Patch::Ptr originalPatch, Direction direction);

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
