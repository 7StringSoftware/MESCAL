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

        ConicGradient& owner;
        std::vector<D2D1_GRADIENT_MESH_PATCH> patches;
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

    void ConicGradient::draw(int numSegments, juce::Image image, juce::AffineTransform transform)
    {
        auto toPOINT_2F = [](juce::Point<float> p)
            {
                return D2D1_POINT_2F{ p.x, p.y };
            };

        juce::zerostruct(pimpl->patches);

        jassert(numSegments >= 4);

        pimpl->patches = std::vector<D2D1_GRADIENT_MESH_PATCH>{ (size_t)numSegments };

        constexpr float circleApproximationConstant = (-4.0f + 4.0f * juce::MathConstants<float>::sqrt2) / 3.0f;
        float angle = 0.0f;
        float angleStep = juce::MathConstants<float>::twoPi / (float)numSegments;
        float radiusX = bounds.getWidth() * 0.5f;
        float radiusY = bounds.getHeight() * 0.5f;
        juce::Point<float> center;
        double colorPosition = 0.0;
        double colorStep = 1.0 / (double)numSegments;
        transform = juce::AffineTransform::scale(radiusX, radiusY).translated(bounds.getCentre()).followedBy(transform);
        for (auto& patch : pimpl->patches)
        {
            auto p1 = center.getPointOnCircumference(1.0f, angle);
            auto p2 = center.getPointOnCircumference(1.0f, angle + juce::MathConstants<float>::halfPi);

            auto patchStartColor = juce::Colour::fromHSV(colorPosition, 1.0f, 1.0f, 1.0f);
                //gradient.getColourAtPosition(colorPosition);
            auto patchStopColor = juce::Colour::fromHSV(colorPosition + colorStep, 1.0f, 1.0f, 1.0f);//gradient.getColourAtPosition(colorPosition + colorStep);

            patch.point00 = toPOINT_2F(p1.transformedBy(transform));
            patch.point03 = toPOINT_2F(p2.transformedBy(transform));
            patch.point33 = toPOINT_2F({ bounds.getCentre() });
            patch.point30 = patch.point33;

            patch.color00 = juce::D2DUtilities::toCOLOR_F(patchStartColor);
            patch.color03 = juce::D2DUtilities::toCOLOR_F(patchStopColor);
            patch.color33 = patch.color03;
            patch.color30 = patch.color00;

            auto rotation = juce::AffineTransform::rotation(angle);
            auto cp0 = juce::Point<float>{ 0.5f + 0.5f * circleApproximationConstant, -1.0f }.transformedBy(rotation);
            auto cp1 = juce::Point<float>{ 1.0f, -0.5f * circleApproximationConstant }.transformedBy(rotation);

            patch.point01 = toPOINT_2F(cp0.transformedBy(transform));
            patch.point02 = toPOINT_2F(cp1.transformedBy(transform));

            patch.point10 = patch.point00;
            patch.point13 = patch.point03;

            patch.point20 = patch.point30;
            patch.point31 = patch.point30;

            patch.point23 = patch.point33;
            patch.point32 = patch.point33;

            patch.point11 = patch.point00;
            patch.point12 = patch.point03;
            patch.point21 = patch.point30;
            patch.point22 = patch.point33;

            patch.topEdgeMode = D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ANTIALIASED;
            patch.rightEdgeMode = D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ALIASED;
            patch.bottomEdgeMode = D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ALIASED;
            patch.leftEdgeMode = D2D1_PATCH_EDGE_MODE::D2D1_PATCH_EDGE_MODE_ALIASED;

            angle += angleStep;
            colorPosition += colorStep;
        }

        pimpl->createResources(image);

        if (pimpl->deviceContext && image.isValid())
        {
            pimpl->gradientMesh = {};
            pimpl->deviceContext->CreateGradientMesh(pimpl->patches.data(), (uint32_t)pimpl->patches.size(), pimpl->gradientMesh.put());

            if (pimpl->gradientMesh)
            {
                if (auto pixelData = dynamic_cast<juce::Direct2DPixelData*>(image.getPixelData()))
                {
                    if (auto bitmap = pixelData->getAdapterD2D1Bitmap())
                    {
                        pimpl->deviceContext->SetTarget(bitmap);
                        pimpl->deviceContext->BeginDraw();
                        pimpl->deviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
                        pimpl->deviceContext->Clear({ 0.0f, 0.0f, 0.0f, 0.0f });

                        if (pimpl->gradientMesh)
                        {
                            pimpl->deviceContext->DrawGradientMesh(pimpl->gradientMesh.get());
                        }

                        pimpl->deviceContext->EndDraw();
                        pimpl->deviceContext->SetTarget(nullptr);
                    }
                }
            }
        }
    }

    void ConicGradient::setColourGradient(juce::ColourGradient gradient_)
    {
        gradient = gradient_;
    }

    void ConicGradient::setBounds(juce::Rectangle<float> bounds_)
    {
        bounds = bounds_;
    }

} // namespace mescal

