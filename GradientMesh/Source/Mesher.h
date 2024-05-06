#pragma once

struct Mesher
{
    Mesher(Path&& p);

    struct Vertex;
    struct Edge
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

        std::array<std::shared_ptr<Vertex>, 2> vertices;
        std::array<std::optional<juce::Point<float>>, 2> controlPoints;
    };

    struct Vertex
    {
        juce::Point<float> point;
        std::vector<std::shared_ptr<Edge>> edges;

        Vertex() = default;

        Vertex(juce::Point<float> p) :
            point(p)
        {
        }

        Vertex(Vertex const& other) :
            point(other.point)
        {
        }

        bool operator==(Vertex const& other) const
        {
            return approximatelyEqual(point.x, other.point.x) && approximatelyEqual(point.y, other.point.y);
        }
    };

    struct Triangle
    {
        std::array<juce::Point<float>, 3> vertices;
    };

    struct PatchBoundary
    {
        std::array<juce::Point<float>, 4> vertices;
    };

    struct Subpath
    {
        std::vector<std::shared_ptr<Vertex>> vertices;
        std::vector<std::shared_ptr<Edge>> edges;
        std::vector<PatchBoundary> quads;
    };

    std::vector<Subpath> subpaths;

    juce::Path path;

    void iterateSubpath(juce::Path::Iterator& it, std::shared_ptr<Vertex> subpathStart);
};
