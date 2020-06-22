#include <Windows.h>
#include "basic_defs.h"
#include "renderer.h"

static i32 MonitorRefreshHz = 30;
f32 GameUpdateHz = ((f32) MonitorRefreshHz); // target frame rate
offscreen_buffer GlobalBackBuffer;

void
ResizeBackBuffer(offscreen_buffer *Buffer,
                 i32 Width, i32 Height)
{
    // deallocate previous bitmap memory on size change
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    // resize global bitmap dimensions
    Buffer->Width = Width;
    Buffer->Height = Height;

    Buffer->BytesPerPixel = 4;	// RGBA
    Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel;

    // filling out the BITMAPINFO struct, the header is the most important
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height; // top-down scheme
    Buffer->Info.bmiHeader.biPlanes = 1; // 1 color plane, no separate RGB channels
    Buffer->Info.bmiHeader.biBitCount = (WORD) (Buffer->BytesPerPixel * 8); // bits per pixel
    Buffer->Info.bmiHeader.biCompression = BI_RGB; // no compression
    Buffer->Info.bmiHeader.biSizeImage = 0;
    Buffer->Info.bmiHeader.biXPelsPerMeter = 0;
    Buffer->Info.bmiHeader.biYPelsPerMeter = 0;
    Buffer->Info.bmiHeader.biClrUsed = 0;
    Buffer->Info.bmiHeader.biClrImportant = 0;

    // allocate bitmap memory
    Buffer->Memory = VirtualAlloc(0, Buffer->BytesPerPixel * Buffer->Width * Buffer->Height,
                                  MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void
UpdateWindowFromBuffer(HDC DevContext, i32 WindowWidth, i32 WindowHeight,
                       offscreen_buffer *Buffer)
{
    if((WindowWidth >= Buffer->Width * 1.2) &&
       (WindowHeight >= Buffer->Height * 1.2)) // window bigger than buffer dimensions
    {
        StretchDIBits(DevContext, // destination device
                      0, 0, WindowWidth, WindowHeight, // destination dimensions (stretch to window dimensions)
                      0, 0, Buffer->Width, Buffer->Height, // source buffer stuff
                      Buffer->Memory, // source buffer memory
                      &Buffer->Info,
                      DIB_RGB_COLORS, // usage
                      SRCCOPY); // raster operation
    }
    else // window same size or smaller than backbuffer
    {
        i32 OffsetX = 10;
        i32 OffsetY = 10;

        // paint buffer offsets inside the window with black (4 regions)
        PatBlt(DevContext, 0, 0, WindowWidth, OffsetY, BLACKNESS);
        PatBlt(DevContext, 0, 0, OffsetX, WindowHeight, BLACKNESS);
        PatBlt(DevContext, OffsetX + Buffer->Width, 0, WindowWidth, WindowHeight, BLACKNESS);
        PatBlt(DevContext, 0, OffsetY + Buffer->Height, OffsetX + Buffer->Width, WindowHeight, BLACKNESS);

        StretchDIBits(
            DevContext, // destination device
            OffsetX, OffsetY, Buffer->Width, Buffer->Height, // destination dimensions (no stretching)
            0, 0, Buffer->Width, Buffer->Height, // source buffer stuff
            Buffer->Memory, // source buffer memory
            &Buffer->Info,
            DIB_RGB_COLORS,	// usage
            SRCCOPY); // raster operation
    }
}

WindowDimension
GetWindowDimension(HWND Window)
{
    RECT ClientRect;
    WindowDimension dim;
    GetClientRect(Window, &ClientRect);
    dim.Height = ClientRect.bottom - ClientRect.top;
    dim.Width = ClientRect.right - ClientRect.left;
    return dim;
};


void RenderRectangle(color Color, f64 X, f64 Y, f64 Width, f64 Height)
{
    i32 MinX = (i32) X;
    i32 MinY = (i32) Y;
    i32 MaxX = (i32) Width + MinX;
    i32 MaxY = (i32) Height + MinY;

    if(MinX < 0)
    {
        MinX = 0;
    }

    if(MaxX > GlobalBackBuffer.Width)
    {
        MaxX = GlobalBackBuffer.Width;
    }

    if(MinY < 0)
    {
        MinY = 0;
    }

    if(MaxY > GlobalBackBuffer.Height)
    {
        MaxY = GlobalBackBuffer.Height;
    }

    u8 *EndOfBuffer = (u8 *) GlobalBackBuffer.Memory +
        ((long) GlobalBackBuffer.Pitch * (long) GlobalBackBuffer.Height);

    u32 PixelColor = ((u32) (Color.Red) << 16) |
        ((u32) (Color.Green) << 8) |
        ((u32) (Color.Blue) << 0);

    u8 *Row = ((u8 *) GlobalBackBuffer.Memory +
               (long) MinY * (long) GlobalBackBuffer.Pitch +
               (long) MinX * (long) GlobalBackBuffer.BytesPerPixel);

    for(int Y = MinY;
        Y < MaxY;
        Y++)
    {
        u32 *Pixel = (u32 *) Row;

        for(int X = MinX;
            X < MaxX;
            X++)
        {
            *Pixel++ = PixelColor;
        }

        Row += GlobalBackBuffer.Pitch;
    }
}
