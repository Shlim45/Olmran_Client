/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jonathan Hawranko $
   $Notice: (C) Copyright 2020 by Jonathan Hawranko. All Rights Reserved. $
   ======================================================================== */
#include "win32_olmran.h"

/* https://stackoverflow.com/questions/16329007/win32-appending-text-to-an-edit-control */
void win32_AppendText(const HWND GameOutput, TCHAR *newText)
{
    // get the current selection
    DWORD StartPos, EndPos;
    SendMessage( GameOutput, EM_GETSEL, reinterpret_cast<WPARAM>(&StartPos), reinterpret_cast<WPARAM>(&EndPos) );
    
    // move the caret to the end of the text
    int outLength = GetWindowTextLength( GameOutput );
    SendMessage( GameOutput, EM_SETSEL, outLength, outLength );
    
    // insert the text at the new caret position
    SendMessage( GameOutput, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(newText) );
    
    // restore the previous selection
    SendMessage( GameOutput, EM_SETSEL, StartPos, EndPos );
}

LRESULT CALLBACK
MainWindowCallback(HWND   Window,
                   UINT   Message,
                   WPARAM WParam,
                   LPARAM LParam)
{
    LRESULT Result = 0;
    local_persist HWND GameOutput;
    
    TCHAR OutputBytes[] = TEXT("This is the MUD Client for Olmran\r\n")
        TEXT("There are many like it, but this one is special!\r\n")
        TEXT("I leared C++ while creating it... maybe?");
    
    switch(Message)
    {
        case WM_CREATE:
        {
            // Create Edit Control
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
            
            if (win32_InitAndConnectSocket(GameOutput))
            {
                win32_AppendText(GameOutput, OutputBytes);
            }
            
            win32_AppendText(GameOutput, TEXT("\r\n\r\nAppending Test TeXt\r\n"));
        } break; 
        
        case WM_SETFOCUS:
        {
            OutputDebugStringA("WM_SETFOCUS\n\r");
            SetFocus(GameOutput); 
        } break; 
        
        case WM_SIZE:
        {
            // Make the static control the size of the window's client 
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

bool win32_InitAndConnectSocket(HWND GameOutput)
{
    // Initialize WinSock
    WSAData data;
    WORD ver = MAKEWORD(2,2);
    int WSResult = WSAStartup(ver, &data);
    
    if (WSResult != 0)
    {
        win32_AppendText(GameOutput, TEXT("Can't start WinSock, Err #\r\n"));
        return false;
    }
    
    // Create Socket
    Socket.sock = socket(AF_INET, SOCK_STREAM, 0);
    if (Socket.sock == INVALID_SOCKET)
    {
        win32_AppendText(GameOutput, TEXT("Can't create socket, Err #\r\n"));
        return false;
    }
    
    // Fill in hint structure
    Socket.addr.sin_family = AF_INET;
    Socket.addr.sin_port = htons(HOST_PORT);
    inet_pton(AF_INET, HOST_ADDRESS, &Socket.addr.sin_addr);
    
    // Connect to server
    if (connect(Socket.sock, (sockaddr*) &Socket.addr, sizeof(Socket.addr)) == SOCKET_ERROR)
    {
        win32_AppendText(GameOutput, TEXT("Can't connect to server, Err #\r\n"));
        
        win32_CloseSocket();
        return false;
    }
    
    Socket.status = 1;
    return true;
}

void win32_CloseSocket()
{
    closesocket(Socket.sock);
    WSACleanup();
    Socket.status = 0;
    //Socket = {};
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
                           WS_OVERLAPPEDWINDOW|WS_SIZEBOX|WS_VISIBLE,
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
            win32_CloseSocket();
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
