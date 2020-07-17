/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jonathan Hawranko $
   $Notice: (C) Copyright 2020 by Jonathan Hawranko. All Rights Reserved. $
   ======================================================================== */

#include <windows.h>

#define ID_EDITCHILD 100

#define global_variable static
#define local_persist static
#define internal static

global_variable bool running;

LRESULT CALLBACK
MainWindowCallback(HWND   Window,
                   UINT   Message,
                   WPARAM WParam,
                   LPARAM LParam)
{
    LRESULT Result = 0;
    local_persist HWND GameOutput;
    
    TCHAR OutputString[] =  TEXT("Lorem ipsum dolor sit amet, consectetur ")
        TEXT("adipisicing elit, sed do eiusmod tempor " )
        TEXT("incididunt ut labore et dolore magna " )
        TEXT("aliqua. Ut enim ad minim veniam, quis " )
        TEXT("nostrud exercitation ullamco laboris nisi " )
        TEXT("ut aliquip ex ea commodo consequat. Duis " )
        TEXT("aute irure dolor in reprehenderit in " )
        TEXT("voluptate velit esse cillum dolore eu " )
        TEXT("fugiat nulla pariatur. Excepteur sint " )
        TEXT("occaecat cupidatat non proident, sunt " )
        TEXT("in culpa qui officia deserunt mollit " )
        TEXT("anim id est laborum.");
    
    TCHAR WelcomeMessage[] = TEXT("This is the MUD Client for Olmran\r\n")
        TEXT("There are many like it, but this one is special!\r\n")
        TEXT("I leared C++ while creating it... maybe?");
    
    
    switch(Message)
    {
        case WM_CREATE:
        {
            GameOutput = CreateWindowEx(
                0, TEXT("EDIT"),   // predefined class 
                NULL,         // no window title 
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
                ES_LEFT | ES_MULTILINE | ES_READONLY, 
                0, 0, 0, 0,   // set size in WM_SIZE message 
                Window,         // parent window 
                (HMENU) ID_EDITCHILD,   // edit control ID 
                (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE), 
                NULL);        // pointer not needed 
            
            // Add text to the window. 
            SendMessage(GameOutput, WM_SETTEXT, 0, (LPARAM) WelcomeMessage); 
        } break; 
        /*
        case WM_COMMAND:
        {
            switch (WParam) 
            {
                case IDM_EDUNDO:
                {
                    // Send WM_UNDO only if there is something to be undone. 
                    
                    if (SendMessage(GameOutput, EM_CANUNDO, 0, 0)) 
                        SendMessage(GameOutput, WM_UNDO, 0, 0); 
                    else 
                    {
                        MessageBox(GameOutput, 
                                   L"Nothing to undo.", 
                                   L"Undo notification", 
                                   MB_OK); 
                    }
                } break; 
                
                case IDM_EDCUT:
                {
                    SendMessage(GameOutput, WM_CUT, 0, 0); 
                } break; 
                
                case IDM_EDCOPY:
                {
                    SendMessage(GameOutput, WM_COPY, 0, 0); 
                }    break; 
                
                case IDM_EDPASTE:
                {
                    SendMessage(GameOutput, WM_PASTE, 0, 0); 
                }    break; 
                
                case IDM_EDDEL:
                {
                    SendMessage(GameOutput, WM_CLEAR, 0, 0); 
                }    break; 
                
                case IDM_ABOUT:
                {
                    DialogBox(hInst,                // current instance 
                              L"AboutBox",           // resource to use 
                              Window,                 // parent handle 
                              (DLGPROC) About); 
                }    break; 
                
                default: 
                    return DefWindowProc(Window, Message, WParam, LParam); 
            } 
        } break; 
        */
        case WM_SETFOCUS:
        {
            SetFocus(GameOutput); 
        } break; 
        
        case WM_SIZE:
        {
            // Make the edit control the size of the window's client 
            OutputDebugStringA("WM_SIZE\n\r");
            
            MoveWindow(GameOutput, 
                       0, 0,                  // starting x- and y-coordinates 
                       LOWORD(LParam),        // width of client area 
                       HIWORD(LParam),        // height of client area 
                       TRUE);                 // repaint window 
            
        } break;
        
        case WM_DESTROY:
        {
            // TODO(jon):  Handle this as an error, recreate window?
            running = false;
        } break;
        
        case WM_CLOSE:
        {
            // TODO(jon):  Handle this with a message to the user?
            running = false;
        } break;
        
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n\r");
            
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
            running = true;
            while (running)
            {
                MSG Message;
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
