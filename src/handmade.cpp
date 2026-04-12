#include <stdint.h>
#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

struct win32_offscreen_buffer {
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct win32_window_dimension {
    int Width;
    int Height;
};

win32_window_dimension GetWindowDimension(HWND Window) {
    win32_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return Result;
}

// TODO: Just global for now
global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackBuffer;

internal void RenderWeirdGradient(win32_offscreen_buffer Buffer, int XOffset, int YOffset) {
    int Width = Buffer.Width;
    int Height = Buffer.Height;

    uint8_t *Row = (uint8_t *)Buffer.Memory;
    for (int Y = 0; Y < Height; ++Y) {
        uint32_t *Pixel = (uint32_t *)Row;
        for (int X = 0; X < Width; ++X) {
            uint8_t Blue = (X + XOffset);
            uint8_t Green = (Y + YOffset);
            *Pixel++ = (Green << 8) | Blue;
        }
        Row += Buffer.Pitch;
    }
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height) {
    // TODO: Bulletproof this
    // Maybe don't free first, free after, then free first if that fails

    if (Buffer->Memory) {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    int BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Buffer->Width * Buffer->Height) * BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    Buffer->Pitch = Width * BytesPerPixel;
}

internal void
Win32DisplayBuffer(HDC DeviceContext, int WindowWidth, int WindowHeight, win32_offscreen_buffer Buffer, int X, int Y) {
    // TODO: Aspect ratio correction
    StretchDIBits(
        DeviceContext,
        0,
        0,
        WindowWidth,
        WindowHeight,
        0,
        0,
        Buffer.Width,
        Buffer.Height,
        Buffer.Memory,
        &Buffer.Info,
        DIB_RGB_COLORS,
        SRCCOPY);
}

LRESULT Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
    LRESULT Result = 0;

    switch (Message) {
    case WM_SIZE: {
    } break;
    case WM_DESTROY: {
        // TODO: Handle this error
        Running = false;
    } break;
    case WM_CLOSE: {
        // TODO: Handle this with a message to the user
        Running = false;
    } break;
    case WM_ACTIVATEAPP: {
        OutputDebugString("WM_ACTIVATEAPP");
    } break;
    case WM_PAINT: {
        PAINTSTRUCT Paint;
        HDC DeviceContext = BeginPaint(Window, &Paint);

        int X = Paint.rcPaint.left;
        int Y = Paint.rcPaint.top;
        int Width = Paint.rcPaint.right - Paint.rcPaint.left;
        int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

        win32_window_dimension Dimension = GetWindowDimension(Window);
        Win32DisplayBuffer(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackBuffer, X, Y);

        EndPaint(Window, &Paint);
    } break;
    default: {
        OutputDebugString("default");
        Result = DefWindowProc(Window, Message, WParam, LParam);
    } break;
    }

    return (Result);
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CommandLine, int ShowCode) {
    WNDCLASS WindowClass = {};

    Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

    WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    // WindowClass.hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClass(&WindowClass)) {
        HWND Window = CreateWindowEx(
            0,
            WindowClass.lpszClassName,
            "Handmade Hero",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            Instance,
            0);

        if (Window) {
            Running = true;
            while (Running) {
                HDC DeviceContext = GetDC(Window);

                int XOffset;
                int YOffset;

                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
                    if (Message.message == WM_QUIT) {
                        Running = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);

                win32_window_dimension Dimension = GetWindowDimension(Window);
                Win32DisplayBuffer(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackBuffer, 0, 0);

                ++XOffset;
                ++YOffset;
            }
        } else {
            // TODO: Logging
        }
    } else {
        // TODO: Logging
    }

    return (0);
}
