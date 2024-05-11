#pragma once

class Mesher
{
public:
    Mesher(Path&& p);
    ~Mesher();

    void updateMesh();

    void draw(juce::Image image, juce::AffineTransform transform);

//private:

    struct Edge;
    struct Vertex
    {
        juce::Point<float> point;
        std::vector<std::weak_ptr<Edge>> edges;
        juce::Colour color;

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

        JUCE_LEAK_DETECTOR(Vertex)
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

        Edge(Type type_, std::weak_ptr<Vertex> v0, std::weak_ptr<Vertex> v1) :
            type(type_),
            angle(v0.lock()->point.getAngleToPoint(v1.lock()->point))
        {
            vertices[0] = v0;
            vertices[1] = v1;
        }

        void dump()
        {
            juce::String line = "Edge ";

            if (auto lock = vertices[0].lock())
            {
                line << lock->point.toString();
            }
            else
            {
                line << "nullptr";
            }

            line << " -> ";

            if (auto lock = vertices[1].lock())
            {
                line << lock->point.toString();
            }
            else
            {
                line << "nullptr";
            }

            line << "   angle:" << angle / juce::MathConstants<float>::pi;

            DBG(line);

            //DBG("Edge " << vertices[0].lock()->point.toString() << " -> " << vertices[1].lock()->point.toString() << "   --   " << controlPoints[0].value_or(juce::Point<float>{}).toString() << " -> " << controlPoints[1].value_or(juce::Point<float>{}).toString());
        }

        std::array<std::weak_ptr<Vertex>, 2> vertices;
        std::array<std::optional<juce::Point<float>>, 2> controlPoints;
        float angle = 0.0f;

        JUCE_LEAK_DETECTOR(Edge)
    };

    struct Patch
    {
        Patch(std::array<std::weak_ptr<Edge>, 4>&& edges_) : edges(edges_)
        {
            juce::String line = "Patch ";
            for (auto const& edge : edges)
            {
                jassert(edge.lock());
                jassert(edge.lock()->vertices[0].lock());
                jassert(edge.lock()->vertices[1].lock());
                line << edge.lock()->vertices[0].lock()->point.toString() << " -> " << edge.lock()->vertices[1].lock()->point.toString() << "   --   ";
            }
            DBG(line);
        }

        std::array<std::weak_ptr<Edge>, 4> edges;

        JUCE_LEAK_DETECTOR(Patch)
    };

    struct Subpath
    {
        std::vector<std::shared_ptr<Vertex>> vertices;
        std::vector<std::shared_ptr<Edge>> edges;
        std::vector<std::shared_ptr<Patch>> patches;

        void addPatches(juce::Point<float> center);
    };

    std::vector<Subpath> subpaths;

    juce::Path path;

    void iterateSubpath(juce::Path::Iterator& it, std::shared_ptr<Vertex> subpathStart);

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
