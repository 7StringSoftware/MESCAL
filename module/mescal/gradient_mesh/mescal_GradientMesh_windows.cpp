namespace mescal
{

#ifdef __INTELLISENSE__

#include "mescal_GradientMesh_windows.h"
#include "../json/mescal_JSON.h"

#endif

    struct GradientMesh::Pimpl
    {
        Pimpl(GradientMesh& owner_) : owner(owner_)
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

        GradientMesh& owner;
        winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
        winrt::com_ptr<ID2D1GradientMesh> gradientMesh;
    };

    GradientMesh::GradientMesh(int numPatches) :
        pimpl(std::make_unique<Pimpl>(*this)),
        patches(numPatches)
    {
    }

    GradientMesh::~GradientMesh()
    {
    }

    void GradientMesh::applyTransform(const juce::AffineTransform& transform) noexcept
    {
    }

    void GradientMesh::addPatch(Patch& patch)
    {
        patches.push_back(patch);
    }

    juce::Rectangle<float> GradientMesh::getBounds() const noexcept
    {
        juce::Rectangle<float> bounds;
        for (auto const& patch : patches)
        {
            for (auto const& edge : patch.edges)
            {
                bounds = bounds.getUnion(juce::Rectangle<float>{ edge.tail.x, edge.tail.y, 1.0f, 1.0f });
            }

        }

        return bounds;
    }

    void GradientMesh::draw(juce::Image image, juce::AffineTransform transform)
    {
        auto toPOINT_2F = [&](juce::Point<float> point)
            {
                auto transformedPoint = point.transformedBy(transform);
                return D2D1_POINT_2F{ transformedPoint.x, transformedPoint.y };
            };
        auto toCOLOR_F = [](Color128 color)
            {
                return D2D1_COLOR_F{ color.red, color.green, color.blue, color.alpha };
            };

        std::vector<D2D1_GRADIENT_MESH_PATCH> d2dPatches;
        for (auto const& patch : patches)
        {
            d2dPatches.emplace_back(D2D1_GRADIENT_MESH_PATCH{});
            auto& d2dPatch = d2dPatches.back();

            d2dPatch.point00 = toPOINT_2F(patch.left().tail);
            d2dPatch.point30 = toPOINT_2F(patch.bottom().tail);
            d2dPatch.point33 = toPOINT_2F(patch.right().tail);
            d2dPatch.point03 = toPOINT_2F(patch.top().tail);

            d2dPatch.color00 = toCOLOR_F(patch.left().tailColor);
            d2dPatch.color30 = toCOLOR_F(patch.bottom().tailColor);
            d2dPatch.color33 = toCOLOR_F(patch.right().tailColor);
            d2dPatch.color03 = toCOLOR_F(patch.top().tailColor);

            {
                juce::Line<float> diagonal{ d2dPatch.point00.x, d2dPatch.point00.y, d2dPatch.point33.x, d2dPatch.point33.y };
                d2dPatch.point11 = toPOINT_2F(diagonal.getPointAlongLine(0.25f));
                d2dPatch.point22 = toPOINT_2F(diagonal.getPointAlongLine(0.75f));
            }
            {
                juce::Line<float> diagonal{ d2dPatch.point03.x, d2dPatch.point03.y, d2dPatch.point30.x, d2dPatch.point30.y };
                d2dPatch.point12 = toPOINT_2F(diagonal.getPointAlongLine(0.25f));
                d2dPatch.point21 = toPOINT_2F(diagonal.getPointAlongLine(0.75f));
            }

            d2dPatch.point10 = toPOINT_2F(patch.left().controlPoints.first);
            d2dPatch.point20 = toPOINT_2F(patch.left().controlPoints.second);

            d2dPatch.point31 = toPOINT_2F(patch.bottom().controlPoints.first);
            d2dPatch.point32 = toPOINT_2F(patch.bottom().controlPoints.second);

            d2dPatch.point23 = toPOINT_2F(patch.right().controlPoints.first);
            d2dPatch.point13 = toPOINT_2F(patch.right().controlPoints.second);

            d2dPatch.point02 = toPOINT_2F(patch.top().controlPoints.first);
            d2dPatch.point01 = toPOINT_2F(patch.top().controlPoints.second);

            d2dPatch.topEdgeMode = D2D1_PATCH_EDGE_MODE_ALIASED;
            d2dPatch.leftEdgeMode = D2D1_PATCH_EDGE_MODE_ALIASED;
            d2dPatch.bottomEdgeMode = D2D1_PATCH_EDGE_MODE_ALIASED;
            d2dPatch.rightEdgeMode = D2D1_PATCH_EDGE_MODE_ALIASED;
        }

        for (auto it = d2dPatches.begin(); it != d2dPatches.end(); ++it)
        {
            for (auto other = it + 1; other != d2dPatches.end(); ++other)
            {
                if (other->point00.x == it->point30.x && other->point00.y == it->point30.y)
                {
                    jassert(other->color00.a == it->color30.a);
                    jassert(other->color00.r == it->color30.r);
                    jassert(other->color00.g == it->color30.g);
                    jassert(other->color00.b == it->color30.b);
                }

                if (other->point03.x == it->point33.x && other->point03.y == it->point33.y)
                {
                    jassert(other->color03.a == it->color33.a);
                    jassert(other->color03.r == it->color33.r);
                    jassert(other->color03.g == it->color33.g);
                    jassert(other->color03.b == it->color33.b);
                }

            }
        }

        pimpl->createResources(image);

        if (pimpl->deviceContext && image.isValid())
        {
            pimpl->gradientMesh = {};
            pimpl->deviceContext->CreateGradientMesh(d2dPatches.data(), (uint32_t)d2dPatches.size(), pimpl->gradientMesh.put());

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

    GradientMesh::Patch::~Patch()
    {
    }

    void GradientMesh::setPatch(size_t index, Patch& patch)
    {
        patches[index] = patch;
    }

    GradientMesh::Patch& GradientMesh::Patch::operator=(Patch const& other)
    {
        edges = other.edges;

        return *this;
    }

    GradientMesh::Color128 GradientMesh::Color128::fromHSV(float hue, float saturation, float brightness, float alpha) noexcept
    {
        brightness = jlimit(0.0f, 1.0f, brightness);

        if (saturation <= 0)
            return { brightness, brightness, brightness, alpha };

        saturation = juce::jmin(1.0f, saturation);
        hue = ((hue - std::floor(hue)) * 360.0f) / 60.0f;
        auto f = hue - std::floor(hue);
        auto x = brightness * (1.0f - saturation);

        if (hue < 1.0f)   return Color128{ brightness, brightness * (1.0f - (saturation * (1.0f - f))), x, alpha, };
        if (hue < 2.0f)   return Color128{ brightness * (1.0f - saturation * f), brightness, x, alpha, };
        if (hue < 3.0f)   return Color128{ x, brightness, brightness * (1.0f - (saturation * (1.0f - f))), alpha, };
        if (hue < 4.0f)   return Color128{ x, brightness * (1.0f - saturation * f), brightness, alpha };
        if (hue < 5.0f)   return Color128{ brightness * (1.0f - (saturation * (1.0f - f))), x, brightness, alpha, };
        return                 Color128{ brightness, x, brightness * (1.0f - saturation * f), alpha, };
    }

} // namespace mescal

