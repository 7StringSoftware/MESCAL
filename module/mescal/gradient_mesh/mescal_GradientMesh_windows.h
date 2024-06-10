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

class GradientMesh
{
public:

    struct Color128
    {
        float red;
        float green;
        float blue;
        float alpha;

        juce::Colour toColour() const noexcept
        {
            return juce::Colour::fromFloatRGBA(red, green, blue, alpha);
        }

        static Color128 fromHSV(float hue, float saturation, float value, float alpha) noexcept;
    };

    enum class EdgePlacement
    {
        left, bottom, right, top
    };

    struct Edge
    {
        juce::Point<float> tail;
        Color128 tailColor;
        std::pair<juce::Point<float>, juce::Point<float>> controlPoints;
    };

    struct Patch
    {
        Patch() = default;
        Patch(Patch const&) = default;
        Patch(Patch&&) noexcept = default;
        ~Patch();

        Patch& operator= (Patch const&);

        std::array<Edge, 4> edges{};

        const Edge& left() const noexcept
        {
            return edges[(int)EdgePlacement::left];
        }

        const Edge& bottom() const noexcept
        {
            return edges[(int)EdgePlacement::bottom];
        }

        const Edge& right() const noexcept
        {
            return edges[(int)EdgePlacement::right];
        }

        const Edge& top() const noexcept
        {
            return edges[(int)EdgePlacement::top];
        }
    };

    GradientMesh(int numPatches = 0);
    ~GradientMesh();

    void clearPatches() noexcept { patches.clear(); }
    void addPatch(Patch& patch);
    auto const& getPatches() const noexcept { return patches; }
    void setPatch(size_t index, Patch& patch);
    juce::Rectangle<float> getBounds() const noexcept;

    void applyTransform(const juce::AffineTransform& transform) noexcept;
    void draw(juce::Image image, juce::AffineTransform transform);

private:
    std::vector<Patch> patches;

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
