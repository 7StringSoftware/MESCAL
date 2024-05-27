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
public:struct Patch;
    struct ControlPoint
    {
        explicit ControlPoint(Patch& patch_, size_t row_, size_t column_) :
            patch(patch_),
            row(row_), column(column_)
        {
        }

        auto& getPatch() const { return patch; }

        void addNeighbor(std::shared_ptr<ControlPoint> neighbor)
        {
            neighbors.push_back(neighbor);
        }

        void setPosition(juce::Point<float> position_);

        juce::Point<float>& getPosition()
        {
            return position;
        }

        bool hasColor() const
        {
            return color.has_value();
        }

        juce::Colour getColor()
        {
            return color.value_or(juce::Colours::white);
        }

        void setColor(juce::Colour color_)
        {
            color = color_;
            patch.update();
        }

        void applyTransform(const AffineTransform& transform) noexcept;

        void swapWith(std::shared_ptr<ControlPoint> other) noexcept;

        size_t const row;
        size_t const column;

    private:
        Patch& patch;
        juce::Point<float> position;
        std::optional<juce::Colour> color;
        std::vector<std::shared_ptr<ControlPoint>> neighbors;

        JUCE_DECLARE_WEAK_REFERENCEABLE(ControlPoint)
    };

    struct Edge
    {
        static constexpr size_t top = 0;
        static constexpr size_t right = 1;
        static constexpr size_t bottom = 2;
        static constexpr size_t left = 3;

        enum class Type
        {
            straight,
            quadratic,
            cubic
        } type;

        bool isValid() const noexcept
        {
            return corners.first && corners.second &&
                bezierControlPoints.first && bezierControlPoints.second;
        }

        std::pair<std::shared_ptr<ControlPoint>, std::shared_ptr<ControlPoint>> corners;
        std::pair<std::shared_ptr<ControlPoint>, std::shared_ptr<ControlPoint>> bezierControlPoints;
    };

    struct Patch
    {
        Patch();
        static std::shared_ptr<Patch> create();

        std::shared_ptr<Patch> createConnectedPatch(size_t edgePosition) const;

        void applyTransform(const AffineTransform& transform) noexcept;
        void flipHorizontally();

        void update();

        auto getControlPoint(size_t row, size_t column) const
        {
            return controlPoints[row * numColumns + column];
        }

        const Path& getPath() const
        {
            return path;
        }

        Edge getEdge(size_t edgePosition) const;
        Edge::Type getEdgeType(size_t edgePosition) const
        {
            return edgeTypes[edgePosition];
        }
        void setEdgeType(size_t edgePosition, Edge::Type type);

        static constexpr size_t numRows = 4;
        static constexpr size_t numColumns = 4;

    private:
        Path path;
        bool modified = true;

        std::array<std::shared_ptr<ControlPoint>, 16> controlPoints{};
        std::array <Edge::Type, 4 > edgeTypes
        {
            Edge::Type::cubic, Edge::Type::cubic, Edge::Type::cubic, Edge::Type::cubic
        };
    };

    GradientMesh();
    ~GradientMesh();

    void addPatch(std::shared_ptr<Patch> patch);
    void applyTransform(const AffineTransform& transform) noexcept;
    void draw(juce::Image image, juce::AffineTransform transform);

    juce::Rectangle<float> getBounds() const noexcept;
    auto const& getPatches() const { return patches; }

private:
    std::vector<std::shared_ptr<Patch>> patches;

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
