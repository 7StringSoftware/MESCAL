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
        approximateQuadratic,
        cubic
    };

    enum class EdgePlacement
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

    static EdgePlacement opposite(EdgePlacement ep)
    {
        return EdgePlacement{ ((int)ep + 2) & 3 };
    }

    static EdgePlacement clockwiseFrom(EdgePlacement ep)
    {
        return EdgePlacement{ ((int)ep + 1) & 3 };
    }

    static CornerPlacement clockwiseFrom(CornerPlacement corner)
    {
        return CornerPlacement{ ((int)corner + 1) & 3 };
    }

    static EdgePlacement counterclockwiseFrom(EdgePlacement ep)
    {
        return EdgePlacement{ ((int)ep - 1) & 3 };
    }

    static std::pair<CornerPlacement, CornerPlacement> toCorners(EdgePlacement ep)
    {
        switch (ep)
        {
        default:
        case EdgePlacement::north:
            return std::pair<CornerPlacement, CornerPlacement>{ { CornerPlacement::topLeft }, { CornerPlacement::topRight } };
        case EdgePlacement::east:
            return std::pair<CornerPlacement, CornerPlacement>{ { CornerPlacement::topRight }, { CornerPlacement::bottomRight } };
        case EdgePlacement::south:
            return std::pair<CornerPlacement, CornerPlacement>{ { CornerPlacement::bottomRight }, { CornerPlacement::bottomLeft } };
        case EdgePlacement::west:
            return std::pair<CornerPlacement, CornerPlacement>{ { CornerPlacement::bottomLeft }, { CornerPlacement::topLeft } };
        }
    }

    static constexpr std::array<EdgePlacement, 4> edgePlacements{ EdgePlacement::north, EdgePlacement::east, EdgePlacement::south, EdgePlacement::west };
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

        juce::String toString(juce::String indent) const
        {
            juce::String text = indent + "Vertex ";
#if 0
            text << position.toString() << "\n";

            for (auto direction : directions)
            {
                text << indent << indent << directionNames[(int)direction] << " ";
                text << (halfedges[(int)direction] ? halfedges[(int)direction]->toString() : "null") << "\n";
            }
#endif

            return text;
        }

        void removeHalfedge(std::shared_ptr<Halfedge> halfedge);

        int getConnectionCount() const;

        size_t const index;
        juce::Point<float> position;
        std::vector<std::weak_ptr<Halfedge>> halfedges;
        juce::Colour color;
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
        }

        std::weak_ptr<Vertex> tail;
        std::weak_ptr<BezierControlPoint> b0, b1;
        std::weak_ptr<Vertex> head;
        float angle = 0.0f;

        std::weak_ptr<Halfedge> twin;

        std::weak_ptr<Patch> patch;

        EdgeType edgeType = EdgeType::cubic;

        juce::String toString() const
        {
            juce::String text = "Halfedge ";

            if (auto v = tail.lock())
                text << v->position.toString() << " ";
            else
                text << "null ";

            if (auto v = head.lock())
                text << v->position.toString() << " ";
            else
                text << "null ";

            text << (int)edgeType;

            return text;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Halfedge)
    };

    struct Patch
    {
        Patch(const std::vector<std::shared_ptr<Halfedge>>& halfedges_);
        ~Patch();

        void release()
        {
        }

        void update();

        auto getCornerVertex(CornerPlacement corner) const
        {
#if 0
            std::weak_ptr<Halfedge> halfedgeWeakPtr;
            switch (corner)
            {
            case CornerPlacement::topLeft:
                halfedgeWeakPtr = halfedges[(int)Direction::north];
                break;

            case CornerPlacement::topRight:
                halfedgeWeakPtr = halfedges[(int)Direction::east];
                break;

            case CornerPlacement::bottomRight:
                halfedgeWeakPtr = halfedges[(int)Direction::south];
                break;

            case CornerPlacement::bottomLeft:
                halfedgeWeakPtr = halfedges[(int)Direction::west];
                break;

            default:
                break;
            }

            if (auto halfedge = halfedgeWeakPtr.lock())
            {
                return halfedge->tail;
            }
#endif

            return std::weak_ptr<Vertex>{};
        }

        const juce::Path& getPath()
        {
            if (path.isEmpty())
                createPath();

            return path;
        }

        auto const& getHalfedges() const
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

        bool isConnected(EdgePlacement edgePlacement) const
        {
#if 0
            bool connected = true;
            if (auto halfedge = halfedges[(int)direction].lock())
            {
                if (auto v = halfedge->tail.lock())
                {
                    connected &= v->getHalfedge(direction).lock() != nullptr;
                }

                if (auto v = halfedge->head.lock())
                {
                    connected &= v->getHalfedge(direction).lock() != nullptr;
                }
            }
#endif

            return false;
        }

    private:
        juce::Path path;
        bool modified = true;

        std::vector<std::weak_ptr<Halfedge>> halfedges;
        std::array<juce::Colour, 4> cornerColors
        {
            juce::Colours::red, juce::Colours::green, juce::Colours::blue, juce::Colours::yellow
        };

        void createPath();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Patch)
    };

    GradientMesh();
    ~GradientMesh();

    void addPatch(juce::Rectangle<float> bounds);
    void addPatch(std::shared_ptr<Patch> patch);
    void removePatch(Patch* patch);
    std::shared_ptr<Patch> addConnectedPatch(Patch* sourcePatch, EdgePlacement sourceConnectedEdge);

    void applyTransform(const juce::AffineTransform& transform) noexcept;
    void draw(juce::Image image, juce::AffineTransform transform);

    void setVertexPosition(Vertex* vertex, juce::Point<float> position);
    void setEdgeType(Halfedge* edge, EdgeType edgeType);

    juce::Rectangle<float> getBounds() const noexcept;
    auto const& getPatches() const { return patches; }

    auto const& getVertices() const { return vertices; }
    auto const& getHalfedges() const { return halfedges; }

    juce::String toString() const;

    static std::unique_ptr<GradientMesh> pathToGrid(juce::Path const& path, 
        juce::AffineTransform const& transform, 
        float tolerance,
        float nominalPatchWidth,
        float nominalPatchHeight);

private:
    std::vector<std::shared_ptr<Vertex>> vertices;
    std::vector<std::shared_ptr<BezierControlPoint>> bezierControlPoints;
    std::vector<std::shared_ptr<Halfedge>> halfedges;
    std::vector<std::shared_ptr<Patch>> patches;

    std::shared_ptr<Vertex> addVertex(juce::Point<float> tail);
#if 0
    std::shared_ptr<Halfedge> addHalfedge(std::shared_ptr<Vertex> tail, std::shared_ptr<Vertex> head,
        std::shared_ptr<BezierControlPoint> b0,
        std::shared_ptr<BezierControlPoint> b1,
        Direction edgePlacement);
#endif
    std::shared_ptr<Halfedge> addHalfedge(std::shared_ptr<Vertex> tail, std::shared_ptr<Vertex> head);
    void removeHalfedge(std::shared_ptr<Halfedge> halfedge);
    void removeVertex(std::shared_ptr<Vertex> vertex);
    void removeBezier(std::shared_ptr<BezierControlPoint> bezier);

#if JUCE_DEBUG
    void check();
#endif

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
