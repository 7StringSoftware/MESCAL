#pragma once

/**
* Color128 holds a single ARGB color value with premultiplied alpha. The color value is stored as four 32-bit floating-point
* values, one for each color channel. Each color channel can range from 0.0 to 1.0.
*/

struct Color128
{
    Color128() {}
    Color128(juce::Colour colour) :
        red(colour.getFloatRed()),
        green(colour.getFloatGreen()),
        blue(colour.getFloatBlue()),
        alpha(colour.getFloatAlpha())
    {
    }
    Color128(float red_, float green_, float blue_, float alpha_) :
        red(red_), green(green_), blue(blue_), alpha(alpha_)
    {
    }

    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
    float alpha = 0.0f;

    juce::Colour toColour() const noexcept
    {
        return juce::Colour::fromFloatRGBA(red, green, blue, alpha);
    }

    static Color128 fromHSV(float hue, float saturation, float value, float alpha) noexcept;
    static Color128 grayLevel(float level) noexcept;
};

/**

 A mesh gradient is a gradient with colors that transition smoothly between a set of points. Mesh gradient colors can blend and flow
 in multiple directions simultaneously.

 A MeshGradient creates and stores a set of points called vertices. Each vertex has a designated color. A group of four vertices defines a patch.
 Each edge of the patch is a cubic Bezier spline.

 Here's a basic mesh gradient with a single patch with the vertices and Bezier control points shown:

 \image html simple_mesh_with_controls.webp width=25%

 By arranging the vertices and control points, the patch can form arbitrary shapes:

 \image html simple_mesh_arbitrary_shape.webp width=35%

 Here's a more complex mesh gradient with multiple patches, shown with and without the patch boundaries:

 \image html 8x8_side_by_side.webp width=50%

 * The actual gradient is painted by a GPU shader onto a JUCE Image by calling the draw method. Note that the Image needs to
 * be a Direct2D Image.
 *
 */

class MeshGradient
{
public:
    MeshGradient(int numRows_, int numColumns_, std::optional<juce::Rectangle<float>> bounds_);
    MeshGradient() = default;
    ~MeshGradient();

    enum class EdgePlacement
    {
        left, bottom, right, top
    };

    enum class CornerPlacement
    {
        topLeft, bottomLeft, bottomRight, topRight, unknown = -1
    };

    static constexpr std::array<EdgePlacement, 4> edgePlacements
    {
        EdgePlacement::left,
        EdgePlacement::bottom,
        EdgePlacement::right,
        EdgePlacement::top
    };

    static constexpr std::array<CornerPlacement, 4> cornerPlacements
    {
        CornerPlacement::topLeft,
        CornerPlacement::bottomLeft,
        CornerPlacement::bottomRight,
        CornerPlacement::topRight
    };

    static constexpr std::array<size_t, 4> cornerIndices
    {
        0, 4 * 3 + 0, 4 * 3 + 3, 4 * 0 + 3
    };

    struct MatrixPosition
    {
        int row, column;
    };

    using BezierControlPair = std::pair<juce::Point<float>, juce::Point<float>>;

    struct Edge
    {
        juce::Point<float> tail;
        BezierControlPair bezierControlPoints;
        juce::Point<float> head;
    };

    struct Patch
    {
        Patch(MeshGradient& owner_, int row_, int column_);

        int const row, column;
        MeshGradient& owner;

        void setBounds(juce::Rectangle<float> rect);

        juce::Point<float> getPosition(int matrixRow, int matrixColumn) const noexcept;
        juce::Point<float> getCornerPosition(CornerPlacement placement) const noexcept;
        void setPosition(int matrixRow, int matrixColumn, juce::Point<float> position);
        void setCornerPosition(CornerPlacement placement, juce::Point<float> position);

        void setColor(CornerPlacement placement, juce::Colour color);
        Color128 getColor(CornerPlacement placement) const noexcept;

        void setEdge(EdgePlacement placement, Edge edge);
        Edge getEdge(EdgePlacement placement) const noexcept;

        std::array<juce::Point<float>, 16> points;
        std::array<Color128, 4> colors;

        static constexpr size_t numMatrixColumns = 4;
    };

    int getNumRows() const
    {
        return numRows;
    }

    int getNumColumns() const
    {
        return numColumns;
    }

#if 0
    static Placement opposite(Placement placement)
    {
        return static_cast<Placement>((static_cast<int>(placement) + 2) % 4);
    }

    std::shared_ptr<Vertex> getVertex(int row, int column);
    auto const& getVertices() const
    {
        return vertices;
    }
#endif

    auto const& getPatches() const noexcept
    {
        return patches;
    }

    std::shared_ptr<Patch> getPatch(int row, int column)
    {
        return patches[row * numColumns + column];
    }

    juce::Rectangle<float> getBounds() const noexcept;
    void applyTransform(juce::AffineTransform const& transform);

    void draw(juce::Image image, juce::AffineTransform transform, juce::Colour backgroundColor = juce::Colours::transparentBlack);

private:
    /** @internal */
    int const numRows;
    int const numColumns;

    std::vector<std::shared_ptr<Patch>> patches;

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
