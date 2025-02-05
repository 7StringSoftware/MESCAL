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

#define _SILENCE_CLANG_COROUTINE_MESSAGE 1
#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <d2d1_3helper.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#include <d2d1effectauthor.h>
#include <d2d1effecthelpers.h>
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_graphics/native/juce_Direct2DMetrics_windows.h>
#include <juce_graphics/native/juce_EventTracing.h>
#include <juce_graphics/native/juce_DirectX_windows.h>
#include <juce_graphics/native/juce_Direct2DPixelDataPage_windows.h>
#include <juce_graphics/native/juce_Direct2DImage_windows.h>
#include <juce_graphics/native/juce_Direct2DGraphicsContext_windows.h>
#include <juce_graphics/native/juce_Direct2DImageContext_windows.h>

#include "mescal.h"

#include "json/mescal_JSON.cpp"
#include "resources/mescal_Resources_windows.cpp"
#include "gradients/mescal_MeshGradient_windows.cpp"
#include "gradients/mescal_ConicGradient_windows.cpp"
#include "effects/mescal_Effects_windows.cpp"
#include "effects/mescal_ImageEffectFilter_windows.cpp"
#include "images/mescal_Image_windows.cpp"
#include "utility/mescal_GPU_windows.cpp"
#include "sprites/mescal_SpriteBatch_windows.cpp"
