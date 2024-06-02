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

    enum class Direction
    {
        north = 0,
        east,
        south,
        west
    };

    enum class CornerPlacement
    {
        topLeft = 0,
        topRight,
        bottomRight,
        bottomLeft
    };

    static constexpr std::array<CornerPlacement, 4> corners{ CornerPlacement::topLeft, CornerPlacement::topRight, CornerPlacement::bottomRight, CornerPlacement::bottomLeft };

    static Direction opposite(Direction direction)
    {
        return Direction{ ((int)direction + 2) & 3 };
    }

    static Direction clockwiseFrom(Direction direction)
    {
        return Direction{ ((int)direction + 1) & 3 };
    }

    static CornerPlacement clockwiseFrom(CornerPlacement corner)
    {
        return CornerPlacement{ ((int)corner + 1) & 3 };
    }

    static Direction counterclockwiseFrom(Direction direction)
    {
        return Direction{ ((int)direction - 1) & 3 };
    }

    static std::pair<CornerPlacement, CornerPlacement> toCorners(Direction direction)
    {
        switch (direction)
        {
        default:
        case Direction::north:
            return std::pair<CornerPlacement, CornerPlacement>{ { CornerPlacement::topLeft }, { CornerPlacement::topRight } };
        case Direction::east:
            return std::pair<CornerPlacement, CornerPlacement>{ { CornerPlacement::topRight }, { CornerPlacement::bottomRight } };
        case Direction::south:
            return std::pair<CornerPlacement, CornerPlacement>{ { CornerPlacement::bottomRight }, { CornerPlacement::bottomLeft } };
        case Direction::west:
            return std::pair<CornerPlacement, CornerPlacement>{ { CornerPlacement::bottomLeft }, { CornerPlacement::topLeft } };
        }
    }

    static constexpr std::array<Direction, 4> directions{ Direction::north, Direction::east, Direction::south, Direction::west };
    static constexpr std::array<const char*, 4> directionNames{ "north", "east", "south", "west" };

    struct Halfedge;
    struct Vertex
    {
        explicit Vertex(juce::Point<float> position_, GradientMesh& mesh_, size_t index_) :
            position(position_),
            mesh(mesh_),
            index(index_)
        {
        }

        std::shared_ptr<Halfedge> getHalfedge(Direction edgePlacement) const
        {
            return halfedges[(size_t)edgePlacement];
        }

        String toString(String indent) const
        {
            String text = indent + "Vertex ";
            text << position.toString() << "\n";

            for (auto direction : directions)
            {
                text << indent << indent << directionNames[(int)direction] << " ";
                text << (halfedges[(int)direction] ? halfedges[(int)direction]->toString() : "null") << "\n";
            }

            return text;
        }

        size_t const index;
        juce::Point<float> position;
        std::array<std::shared_ptr<Halfedge>, 4> halfedges;
        GradientMesh& mesh;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Vertex)
    };

    struct BezierControlPoint
    {
        explicit BezierControlPoint(juce::Point<float> position_, GradientMesh& mesh_) :
            position(position_),
            mesh(mesh_)
        {
        }

        juce::Point<float> position;
        GradientMesh& mesh;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BezierControlPoint)
    };

    struct Patch;
    struct Halfedge
    {
        Halfedge() = default;

        void release()
        {
            tail.reset();
            b0.reset();
            b1.reset();
            head.reset();
            twin.reset();
            patch.reset();
        }

        std::shared_ptr<Vertex> tail;
        std::shared_ptr<BezierControlPoint> b0, b1;
        std::shared_ptr<Vertex> head;

        std::shared_ptr<Halfedge> twin;

        std::shared_ptr<Patch> patch;

        EdgeType edgeType = EdgeType::cubic;

        String toString() const
        {
            String text = "Halfedge ";
            text << tail->position.toString() << " " << head->position.toString() << " " << (int)edgeType;
            return text;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Halfedge)
    };

    struct Patch
    {
        Patch(std::array<std::shared_ptr<Halfedge>, 4>& halfedges_);
        ~Patch();

        void release()
        {
            for (auto& halfedge : halfedges)
                halfedge->release();
        }

        void update();

        auto getCornerVertex(CornerPlacement corner) const
        {
            switch (corner)
            {
            case CornerPlacement::topLeft:
                return halfedges[(int)Direction::north]->tail;

            case CornerPlacement::topRight:
                return halfedges[(int)Direction::east]->tail;

            case CornerPlacement::bottomRight:
                return halfedges[(int)Direction::south]->tail;

            case CornerPlacement::bottomLeft:
                return halfedges[(int)Direction::west]->tail;

            default:
                break;
            }

            return std::shared_ptr<Vertex>{};
        }

        const Path& getPath() const
        {
            return path;
        }

        const auto& getHalfedges() const
        {
            return halfedges;
        }

        const auto& getColors() const
        {
            return cornerColors;
        }

        void setColor(CornerPlacement corner, juce::Colour color)
        {
            cornerColors[(int)corner] = color;
        }

        bool isConnected(Direction direction) const
        {
            auto halfedge = halfedges[(int)direction];

            return halfedge->tail->getHalfedge(direction) != nullptr && halfedge->head->getHalfedge(direction) != nullptr;
        }

    private:
        Path path;
        bool modified = true;

        std::array<std::shared_ptr<Halfedge>, 4> halfedges;
        std::array<juce::Colour, 4> cornerColors
        {
            juce::Colours::red, juce::Colours::green, juce::Colours::blue, juce::Colours::yellow
        };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Patch)
    };

    GradientMesh();
    ~GradientMesh();

    void addPatch(juce::Rectangle<float> bounds);
    void addPatch(std::shared_ptr<Patch> patch);
    std::shared_ptr<Patch> addConnectedPatch(Patch* sourcePatch, Direction direction);

    void applyTransform(const AffineTransform& transform) noexcept;
    void draw(juce::Image image, juce::AffineTransform transform);

    void setVertexPosition(Vertex* vertex, juce::Point<float> position);
    void setEdgeType(Halfedge* edge, EdgeType edgeType);

    juce::Rectangle<float> getBounds() const noexcept;
    auto const& getPatches() const { return patches; }

    auto const& getVertices() const { return vertices; }
    auto const& getHalfedges() const { return halfedges; }

    String toString() const;

private:
    std::vector<std::shared_ptr<Vertex>> vertices;
    std::vector<std::shared_ptr<BezierControlPoint>> bezierControlPoints;
    std::vector<std::shared_ptr<Halfedge>> halfedges;
    std::vector<std::shared_ptr<Patch>> patches;

    std::shared_ptr<Vertex> addVertex(juce::Point<float> tail);
    std::shared_ptr<Halfedge> addHalfedge(std::shared_ptr<Vertex> tail, std::shared_ptr<Vertex> head,
        std::shared_ptr<BezierControlPoint> b0,
        std::shared_ptr<BezierControlPoint> b1,
        Direction edgePlacement);

#if JUCE_DEBUG
    void check();
#endif

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
