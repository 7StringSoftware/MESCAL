namespace mescal
{

    struct ConicGradient::Pimpl
    {
        Pimpl(ConicGradient& owner_) : owner(owner_)
        {
        }

        void createResources()
        {
            resources->create();
        }

        void draw(juce::Span<Stop> stops, juce::Image image, juce::AffineTransform transform, juce::Colour backgroundColor, bool replaceContents)
        {
            auto toPOINT_2F = [](juce::Point<float> p)
                {
                    return D2D1_POINT_2F{ p.x, p.y };
                };

            if (stops.size() < 2)
                return;

            auto patches = std::vector<D2D1_GRADIENT_MESH_PATCH>{ stops.size() - 1 };

            jassert(owner.radiusRange.getLength() > 0.0f);

            float outerRadius = owner.radiusRange.getEnd();
            float innerRadius = owner.radiusRange.getStart();
            juce::Point<float> center{ 0.0f, 0.0f };
            auto bottomEdgeMode = innerRadius > 0.0f ? D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ANTIALIASED : D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ALIASED;

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

                patch.color00 = { stop.outerColor.red, stop.outerColor.green, stop.outerColor.blue, stop.outerColor.alpha };
                patch.color30 = { stop.innerColor.red, stop.innerColor.green, stop.innerColor.blue, stop.innerColor.alpha };
                patch.color03 = { nextStop.outerColor.red, nextStop.outerColor.green, nextStop.outerColor.blue, nextStop.outerColor.alpha };
                patch.color33 = { nextStop.innerColor.red, nextStop.innerColor.green, nextStop.innerColor.blue, nextStop.innerColor.alpha };

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

            createResources();

            auto& deviceContext = resources->deviceContext;
            if (deviceContext && image.isValid())
            {
                gradientMesh = {};
                deviceContext->CreateGradientMesh(patches.data(), (uint32_t)patches.size(), gradientMesh.put());

                if (gradientMesh)
                {
                    //if (auto pixelData = dynamic_cast<juce::Direct2DPixelData*>(image.getPixelData().get()))
                    if (auto pixelData = dynamic_cast<juce::Direct2DPixelData*>(image.getPixelData()))
                    {
                        //if (auto bitmap = pixelData->getFirstPageForDevice(pimpl->resources->adapter->direct2DDevice))
                        if (auto bitmap = pixelData->getFirstPageForContext(deviceContext))
                        {
                            deviceContext->SetTarget(bitmap);
                            deviceContext->BeginDraw();
                            deviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
                            if (replaceContents)
                            {
                                deviceContext->Clear(juce::D2DUtilities::toCOLOR_F(backgroundColor));
                            }

                            deviceContext->DrawGradientMesh(gradientMesh.get());

                            deviceContext->EndDraw();
                            deviceContext->SetTarget(nullptr);
                        }
                    }
                }
            }
        }

        ConicGradient& owner;
        juce::SharedResourcePointer<DirectXResources> resources;
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

    void ConicGradient::addStop(float angle, Color128 innerColor, Color128 outerColor)
    {
        stops.emplace_back(Stop{ angle, innerColor, outerColor });
        sortStops();
    }

    void ConicGradient::addStops(juce::Span<ConicGradient::Stop> newStops)
    {
        for (auto const& newStop : newStops)
        {
            stops.emplace_back(Stop{ newStop.angle, newStop.innerColor, newStop.outerColor });
        }
        sortStops();
    }

    void ConicGradient::draw(juce::Image image, juce::AffineTransform transform, juce::Colour backgroundColor, bool replaceContents)
    {
        pimpl->draw(stops, image, transform, backgroundColor, replaceContents);
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

    void ConicGradient::setStopColor(size_t index, Color128 innerColor, Color128 outerColor)
    {
        stops[index].innerColor = innerColor;
        stops[index].outerColor = outerColor;
    }

} // namespace mescal
