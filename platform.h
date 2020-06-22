#pragma once
#include <Windows.h>
#include "basic_defs.h"

struct WindowDimension {
    i32 Width;
    i32 Height;
};

struct offscreen_buffer {
    BITMAPINFO Info;
    void *Memory;
    i32 Width;  // in pixels
    i32 Height; // in pixels
    i32 BytesPerPixel;
    i32 Pitch;  // bytes per row
};

void RenderRectangle(color Color, f64 X, f64 Y, f64 Width, f64 Height);