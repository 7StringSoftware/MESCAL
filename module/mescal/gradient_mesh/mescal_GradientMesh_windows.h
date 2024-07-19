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
};

class GradientMesh
{
public:
    GradientMesh(int numRows_, int numColumns_);
    GradientMesh() = default;
    ~GradientMesh();

    struct Halfedge;
    struct Vertex
    {
        Vertex(int row_, int column_, juce::Point<float> position_) :
            row(row_), column(column_),
            position(position_)
        {
        }

        bool operator== (const Vertex& other) const
        {
            return juce::approximatelyEqual(position.x, other.position.x) && juce::approximatelyEqual(position.y, other.position.y);
        }

        void configure(juce::Point<float> position_, juce::Colour color)
        {
            position = position_;
            setColors(color);
        }

        void setColors(juce::Colour color)
        {
            northwestColor = color;
            southwestColor = color;
            southeastColor = color;
            northeastColor = color;
        }

        int const row, column;
        juce::Point<float> position;
        std::weak_ptr<Halfedge> northHalfedge, eastHalfedge, southHalfedge, westHalfedge;

        juce::Colour northwestColor;
        juce::Colour southwestColor;
        juce::Colour southeastColor;
        juce::Colour northeastColor;
    };

    struct Halfedge
    {
        std::weak_ptr<Vertex> tail, head;
        std::weak_ptr<Halfedge> twin;

        std::optional<std::pair<juce::Point<float>, juce::Point<float>>> bezierControlPoints;

        bool antialiasing = false;
    };

    int getNumRows() const
    {
        return numRows;
    }

    int getNumColumns() const
    {
        return numColumns;
    }

    const auto& getVertices() const { return vertices; }
    const auto& getHalfedges() const { return halfedges; }
    std::shared_ptr<Vertex> getVertex(int row, int column);
    std::shared_ptr<Halfedge> getHalfedge(std::shared_ptr<Vertex> tail, std::shared_ptr<Vertex> head);

    void applyTransform(juce::AffineTransform const& transform);
    void configureVertices(std::function<void(std::shared_ptr<Vertex> vertex)> callback);

    void draw(juce::Image image, juce::AffineTransform transform, juce::Colour backgroundColor = juce::Colours::transparentBlack);

    void makeConicGradient(juce::Rectangle<float> bounds);

private:
    int const numRows;
    int const numColumns;
    std::vector<std::shared_ptr<Vertex>> vertices;
    std::vector<std::shared_ptr<Halfedge>> halfedges;

    std::shared_ptr<Halfedge> addHalfedge(std::shared_ptr<Vertex> tail, std::shared_ptr<Vertex> head);

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
