#pragma once

class Mesher
{
public:
    Mesher(Path&& p);
    ~Mesher();

    void updateMesh(int numPatchEdges = 4);

    void draw(juce::Image image, juce::AffineTransform transform);

//private:

    struct Edge;
    struct Vertex
    {
        juce::Point<float> point;
        std::vector<std::weak_ptr<Edge>> vertexConnectedEdges;
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

        std::weak_ptr<Edge> turnRight(int edgeIndex) const;

        JUCE_LEAK_DETECTOR(Vertex)
    };

    struct Endpoint
    {
        std::weak_ptr<Vertex> vertex;
        int edgeIndex = -1;
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
            endpoints{ { {v0, -1}, {v1, -1} }  },
            angle(v0.lock()->point.getAngleToPoint(v1.lock()->point))
        {
            DBG("Created edge " << uniqueID << " with angle " << angle);
        }

        void dump() const noexcept
        {
            juce::String line = "Edge #";
            
            line << uniqueID << " ";

            auto edgeVertices = getVertices();

            line << edgeVertices.first->point.toString();
            line << " -> ";
            line << edgeVertices.second->point.toString();
            line << "   angle:" << angle / juce::MathConstants<float>::pi;
            line << "   endpoint edge indices: " << endpoints[0].edgeIndex << " " << endpoints[1].edgeIndex;

            DBG(line);
        }

        std::pair<std::shared_ptr<Vertex>, std::shared_ptr<Vertex>> getVertices() const
        {
            return { endpoints[0].vertex.lock(), endpoints[1].vertex.lock() };
        }

        float getAngle(Vertex* start)const
        {
            if (auto v0 = endpoints[0].vertex.lock())
            {
                if (v0.get() == start)
                {
                    return angle;
                }
            }

            jassert(start == endpoints[1].vertex.lock().get());

            return angle + juce::MathConstants<float>::pi;
        }

        Endpoint& getEndpoint(Vertex* vertex)
        {
            if (auto v0 = endpoints[0].vertex.lock())
            {
                if (v0.get() == vertex)
                {
                    return endpoints[0];
                }
            }

            jassert(vertex == endpoints[1].vertex.lock().get());

            return endpoints[1];
        }

        std::weak_ptr<Edge> nextEdgeRightTurn(Vertex const* const start);

        static int uniqueIDCounter;
        int const uniqueID = uniqueIDCounter++;
        std::array<Endpoint, 2> endpoints;
        std::array<std::optional<juce::Point<float>>, 2> controlPoints;
        float angle = 0.0f;

        JUCE_LEAK_DETECTOR(Edge)
    };

    struct Patch
    {
        Patch(std::array<std::weak_ptr<Edge>, 4>&& edges_) : edges(edges_)
        {
        }

        std::array<std::weak_ptr<Edge>, 4> edges;

        JUCE_LEAK_DETECTOR(Patch)
    };

    struct Subpath
    {
        std::vector<std::shared_ptr<Vertex>> vertices;
        std::vector<std::shared_ptr<Edge>> edges;
        std::vector<std::shared_ptr<Patch>> patches;

        void addPatches(juce::Point<float> center, int numPatchEdges);
    };

    std::vector<Subpath> subpaths;

    juce::Path path;

    void iterateSubpath(juce::Path::Iterator& it, std::shared_ptr<Vertex> subpathStart);

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
