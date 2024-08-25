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


    For three-sided faces, squish the upper right corner onto the upper left
    corner so the top edge has zero length

                    RIGHT EDGE
    P00 / P01 / P02 / P03 -----------P33
             |                        |
             |                       /
             |                      /
             |                     /
             |                    /
             |                   /
           L |                  /
           E |                 / B
           F |                / O
           T |               / T
             |              / T
           E |             / O
           D |            / M
           G |           /
           E |          / E
             |         / D
             |        / G
             |       / E
             |      /
             |     /
             |    /
             |   /
              P30

*/

/**
* Color128 holds a single ARGB color value with premultiplied alpha. Each color channel is stored a 32-bit floating point value.
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

 By arranging the verices and control points, the patch can form arbitrary shapes:

 \image html simple_mesh_arbitrary_shape.webp width=25%

 But that's not much of a mesh. Here's a more complex mesh gradient with multiple patches, shown with and without the patch boundaries:

 \image html 8x8_side_by_side.webp width=50%

 */

class MeshGradient
{
public:
    MeshGradient(int numRows_, int numColumns_, std::optional<juce::Rectangle<float>> bounds = {});
    MeshGradient() = default;
    ~MeshGradient();

    enum class Placement
    {
        top, left, bottom, right
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

    void applyTransform(juce::AffineTransform const& transform);

    void draw(juce::Image image, juce::AffineTransform transform, juce::Colour backgroundColor = juce::Colours::transparentBlack);

private:
    int const numRows;
    int const numColumns;

    std::vector<std::shared_ptr<Vertex>> vertices;

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
