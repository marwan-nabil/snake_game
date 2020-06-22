#include <Windows.h>
#include "basic_defs.h"
#include "renderer.h"
#include "draw.h"
#include "game.h"
#include "snake.h"

bool GlobalRunning;
color BackgroundColor = {230, 230, 230};

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

static LARGE_INTEGER PerfCounterFrequency;
static LARGE_INTEGER LastCounter;

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

