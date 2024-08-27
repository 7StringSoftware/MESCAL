/*
    Copyright (c) 2024 7 String Software

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#pragma once

/*
BEGIN_JUCE_MODULE_DECLARATION

ID:               mescal
vendor:           7StringSoftware
version:          0.0.1
name:             MESCAL
description:      JUCE Direct2D bonus features
website:          https://github.com/mattgonzalez/MESCAL/
license:          MIT

dependencies:     juce_graphics, juce_core, juce_events

END_JUCE_MODULE_DECLARATION

 */

#include <array>

namespace mescal
{
    #include "json/mescal_JSON.h"
    #include "gradients/mescal_MeshGradient_windows.h"
    #include "gradients/mescal_ConicGradient_windows.h"
    #include "sprites/mescal_SpriteBatch_windows.h"
    #include "effects/mescal_Effects_windows.h"
}
