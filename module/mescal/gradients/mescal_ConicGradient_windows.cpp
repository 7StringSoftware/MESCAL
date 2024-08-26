namespace mescal
{

#ifdef __INTELLISENSE__

#include "mescal_GradientMesh_windows.h"

#endif

    struct ConicGradient::Pimpl
    {
        Pimpl(ConicGradient& owner_) : owner(owner_)
        {
        }

        void createResources(juce::Image image)
        {
            if (!deviceContext)
            {
                if (auto pixelData = dynamic_cast<juce::Direct2DPixelData*>(image.getPixelData()))
                {
                    if (auto adapter = pixelData->getAdapter())
                    {
                        winrt::com_ptr<ID2D1DeviceContext1> deviceContext1;
                        if (const auto hr = adapter->direct2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
                            deviceContext1.put());
                            FAILED(hr))
                        {
                            jassertfalse;
                            return;
                        }

                        deviceContext = deviceContext1.as<ID2D1DeviceContext2>();
                    }
                }
            }
        }

        void draw(juce::Span<Stop> stops, juce::Image image, juce::AffineTransform transform)
        {
            auto toPOINT_2F = [](juce::Point<float> p)
                {
                    return D2D1_POINT_2F{ p.x, p.y };
                };

            jassert(stops.size() >= 2);
            auto patches = std::vector<D2D1_GRADIENT_MESH_PATCH>{ stops.size() - 1 };

            float outerRadius = owner.radiusRange.getEnd();
            float innerRadius = owner.radiusRange.getStart();
            juce::Point<float> center{ 0.0f, 0.0f };
            auto bottomEdgeMode = innerRadius >= 0.0f ? D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ANTIALIASED : D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ALIASED;

            for (size_t index = 0; index < patches.size(); ++index)
            {
                auto& stop = stops[index];
                auto& nextStop = stops[index + 1];
                auto& patch = patches[index];

                auto outerArcStart = center.getPointOnCircumference(outerRadius, stop.angle);
                auto outerArcEnd = center.getPointOnCircumference(outerRadius, nextStop.angle);
                auto innerArcStart = center.getPointOnCircumference(innerRadius, stop.angle);
                auto innerArcEnd = center.getPointOnCircumference(innerRadius, nextStop.angle);

                /*

                Direct2D mesh gradient patch layout:

               P00------------P03
                |              |
                |              |
                |              |
                |              |
                |              |
                |              |
               P30------------P33

                To make an arc segment with an inner and outer radius:

                P00--------.
                |          \
                |           \
               P30-----      \
                       \      \
                        \      \
                         \      |
                         |      |
                         |      |
                         |      |
                         P33---P03

                If the inner radius zero, collapse the inner two points to the center.

               P00--------.
                |          \
                |           \
                |            \
                |             \
                |              \
                |               \
                |               |
                |               |
               P30-------------P03
               P33

                */
                patch.point00 = toPOINT_2F(outerArcStart.transformedBy(transform));
                patch.point03 = toPOINT_2F(outerArcEnd.transformedBy(transform));
                patch.point30 = toPOINT_2F(innerArcStart.transformedBy(transform));
                patch.point33 = toPOINT_2F(innerArcEnd.transformedBy(transform));

                patch.color00 = { stop.color128.red, stop.color128.green, stop.color128.blue, stop.color128.alpha };
                patch.color03 = { nextStop.color128.red, nextStop.color128.green, nextStop.color128.blue, nextStop.color128.alpha };
                patch.color33 = patch.color03;
                patch.color30 = patch.color00;

                auto arcAngle = nextStop.angle - stop.angle;
                auto controlPointDistance = 4.0f * std::tan(arcAngle * 0.25f) / 3.0f;
                auto controlPointAngle0 = stop.angle + juce::MathConstants<float>::halfPi; // control points should be tangential to the arc segment
                auto controlPointAngle1 = nextStop.angle - juce::MathConstants<float>::halfPi;

                auto outerControlPointDistance = outerRadius * controlPointDistance;
                auto outerArcControlPoint0 = outerArcStart.getPointOnCircumference(outerControlPointDistance, controlPointAngle0);
                auto outerArcControlPoint1 = outerArcEnd.getPointOnCircumference(outerControlPointDistance, controlPointAngle1);

                auto innerControlPointDistance = innerRadius * controlPointDistance;
                auto innerArcControlPoint0 = innerArcStart.getPointOnCircumference(innerControlPointDistance, controlPointAngle0);
                auto innerArcControlPoint1 = innerArcEnd.getPointOnCircumference(innerControlPointDistance, controlPointAngle1);

                patch.point01 = toPOINT_2F(outerArcControlPoint0.transformedBy(transform));
                patch.point02 = toPOINT_2F(outerArcControlPoint1.transformedBy(transform));
                patch.point31 = toPOINT_2F(innerArcControlPoint0.transformedBy(transform));
                patch.point32 = toPOINT_2F(innerArcControlPoint1.transformedBy(transform));

                patch.point10 = patch.point00;
                patch.point13 = patch.point03;
                patch.point20 = patch.point30;
                patch.point23 = patch.point33;

                patch.point11 = patch.point00;
                patch.point12 = patch.point03;
                patch.point21 = patch.point30;
                patch.point22 = patch.point33;

                patch.topEdgeMode = D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ANTIALIASED;
                patch.rightEdgeMode = D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ALIASED;
                patch.bottomEdgeMode = bottomEdgeMode;
                patch.leftEdgeMode = D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ALIASED;
            }

            patches.front().leftEdgeMode = D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ANTIALIASED;
            patches.back().rightEdgeMode = D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ANTIALIASED;

            createResources(image);

            if (deviceContext && image.isValid())
            {
                gradientMesh = {};
                deviceContext->CreateGradientMesh(patches.data(), (uint32_t)patches.size(), gradientMesh.put());

                if (gradientMesh)
                {
                    if (auto pixelData = dynamic_cast<juce::Direct2DPixelData*>(image.getPixelData()))
                    {
                        if (auto bitmap = pixelData->getAdapterD2D1Bitmap())
                        {
                            deviceContext->SetTarget(bitmap);
                            deviceContext->BeginDraw();
                            deviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
                            deviceContext->Clear({ 0.0f, 0.0f, 0.0f, 0.0f });

                            deviceContext->DrawGradientMesh(gradientMesh.get());

                            deviceContext->EndDraw();
                            deviceContext->SetTarget(nullptr);
                        }
                    }
                }
            }
        }

        ConicGradient& owner;
        winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
        winrt::com_ptr<ID2D1GradientMesh> gradientMesh;
    };

    ConicGradient::ConicGradient() :
        pimpl(std::make_unique<Pimpl>(*this))
    {
    }

    ConicGradient::~ConicGradient()
    {
    }

    void ConicGradient::clearStops()
    {
        stops.clear();
    }

    void ConicGradient::addStop(float angle, Color128 color128)
    {
        stops.emplace_back(Stop{ angle, color128 });
        sortStops();
    }

    void ConicGradient::addStops(juce::Span<ConicGradient::Stop> newStops)
    {
        for (auto const& newStop : newStops)
        {
            stops.emplace_back(Stop{ newStop.angle, Color128{ newStop.color128 } });
        }
        sortStops();
    }

    void ConicGradient::draw(juce::Image image, juce::AffineTransform transform)
    {
        pimpl->draw(stops, image, transform);
    }

    void ConicGradient::sortStops()
    {
        std::sort(stops.begin(), stops.end(), [](auto const& lhs, auto const& rhs)
            {
                return lhs.angle < rhs.angle;
            });
    }

    void ConicGradient::setStopAngle(size_t index, float angle)
    {
        stops[index].angle = angle;
    }

} // namespace mescal
