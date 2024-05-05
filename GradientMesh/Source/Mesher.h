#pragma once

struct Mesher
{
    Mesher(Path&& p);

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

        juce::Line<float> line;
        std::array<std::optional<juce::Point<float>>, 2> controlPoints;
    };

    struct Vertex
    {
        juce::Point<float> point;

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

    struct Quadrilateral
    {
        std::array<juce::Point<float>, 4> vertices;
    };

    juce::Array<Vertex> perimeterVertices;
    juce::Array<Edge> perimeterEdges;
    juce::Array<Quadrilateral> quads;
    juce::Path path;
};


