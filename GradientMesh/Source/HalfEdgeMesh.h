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

class HalfEdgeMesh
{
public:
    HalfEdgeMesh(Path&& p);
    ~HalfEdgeMesh();

    void updateMesh(int numPatchEdges = 4);

    void draw(juce::Image image, juce::AffineTransform transform);

    //private:

    struct Halfedge;
    struct Vertex
    {
        juce::Point<float> point;
        Halfedge* halfedge = nullptr;
        juce::Colour color;

        Vertex() = default;

        Vertex(juce::Point<float> p) :
            point(p)
        {
        }

        Vertex(Vertex const& other) = default;

        bool operator==(Vertex const& other) const
        {
            return approximatelyEqual(point.x, other.point.x) && approximatelyEqual(point.y, other.point.y);
        }

        void dump() const;

        JUCE_LEAK_DETECTOR(Vertex)
    };

    using ControlPoints = std::pair<juce::Point<float>, juce::Point<float>>;

    struct Halfedge
    {
        enum class Type
        {
            unknown = -1,
            start,
            line,
            quadratic,
            cubic,
            close,
        } type = Type::unknown;

        Vertex* tailVertex = nullptr;
        Vertex* headVertex = nullptr;
        Halfedge* twin = nullptr;
        Halfedge* next = nullptr;
        Halfedge* previous = nullptr;

        std::optional<ControlPoints> controlPoints;

        juce::String print() const
        {
            juce::String line;
            line << "Halfedge type:" << (int)type << " " << tailVertex->point.toString() << " -> " << headVertex->point.toString() << "   previous:" << (previous ? previous->tailVertex->point.toString() : "null") << "   next:" << (next ? next->tailVertex->point.toString() : "null");
            return line;
        }

        void dump(String indent = {}) const
        {
            DBG(indent << print());
        }
    };

    struct Face
    {
        std::array<Halfedge*, 3> halfedges;
    };

    struct Patch
    {

        JUCE_LEAK_DETECTOR(Patch)
    };

    struct Subpath
    {
        std::vector<std::unique_ptr<Vertex>> vertices;
        std::vector<std::unique_ptr<Halfedge>> halfedges;
        std::vector<std::unique_ptr<Face>> faces;
        std::vector<std::shared_ptr<Patch>> patches;

        void iterateFaces(const std::vector<Halfedge*>& perimeterHalfedges);
        void addPatches(juce::Point<float> center, int numPatchEdges);
    };

    std::vector<Subpath> subpaths;

    juce::Path path;

    void iterateSubpath(juce::Path::Iterator& it, Point<float> subpathStart);

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
