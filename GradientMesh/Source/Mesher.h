#pragma once

struct Mesher
{
    Mesher(Path&& p);

    struct Vertex
    {
        enum class Type
        {
            unknown = -1,
            start,
            line,
            quadratic,
            cubic,
            close,
        } type;

        juce::Point<float> point;
        juce::Rectangle<float> containingBounds;

        Vertex(Type t, juce::Point<float> p, juce::Rectangle<float> containingBounds_) :
            type(t),
            point(p),
            containingBounds(containingBounds_)
        {
        }

        bool operator<(const Vertex& other) const
        {
            return getScore() < other.getScore();
        }

        bool operator>(const Vertex& other) const
        {
            return getScore() > other.getScore();
        }

        bool operator==(const Vertex& other) const
        {
            return approximatelyEqual(point.x, other.point.x) && approximatelyEqual(point.y, other.point.y);
        }

        bool operator!=(const Vertex& other) const
        {
            return !(*this == other);
        }

        bool operator<=(const Vertex& other) const
        {
            return *this < other || *this == other;
        }

        bool operator>=(const Vertex& other) const
        {
            return *this > other || *this == other;
        }

        float getScore() const noexcept
        {
            return point.y * containingBounds.getWidth() + point.x;
        }
    };

    juce::Array<Vertex> perimeterVertices;
    juce::SortedSet<Vertex> xySortedVertices;
    juce::SortedSet<float> xPositions;
    juce::SortedSet<float> yPositions;
    juce::Path path;
};


