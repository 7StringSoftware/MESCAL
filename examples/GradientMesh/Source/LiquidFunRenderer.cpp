/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

//namespace juce
#include <JuceHeader.h>
using namespace juce;
#include "LiquidFunRenderer.h"

LiquidFunRenderer::LiquidFunRenderer() noexcept   : graphics (nullptr)
{
    SetFlags (e_shapeBit | e_particleBit);

    juce::ColourGradient radialGradient{ juce::Colours::lightskyblue, particleImage.getBounds().getCentre().toFloat(),
        juce::Colours::aliceblue, {0.0f, particleImage.getHeight() * 0.5f},
        true };

    {
        Graphics g{ particleImage };
        g.setGradientFill(radialGradient);
        //g.drawEllipse(particleImage.getBounds().reduced(2).toFloat(), 1.0f);
        g.fillEllipse(particleImage.getBounds().reduced(2).toFloat());
    }
}

void LiquidFunRenderer::resize(juce::Rectangle<int> r)
{
#if 0
    mesh.getVertex(0,0)->configure(r.getTopLeft().toFloat(), juce::Colours::deepskyblue);
    mesh.getVertex(1, 0)->configure(r.getTopRight().toFloat(), juce::Colours::lightskyblue);
    mesh.getVertex(1, 0)->configure(r.getBottomLeft().toFloat(), juce::Colours::darkblue);
    mesh.getVertex(1, 1)->configure(r.getBottomRight().toFloat(), juce::Colours::darkcyan);

    outputImage = juce::Image{ juce::Image::ARGB, r.getWidth(), r.getHeight(), true };
#endif
    //mesh.draw(meshImage, {});
}

void LiquidFunRenderer::render (Graphics& g, b2World& world,
    const Rectangle<float> boxWorldArea,
    const Rectangle<float>& target)
{
    graphics = &g;

    auto left = boxWorldArea.getX();
    auto topControlPoint = boxWorldArea.getBottom(); // switch Y & bottom sides to flip Y axis
    auto right = boxWorldArea.getRight();
    auto bottom = boxWorldArea.getY();
    transform = AffineTransform::fromTargetPoints (left,  topControlPoint,    target.getX(),     target.getY(),
                                                       right, topControlPoint,    target.getRight(), target.getY(),
                                                       left,  bottom, target.getX(),     target.getBottom());

    meshImage.clear(meshImage.getBounds());

    world.SetDebugDraw (this);
    world.DrawDebugData();
}

Colour LiquidFunRenderer::getColour (const b2Color& c) const
{
    return Colour::fromFloatRGBA (c.r, c.g, c.b, 1.0f);
}

float LiquidFunRenderer::getLineThickness() const
{
    return 0.1f;
}

void LiquidFunRenderer::DrawParticles(const b2Vec2* centers, float32 radius, const b2ParticleColor* colors, int32 count)
{
    if (sprites.size() < count)
    {
        sprites.resize(count);
    }

    radius *= 1.3f;

    for (int index = 0; index < count; ++index)
    {
        auto& sprite = sprites[index];
        juce::Rectangle<float> destination{ centers[index].x, centers[index].y, radius * 2.0f, radius * 2.0f };
        sprite.destination = destination.transformedBy(transform);
        sprite.source = particleImage.getBounds();
    }

    spriteBatch.setAtlas(particleImage);
    graphics->setColour(juce::Colours::transparentBlack);
    graphics->getInternalContext().fillRect(outputImage.getBounds(), true);
    spriteBatch.draw(outputImage, sprites, true);

    graphics->setColour(juce::Colours::black);
    graphics->drawImageAt(outputImage, 0, 0, false);

}

void LiquidFunRenderer::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
{
    juce::Rectangle<float> r{ size * 2.0f, size * 2.0f };
    graphics->setColour(getColour(color));
    graphics->fillRect(r.withCentre({ p.x,p.y }));
}

static void createPath (Path& p, const b2Vec2* vertices, int32 vertexCount)
{
    p.startNewSubPath (vertices[0].x, vertices[0].y);

    for (int i = 1; i < vertexCount; ++i)
        p.lineTo (vertices[i].x, vertices[i].y);

    p.closeSubPath();
}

void LiquidFunRenderer::DrawPolygon (const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    graphics->setColour (getColour (color));

    Path p;
    createPath (p, vertices, vertexCount);
    graphics->strokePath (p, PathStrokeType (getLineThickness()));
}

void LiquidFunRenderer::DrawSolidPolygon (const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
#if 1
    graphics->setColour (getColour (color).withAlpha(0.5f));

    Path p1, p2;
    createPath (p1, vertices, vertexCount);
    p2 = p1;
    p2.applyTransform(transform);
    graphics->fillPath (p2);
#endif
}

void LiquidFunRenderer::DrawCircle (const b2Vec2& center, float32 radius, const b2Color& color)
{
    Rectangle<float> r{ radius * 2.0f, radius * 2.0f };
    r.setCentre (center.x, center.y);
    r = r.transformedBy(transform);

    graphics->setColour (getColour (color));
    graphics->drawEllipse (r, getLineThickness());
}

void LiquidFunRenderer::DrawSolidCircle (const b2Vec2& center, float32 radius, const b2Vec2& /*axis*/, const b2Color& colour)
{
#if 1
    Rectangle<float> r{ radius * 2.0f, radius * 2.0f };
    r.setPosition(center.x, center.y);
    r.setCentre(center.x, center.y);
    r = r.transformedBy(transform);

    graphics->setColour (getColour (colour));
    graphics->drawEllipse(r, 1.0f);
#endif
}

void LiquidFunRenderer::DrawSegment (const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
    graphics->setColour (getColour (color).withAlpha(0.5f));
    graphics->drawLine (p1.x, p1.y, p2.x, p2.y, getLineThickness());
}

void LiquidFunRenderer::DrawTransform (const b2Transform&)
{
}

