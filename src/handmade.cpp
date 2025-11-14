#include <windows.h>
#include <wingdi.h>
#include <winuser.h>

LRESULT MainWindowCallback(HWND Window, UINT Message, WPARAM WParam,
                           LPARAM LParam) {
  LRESULT Result = 0;

  switch (Message) {
    case WM_SIZE: {
      OutputDebugString("WM_SIZE");
    } break;
    case WM_DESTROY: {
      OutputDebugString("WM_DESTROY");
    } break;
    case WM_CLOSE: {
      PostQuitMessage(0);
      OutputDebugString("WM_CLOSE");
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
      PatBlt(DeviceContext, X, Y, Width, Height, WHITENESS);
      EndPaint(Window, &Paint);
    } break;
    default: {
      OutputDebugString("default");
      Result = DefWindowProc(Window, Message, WParam, LParam);
    } break;
  }

  return (Result);
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CommandLine,
                   int ShowCode) {
  WNDCLASS WindowClass = {};

  // WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = MainWindowCallback;
  WindowClass.hInstance = Instance;
  // WindowClass.hIcon;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";

  if (RegisterClass(&WindowClass)) {
    HWND WindowHanlde = CreateWindowEx(
        0, WindowClass.lpszClassName, "Handmade Hero",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);

    if (WindowHanlde) {
      MSG Message;
      for (;;) {
        BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
        if (MessageResult > 0) {
          TranslateMessage(&Message);
          DispatchMessage(&Message);
        } else {
          break;
        }
      }
    } else {
      // TODO: Logging
    }
  } else {
    // TODO: Logging
  }

  return (0);
}
