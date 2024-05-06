#pragma once

class Mesher
{
public:
    Mesher(Path&& p);
    ~Mesher();

    void draw(juce::Image image, juce::AffineTransform transform);

private:

    struct Edge;
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

    struct Patch
    {
        std::array<std::shared_ptr<Edge>, 4> edges;
    };

    struct Subpath
    {
        std::vector<std::shared_ptr<Vertex>> vertices;
        std::vector<std::shared_ptr<Edge>> edges;
        std::vector<Patch> patches;
    };

    std::vector<Subpath> subpaths;

    juce::Path path;

    void iterateSubpath(juce::Path::Iterator& it, std::shared_ptr<Vertex> subpathStart);

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
