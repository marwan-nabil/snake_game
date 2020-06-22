#pragma once
#include <Windows.h>
#include "basic_defs.h"


struct color {
    u8 Red, Green, Blue;
};

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

extern f32 GameUpdateHz;
extern offscreen_buffer GlobalBackBuffer;

void ResizeBackBuffer(offscreen_buffer *Buffer, i32 Width, i32 Height);

void UpdateWindowFromBuffer(HDC DevContext, i32 WindowWidth, i32 WindowHeight, offscreen_buffer *Buffer);

WindowDimension GetWindowDimension(HWND Window);

void RenderRectangle(color Color, f64 X, f64 Y, f64 Width, f64 Height);