#include "GradientMeshDemo.h"
#include "Commander.h"

GradientMeshDemo::GradientMeshDemo()
    : displayComponent(*this)
{
    setOpaque(true);

    gradientMesh = std::make_unique<GradientMesh>();
    innerMesh = std::make_unique<GradientMesh>();

    addAndMakeVisible(displayComponent);

    updater.addAnimator(fadeInAnimator);
    fadeInAnimator.start();
}

GradientMeshDemo::~GradientMeshDemo()
{
}

void GradientMeshDemo::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void GradientMeshDemo::paintOverChildren(Graphics& g)
{
#if 0
    for (auto& patch : gradientMesh->getPatches())
    {
        juce::Rectangle<float> r{ patch.left().tail, patch.right().tail };
        if (r.contains(getMouseXYRelative().toFloat()))
        {
            g.setColour(juce::Colours::white);
            g.drawRect(r);

            for (auto const& edge : patch.edges)
            {
                g.setColour(edge.tailColor.toColour());
                g.fillEllipse(edge.tail.x - 5.0f, edge.tail.y - 5.0f, 10.0f, 10.0f);

                g.setColour(juce::Colours::white);
                g.fillEllipse(edge.controlPoints.first.x - 5.0f, edge.controlPoints.first.y - 5.0f, 10.0f, 10.0f);
                g.fillEllipse(edge.controlPoints.second.x - 5.0f, edge.controlPoints.second.y - 5.0f, 10.0f, 10.0f);
            }
        }
    }
#endif

    g.setColour(juce::Colours::black);
    g.setFont({ getHeight() * 0.45f, juce::Font::bold });
    g.drawText("MESCAL", getLocalBounds(), juce::Justification::centred);
}

void GradientMeshDemo::resized()
{
    displayComponent.setBounds(getLocalBounds());
}

void GradientMeshDemo::createGradientMesh()
{
    gradientMesh->clearPatches();

    juce::Point<float> topLeftCorner;
    float patchWidth = (float)getWidth() * 0.25;
    float patchHeight = (float)getHeight() * 0.25f;
    while (topLeftCorner.x <= (float)getWidth() && topLeftCorner.y <= (float)getHeight())
    {
        GradientMesh::Patch patch;

        auto setEdge = [&](GradientMesh::EdgePlacement edgePlacement, juce::Line<float> line)
            {
                auto& edge = patch.edges[(int)edgePlacement];
                edge.tail = line.getStart();

                auto colorX = line.getStartX() / (float)getWidth();
                auto normalizedY = line.getStart().y / (float)getHeight();

                auto hue = 0.5f * ((float)std::sin((colorX * 0.1f + normalizedY * 0.1f + displayComponent.timestamp * 0.0125) * juce::MathConstants<float>::twoPi) * 0.5f + 0.5f);
                auto saturation = std::cos(normalizedY * juce::MathConstants<float>::twoPi) * 0.5f + 0.5f;

                edge.tailColor = mescal::GradientMesh::Color128::fromHSV(hue,
                    saturation,
                    displayComponent.gradientOpacity,
                    displayComponent.gradientOpacity);

                float offset = 50.0f;
                auto angle = line.getAngle();
                if (edge.tail.x < patchWidth || edge.tail.x >(float)getWidth() - patchWidth ||
                    edge.tail.y < patchHeight || edge.tail.y >(float)getHeight() - patchHeight)
                {
                    offset = 0.0f;
                }
                angle += juce::MathConstants<float>::halfPi;
                edge.controlPoints =
                {
                    line.getPointAlongLineProportionally(0.25f).getPointOnCircumference(offset, angle + juce::MathConstants<float>::halfPi),
                    line.getPointAlongLineProportionally(0.75f).getPointOnCircumference(offset, angle - juce::MathConstants<float>::halfPi)
                };
            };

        std::array<juce::Point<float>, 4> patchCorners
        {
            topLeftCorner.translated(patchWidth, patchHeight),
            topLeftCorner.translated(patchWidth, 0.0f),
            topLeftCorner,
            topLeftCorner.translated(0.0f, patchHeight)
        };
        size_t patchCornerIndex = 0;
        juce::Line<float> line{ patchCorners[2], patchCorners[3] };
        for (auto edgePlacement : { GradientMesh::EdgePlacement::left, GradientMesh::EdgePlacement::bottom, GradientMesh::EdgePlacement::right, GradientMesh::EdgePlacement::top })
        {
            setEdge(edgePlacement, line);

            line = juce::Line<float>{ line.getEnd(), patchCorners[patchCornerIndex++] };
        }

        topLeftCorner += { patchWidth, 0.0f };
        if (topLeftCorner.x >= (float)getWidth())
        {
            topLeftCorner.x = 0.0f;
            topLeftCorner += { 0.0f, patchHeight};
        }

        gradientMesh->addPatch(patch);
    }
}

GradientMeshDemo::DisplayComponent::DisplayComponent(GradientMeshDemo& owner_) :
    owner(owner_)
{
    setInterceptsMouseClicks(false, false);
}

void GradientMeshDemo::DisplayComponent::paint(juce::Graphics& g)
{
    if (meshImage.isNull() || meshImage.getWidth() != getWidth() || meshImage.getHeight() != getHeight())
    {
        meshImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
    }

    owner.gradientMesh->draw(meshImage, {});
    g.drawImageAt(meshImage, 0, 0);
}


