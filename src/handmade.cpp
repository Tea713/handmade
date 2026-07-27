#include <stdint.h>
#include <windows.h>
#include <winerror.h>
#include <xinput.h>

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

#define XINPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef XINPUT_GET_STATE(xinput_get_state);
XINPUT_GET_STATE(XInputGetStateStub) { return (ERROR_DEVICE_NOT_CONNECTED); }

#define XINPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef XINPUT_SET_STATE(xinput_set_state);
XINPUT_SET_STATE(XInputSetStateStub) { return (ERROR_DEVICE_NOT_CONNECTED); }

global_variable xinput_get_state *XInputGetState_ = XInputGetStateStub;
global_variable xinput_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

internal void Win32LoadXInput(void) {
    HMODULE xinput_library = LoadLibraryA("xinput1_4.dll");
    if (!xinput_library) {
        xinput_library = LoadLibraryA("xinput1_3.dll");
    }
    if (xinput_library) {
        XInputGetState = (xinput_get_state *)GetProcAddress(xinput_library, "XInputGetState");
        if (!XInputGetState) { XInputGetState = XInputGetStateStub;}
        XInputSetState = (xinput_set_state *)GetProcAddress(xinput_library, "XInputSetState");
        if (!XInputSetState) { XInputSetState = XInputSetStateStub;}
    }
}

internal win32_window_dimension GetWindowDimension(HWND Window) {
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

internal void RenderWeirdGradient(win32_offscreen_buffer *Buffer, int XOffset, int YOffset) {
    int Width = Buffer->Width;
    int Height = Buffer->Height;

    uint8_t *Row = (uint8_t *)Buffer->Memory;
    for (int Y = 0; Y < Height; ++Y) {
        uint32_t *Pixel = (uint32_t *)Row;
        for (int X = 0; X < Width; ++X) {
            uint8_t Blue = (X + XOffset);
            uint8_t Green = (Y + YOffset);
            *Pixel++ = (Green << 8) | Blue;
        }
        Row += Buffer->Pitch;
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
Win32DisplayBuffer(HDC DeviceContext, int WindowWidth, int WindowHeight, win32_offscreen_buffer *Buffer, int X, int Y) {
    // TODO: Aspect ratio correction
    StretchDIBits(
        DeviceContext,
        0,
        0,
        WindowWidth,
        WindowHeight,
        0,
        0,
        Buffer->Width,
        Buffer->Height,
        Buffer->Memory,
        &Buffer->Info,
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
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP: {
        uint32_t vk_code = WParam;
        bool was_down = (LParam & (1 << 30)) != 0;
        bool is_down = (LParam & (1 << 31)) == 0;
        if (was_down != is_down) {
            if (vk_code == 'W') {
            } else if (vk_code == 'A') {
            } else if (vk_code == 'S') {
            } else if (vk_code == 'D') {
            } else if (vk_code == 'Q') {
            } else if (vk_code == 'E') {
            } else if (vk_code == VK_UP) {
            } else if (vk_code == VK_LEFT) {
            } else if (vk_code == VK_DOWN) {
            } else if (vk_code == VK_RIGHT) {
            } else if (vk_code == VK_ESCAPE) {
                OutputDebugStringA("Escape: ");
                if (is_down) {
                    OutputDebugStringA("IsDown ");
                }
                if (was_down) {
                    OutputDebugStringA("WasDown");
                }
                OutputDebugStringA("\n");
            } else if (vk_code == VK_SPACE) {
            }
        }
        bool alt_key_was_down = (LParam & (1 << 29)) != 1;;
        if (vk_code == VK_F4 && alt_key_was_down) {
            Running = false;
        }
    } break;
    case WM_CLOSE: {
        // TODO: Handle this with a message to the user
        Running = false;
    } break;
    case WM_ACTIVATEAPP: {
        OutputDebugString("WM_ACTIVATEAPP\n");
    } break;
    case WM_PAINT: {
        PAINTSTRUCT Paint;
        HDC DeviceContext = BeginPaint(Window, &Paint);

        int X = Paint.rcPaint.left;
        int Y = Paint.rcPaint.top;
        int Width = Paint.rcPaint.right - Paint.rcPaint.left;
        int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

        win32_window_dimension Dimension = GetWindowDimension(Window);
        Win32DisplayBuffer(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBackBuffer, X, Y);

        EndPaint(Window, &Paint);
    } break;
    default: {
        // OutputDebugString("default\n");
        Result = DefWindowProc(Window, Message, WParam, LParam);
    } break;
    }

    return (Result);
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CommandLine, int ShowCode) {
    Win32LoadXInput();

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

        int XOffset = 0;
        int YOffset = 0;

        if (Window) {
            Running = true;
            while (Running) {

                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
                    if (Message.message == WM_QUIT) {
                        Running = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                // TODO: poll more frequently?
                for (DWORD controller_index = 0; controller_index < XUSER_MAX_COUNT; controller_index++) {
                    XINPUT_STATE state;
                    DWORD dw_result = XInputGetState(controller_index, &state);

                    if (dw_result == ERROR_SUCCESS) {
                        // Controller connected
                        XINPUT_GAMEPAD *pad = &state.Gamepad;

                        bool up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
                        bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
                        bool left_thumb = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
                        bool right_thumb = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
                        bool left_shoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool right_shoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool a = (pad->wButtons & XINPUT_GAMEPAD_A);
                        bool b = (pad->wButtons & XINPUT_GAMEPAD_B);
                        bool x = (pad->wButtons & XINPUT_GAMEPAD_X);
                        bool y = (pad->wButtons & XINPUT_GAMEPAD_Y);

                        int8_t left_trigger = pad->bLeftTrigger;
                        int8_t right_trigger = pad->bRightTrigger;

                        int16_t thumb_x = pad->sThumbLX;
                        int16_t thumb_y = pad->sThumbLY;

                        if (a) {
                            ++XOffset;
                        }

                        if (b) {
                            ++YOffset;
                        }
                    } else {
                        // Controller not connected
                    }
                }

                XINPUT_VIBRATION Vibration;
                Vibration.wLeftMotorSpeed = 60000;
                Vibration.wRightMotorSpeed = 60000;
                XInputSetState(0, &Vibration);

                RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

                HDC DeviceContext = GetDC(Window);
                win32_window_dimension Dimension = GetWindowDimension(Window);
                Win32DisplayBuffer(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBackBuffer, 0, 0);

                // ++XOffset;
                // ++YOffset;
            }
        } else {
            // TODO: Logging
        }
    } else {
        // TODO: Logging
    }

    return (0);
}
