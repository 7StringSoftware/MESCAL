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

using namespace juce;

#include <JuceHeader.h>
#include "liquidfun/Box2D.h"

//==============================================================================
/** A simple implementation of the b2Draw class, used to draw a Box2D world.

    To use it, simply create an instance of this class in your paint() method,
    and call its render() method.

    @tags{Box2D}
*/
class LiquidFunRenderer   : public b2Draw

{
public:
    LiquidFunRenderer() noexcept;

    void resize(juce::Rectangle<int> r);

    /** Renders the world.

        @param g        the context to render into
        @param world    the world to render
        @param box2DWorldLeft   the left coordinate of the area of the world to be drawn
        @param box2DWorldTop    the top coordinate of the area of the world to be drawn
        @param box2DWorldRight  the right coordinate of the area of the world to be drawn
        @param box2DWorldBottom the bottom coordinate of the area of the world to be drawn
        @param targetArea   the area within the target context onto which the source
                            world rectangle should be mapped
    */
    void render (Graphics& g,
                 b2World& world,
                    const Rectangle<float> boxWorldArea,
                 const Rectangle<float>& targetArea);

    // b2Draw methods:
    void DrawPolygon (const b2Vec2*, int32, const b2Color&) override;
    void DrawSolidPolygon (const b2Vec2*, int32, const b2Color&) override;
    void DrawCircle (const b2Vec2& center, float32 radius, const b2Color&) override;
    void DrawSolidCircle (const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color&) override;
    void DrawSegment (const b2Vec2& p1, const b2Vec2& p2, const b2Color&) override;
    void DrawTransform (const b2Transform& xf) override;

    /** Converts a b2Color to a juce Colour. */
    virtual Colour getColour (const b2Color&) const;
    /** Returns the thickness to use for drawing outlines. */
    virtual float getLineThickness() const;

    void DrawParticles(const b2Vec2* centers, float32 radius, const b2ParticleColor* colors, int32 count) override;
    void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color);

    void DrawString(int x, int y, const char* string, ...) {}
    void DrawString(const b2Vec2& p, const char* string, ...) {}

    juce::Image meshImage;

protected:
    Graphics* graphics;
    AffineTransform transform;
    static constexpr int spriteSize = 16;
    Image particleImage{ Image::ARGB, 16, 16, true };
    mescal::GradientMesh mesh{ 2, 2 };
    mescal::SpriteBatch spriteBatch;
    std::vector<mescal::Sprite> sprites;
    juce::Image outputImage{ Image::ARGB, 1000, 1000, true };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LiquidFunRenderer)
};

#define DRAW_STRING_NEW_LINE 25
