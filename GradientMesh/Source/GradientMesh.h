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

    struct Patch;

    enum class EdgeType
    {
        unknown = -1,
        straight,
        quadratic,
        cubic
    };

    class ControlPoint
    {
    public:
        explicit ControlPoint(Patch& patch_, size_t row_, size_t column_) :
            patch(patch_),
            row(row_), column(column_)
        {
        }

        ~ControlPoint()
        {
        }

        auto& getPatch() const { return patch; }

        void addNeighbor(std::shared_ptr<ControlPoint> neighbor)
        {
            neighbors.push_back(neighbor);
        }

        auto const& getNeighbors() const { return neighbors; }

        enum UpdateType
        {
            update,
            doNotUpdate
        };

        virtual void setPosition(juce::Point<float> position_, UpdateType updateType = update);

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
        }

        void applyTransform(const AffineTransform& transform) noexcept;

        void swapWith(std::shared_ptr<ControlPoint> other) noexcept;

        virtual void release()
        {
            neighbors.clear();
        }

        size_t const row;
        size_t const column;

    protected:
        Patch& patch;
        juce::Point<float> position;
        std::optional<juce::Colour> color;
        std::vector<std::shared_ptr<ControlPoint>> neighbors;

        static void updateNeighbors(ControlPoint* cp, UpdateType updateType)
            {
                for (auto& neighbor : cp->neighbors)
                {
                    neighbor->position = cp->position;

                    if (updateType == ControlPoint::update)
                    {
                        neighbor->patch.update();
                    }
                }
            };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlPoint)
    };

    struct Edge;
    class BezeierControlPoint : public ControlPoint
    {
    public:
        explicit BezeierControlPoint(Patch& patch_, size_t row_, size_t column_, std::shared_ptr<ControlPoint> corner_) :
            ControlPoint(patch_, row_, column_), corner(corner_)
        {
        }

        void configure(Edge* edge_, std::shared_ptr<BezeierControlPoint> buddy_)
        {
            edge = edge_;
            buddy = buddy_;
        }

        void release() override
        {
            corner.reset();
            buddy.reset();

            ControlPoint::release();
        }

        void setPosition(juce::Point<float> position_, UpdateType updateType = update) override;

    protected:
        std::shared_ptr<ControlPoint> corner;
        Edge* edge = nullptr;
        std::shared_ptr<BezeierControlPoint> buddy;
    };

    struct Edge
    {
        static constexpr size_t top = 0;
        static constexpr size_t right = 1;
        static constexpr size_t bottom = 2;
        static constexpr size_t left = 3;

        EdgeType type = EdgeType::cubic;

        bool isValid() const noexcept
        {
            return corners.first && corners.second &&
                bezierControlPoints.first && bezierControlPoints.second;
        }

        static size_t getOppositePosition(size_t position)
        {
            return (position + 2) % 4;
        }

        std::pair<std::shared_ptr<ControlPoint>, std::shared_ptr<ControlPoint>> corners;
        std::pair<std::shared_ptr<BezeierControlPoint>, std::shared_ptr<BezeierControlPoint>> bezierControlPoints;
    };

    struct Patch
    {
        Patch();
        ~Patch();

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

        const Edge* const getEdge(size_t edgePosition) const;
        const Edge* const getOppositeEdge(size_t edgePosition) const;
        void setEdgeType(size_t edgePosition, EdgeType type);
        EdgeType getEdgeType(size_t edgePosition) const
        {
            return edges[edgePosition]->type;
        }

        static constexpr size_t numRows = 4;
        static constexpr size_t numColumns = 4;

    private:
        Path path;
        bool modified = true;

        std::array<std::shared_ptr<ControlPoint>, 16> controlPoints{};
        std::array <std::unique_ptr<Edge>, 4 > edges{};
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
