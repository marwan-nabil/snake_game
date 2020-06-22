#include <Windows.h>
#include <list>

#include "basic_defs.h"
#include "game.h"
#include "platform.h"


static bool GlobalRunning;
static color BackgroundColor = {230, 230, 230};
static i32 MonitorRefreshHz = 30;
static f32 GameUpdateHz = ((f32) MonitorRefreshHz); // target frame rate
static offscreen_buffer GlobalBackBuffer;
static LARGE_INTEGER PerfCounterFrequency;
static LARGE_INTEGER LastCounter;


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

void
HandleKeyboardMessages(MSG Message, snake_game *Game)
{
    u32 VKCode = (u32) Message.wParam;
    bool WasDown = ((Message.lParam & (1 << 30)) != 0);
    bool IsDown = ((Message.lParam & (1 << 31)) == 0);
    // ignore repeating messages of the same key if there's no transitions in between them
    if(WasDown != IsDown)
    {
        if(VKCode == VK_UP)
        {
            KeyPressHandler(Game, KeyUp);
        }
        else if(VKCode == VK_DOWN)
        {
            KeyPressHandler(Game, KeyDown);
        }
        else if(VKCode == VK_LEFT)
        {
            KeyPressHandler(Game, KeyLeft);
        }
        else if(VKCode == VK_RIGHT)
        {
            KeyPressHandler(Game, KeyRight);
        }
    }
}

void
HandleWM_PAINT(HWND Window)
{
    PAINTSTRUCT Paint;
    HDC DevContext = BeginPaint(Window, &Paint);
    WindowDimension Dimension = GetWindowDimension(Window);
    UpdateWindowFromBuffer(DevContext,
                           Dimension.Width, Dimension.Height,
                           &GlobalBackBuffer);
    EndPaint(Window, &Paint);
}

LRESULT CALLBACK
WindowProc(HWND   Window,
           UINT   Message,
           WPARAM wParam,
           LPARAM lParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_SIZE:
        {
        } break;

        case WM_DESTROY:
        {
            GlobalRunning = false;
        } break;

        case WM_CLOSE:
        {
            GlobalRunning = false;
        } break;

        case WM_PAINT:
        {
            HandleWM_PAINT(Window);
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            Assert(!"keyboard input came in through a non-dispatch message")
        } break;

        default:
        {
            Result = DefWindowProcA(Window, Message, wParam, lParam);
        } break;
    }
    return Result;
}

void
ProcessPendingMessages(snake_game *Game)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYUP:
            case WM_KEYDOWN:
            {
                HandleKeyboardMessages(Message, Game);
            } break;
            default:
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            } break;
        }
    }
}

WNDCLASSA
CreateWindowClass(HINSTANCE hInstance)
{
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = "SnakeGameWindowClass";
    WindowClass.hCursor = LoadCursor(0, IDC_CROSS);
    return(WindowClass);
}

inline f32
GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    f32 Result = ((f32) (End.QuadPart - Start.QuadPart) / (f32) PerfCounterFrequency.QuadPart);
    return(Result);
}

inline LARGE_INTEGER
GetWallClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

void SyncFrame(f32 TargetFramePeriod)
{
    f32 TimeSpentWorking = GetSecondsElapsed(LastCounter, GetWallClock());
    // synchronizing with the target frame rate
    f32 SecondsElapsedForFrame = TimeSpentWorking;
    if(SecondsElapsedForFrame < TargetFramePeriod)
    {
        DWORD msSleep = (DWORD) (1000.0f * (TargetFramePeriod - SecondsElapsedForFrame));
        if(msSleep > 0)
        {
            Sleep(msSleep);
        }

        SecondsElapsedForFrame = GetSecondsElapsed(LastCounter, GetWallClock());

        if(SecondsElapsedForFrame < TargetFramePeriod)
        {
            // TODO: log missing a frame here

        }

        // spinlock for the rest of the interval
        while(SecondsElapsedForFrame < TargetFramePeriod)
        {
            SecondsElapsedForFrame = GetSecondsElapsed(LastCounter, GetWallClock());
        }
    }
    else
    {
        // we've missed a frame
    }

    // reset stats each frame
    LastCounter = GetWallClock();
}

i32 CALLBACK
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR     lpCmdLine,
        i32       nCmdShow)
{
    snake_game Game = NewGame(30, 20);
    ResizeBackBuffer(&GlobalBackBuffer, 960, 540);
    WNDCLASSA WindowClass = CreateWindowClass(hInstance);
    f32 TargetFramePeriod = 1.0f / GameUpdateHz;
    QueryPerformanceFrequency(&PerfCounterFrequency);
    LastCounter = GetWallClock();

    if(!RegisterClassA(&WindowClass))
    {
        Assert(0);
    }

    HWND Window = CreateWindowExA(0, "SnakeGameWindowClass", "Snake Game",
                                  WS_VISIBLE | WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  0, 0, hInstance, 0);
    if(!Window)
    {
        Assert(0);
    }

    GlobalRunning = true;
    while(GlobalRunning)
    {
        f64 TimeDelta = TargetFramePeriod;
        // handle all window messages
        ProcessPendingMessages(&Game);
        DrawRectangle(BackgroundColor, 0, 0, Game.Width, Game.Height);
        DrawGame(&Game);
        UpdateGame(&Game, TimeDelta);
        HDC DevContext = GetDC(Window);
        WindowDimension Dim = GetWindowDimension(Window);
        UpdateWindowFromBuffer(DevContext, Dim.Width, Dim.Height, &GlobalBackBuffer);

        // sleep the rest of the frame
        SyncFrame(TargetFramePeriod);
    }
}

