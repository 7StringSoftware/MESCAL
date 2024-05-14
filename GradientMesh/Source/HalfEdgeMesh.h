#pragma once

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

        void dump() const
        {
            DBG("Vertex " << point.toString());
        }

        JUCE_LEAK_DETECTOR(Vertex)
    };

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

        void dump(String indent = {}) const
        {
            DBG(indent << "Halfedge " << (int)type << " " << tailVertex->point.toString() << " -> " << headVertex->point.toString());
        }
    };

    struct Patch
    {

        JUCE_LEAK_DETECTOR(Patch)
    };

    struct Subpath
    {
        std::vector<std::unique_ptr<Vertex>> vertices;
        std::vector<std::unique_ptr<Halfedge>> halfedges;
        std::vector<std::shared_ptr<Patch>> patches;

        void addPatches(juce::Point<float> center, int numPatchEdges);
    };

    std::vector<Subpath> subpaths;

    juce::Path path;

    void iterateSubpath(juce::Path::Iterator& it, Point<float> subpathStart);

//     struct Pimpl;
//     std::unique_ptr<Pimpl> pimpl;
};
