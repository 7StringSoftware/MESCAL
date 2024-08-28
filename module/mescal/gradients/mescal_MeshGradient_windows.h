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
    MeshGradient(int numRows_, int numColumns_, std::optional<juce::Rectangle<float>> bounds = {});
    MeshGradient() = default;
    ~MeshGradient();

    enum class Placement
    {
        top, left, bottom, right, unknown = -1
    };

    struct Vertex
    {
        Vertex(MeshGradient& owner_, int row_, int column_, juce::Point<float> position_) :
            owner(owner_),
            row(row_), column(column_),
            position(position_)
        {
        }

        bool operator== (const Vertex& other) const
        {
            return juce::approximatelyEqual(position.x, other.position.x) && juce::approximatelyEqual(position.y, other.position.y);
        }

        std::shared_ptr<Vertex> getAdjacentVertex(Placement placement) const;

        MeshGradient& owner;
        int const row, column;
        juce::Point<float> position;
        Color128 color;

        struct BezierControlPoints
        {
            std::optional<juce::Point<float>> topControlPoint;
            std::optional<juce::Point<float>> leftControlPoint;
            std::optional<juce::Point<float>> bottomControlPoint;
            std::optional<juce::Point<float>> rightControlPoint;

            std::optional<juce::Point<float>> getControlPoint(Placement placement) const;
            void setControlPoint(Placement placement, juce::Point<float> point);
        } bezier;
    };

    int getNumRows() const
    {
        return numRows;
    }

    int getNumColumns() const
    {
        return numColumns;
    }

    static Placement opposite(Placement placement)
    {
        return static_cast<Placement>((static_cast<int>(placement) + 2) % 4);
    }

    std::shared_ptr<Vertex> getVertex(int row, int column);
    auto const& getVertices() const
    {
        return vertices;
    }

    juce::Rectangle<float> getBounds() const noexcept;
    void applyTransform(juce::AffineTransform const& transform);

    void draw(juce::Image image, juce::AffineTransform transform, juce::Colour backgroundColor = juce::Colours::transparentBlack);

private:
    /** @internal */
    int const numRows;
    int const numColumns;

    std::vector<std::shared_ptr<Vertex>> vertices;

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
