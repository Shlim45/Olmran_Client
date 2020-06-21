/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jonathan Hawranko $
   $Notice: (C) Copyright 2020 by Jonathan Hawranko. All Rights Reserved. $
   ======================================================================== */

#include <windows.h>

int
GetTextSize (LPSTR a0)
{
    for (int iLoopCounter = 0; ;iLoopCounter++)
    {
        if (a0 [iLoopCounter] == '\0')
            return iLoopCounter;
    }
}

LPSTR
TextArray [] = {
    "Welcome to Olmran."
};

LRESULT CALLBACK
MainWindowCallback(HWND   Window,
                   UINT   Message,
                   WPARAM WParam,
                   LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n\r");
        } break;
        case WM_DESTROY:
        {
            OutputDebugStringA("WM_DESTROY\n\r");
            PostQuitMessage(0);
        } break;
        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n\r");
            DestroyWindow(Window);
        } break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n\r");

        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            
            TextOut (DeviceContext,
                     // Location of the text
                     TA_LEFT,
                     TA_TOP,
                     // Text to print
                     TextArray[0],
                     // Size of the text, my function gets this for us
                     GetTextSize(TextArray [0]));

            EndPaint (Window, &Paint);
        } break;
        
        default:
        {
//            OutputDebugStringA("default case\n\r");
            Result = DefWindowProc(Window, Message, WParam, LParam);
        }
    }

    return Result;
}

int CALLBACK
WinMain(
    HINSTANCE   Instance,
    HINSTANCE   PrevInstance,
    LPSTR       CommandLine,
    int         ShowCode)
{
    WNDCLASS WindowClass = {};
    // TODO(jon):  Check if any of this is needed.
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance;
//    WindowClass.hIcon = ;
//    WindowClass.hCursor = ;
//    WindowClass.hbrBackground = ;
//    WindowClass.lpszMenuName = ;
    WindowClass.lpszClassName = "OlmranWindowClass";
//    WindowClass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    
    if(RegisterClass(&WindowClass))
    {
        HWND WindowHandle =
            CreateWindowEx(
                0,
                WindowClass.lpszClassName,
                "Olmran Client",
                WS_OVERLAPPEDWINDOW|WS_SIZEBOX|WS_MAXIMIZE|WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                Instance,
                0);

        if(WindowHandle)
        {
            MSG Message;
            for(;;)
            {
                BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
                if(MessageResult > 0)
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                else
                    break;
            }
        }
        else
        {
            // TODO(jon): Logging (failed)
        }
    }
    else
    {
        // TODO(jon):  Logging (failed)
    }

    return 0;
}
