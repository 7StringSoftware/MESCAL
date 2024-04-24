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
    GradientMesh(juce::Rectangle<float> initialPatchArea);
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

    struct Edge;
    struct Face;

    struct Vertex
    {
        Vertex(int gridX, int gridY, juce::Point<float> pos) :
            gridIndex{ gridX, gridY },
            position(pos)
        {
        }

        juce::Point<int> gridIndex;
        juce::Point<float> position;

        std::array<juce::WeakReference<Edge>, 4> edges; // top, right, bottom, left

#if JUCE_DEBUG
        juce::String name;
        juce::String dump() const;
#endif

        JUCE_DECLARE_WEAK_REFERENCEABLE(Vertex)
    };

    struct Edge
    {
        Edge(GradientMesh::Orientation orientation_, Point<float> cp1, Point<float> cp2, Vertex* v1, Vertex* v2) :
            orientation(orientation_),
            edgeControlPoints{ cp1, cp2 },
            vertices{ v1, v2 }
        {
        }

        GradientMesh::Orientation orientation;
        std::array<juce::Point<float>, 2> edgeControlPoints;
        std::array<juce::WeakReference<Vertex>, 2> vertices;

        juce::Line<float> toLine() const noexcept
        {
            return { vertices[0]->position, vertices[1]->position };
        }

#if JUCE_DEBUG
        juce::String name;
        juce::String dump() const;
#endif

        JUCE_DECLARE_WEAK_REFERENCEABLE(Edge)
    };

    struct Face
    {
        std::array<juce::WeakReference<Vertex>, 4> vertices; // top left, top right, bottom right, bottom left
        std::array<juce::WeakReference<Edge>, 4> edges; // top, right, bottom, left
        std::array<juce::Colour, 4> colors;
        std::array<juce::Point<float>, 4> innerControlPoints;

        void setInnerControlPoints();

#if JUCE_DEBUG
        juce::String name;
        juce::String dump() const;
#endif

        JUCE_DECLARE_WEAK_REFERENCEABLE(Face)
    };

    class Patch : public juce::ReferenceCountedObject
    {
    public:
        Patch(Face* face_);
        Patch(Patch const& other) = default;
        ~Patch();

        enum
        {
            numRows = 4,
            numColumns = 4,
            numControlPoints = numRows * numColumns,
            numColors = 4
        };

        juce::Rectangle<float> getBounds() const noexcept;

        using Ptr = juce::ReferenceCountedObjectPtr<Patch>;

    private:
        friend class GradientMesh;

        struct PatchPimpl;
        std::unique_ptr<PatchPimpl> pimpl;
        juce::WeakReference<Face> face;

        static const std::array<juce::Colour, 4> defaultColors;

        JUCE_LEAK_DETECTOR(Patch)
    };

    juce::Rectangle<float> getBounds() const noexcept;

    void draw(juce::Image image, juce::AffineTransform transform);

    Patch::Ptr addConnectedPatch(Patch::Ptr existingPatch, Direction direction, std::array<juce::Colour, 4> colors);

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
