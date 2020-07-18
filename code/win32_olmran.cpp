/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jonathan Hawranko $
   $Notice: (C) Copyright 2020 by Jonathan Hawranko. All Rights Reserved. $
   ======================================================================== */

#include <ws2tcpip.h>
#include <windows.h>

#define ID_EDITCHILD 100
#define HOST_ADDRESS "192.168.1.208"
#define HOST_PORT 4000

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
    local_persist SOCKET sock;
    
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
            
            // Initialize WinSock
            WSAData data;
            WORD ver = MAKEWORD(2,2);
            int WSResult = WSAStartup(ver, &data);
            
            if (WSResult != 0)
            {
                TCHAR ErrorOutput[] = TEXT("Can't start WinSock, Err #\r\n");
                SendMessage(GameOutput, WM_SETTEXT, 0, (LPARAM) ErrorOutput);
                break;
            }
            
            // Create socket
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET)
            {
                TCHAR ErrorOutput[] = TEXT("Can't create socket, Err #\r\n");
                SendMessage(GameOutput, WM_SETTEXT, 0, (LPARAM) ErrorOutput);
                break;
            }
            
            // Fill in hint structure
            sockaddr_in hint;
            hint.sin_family = AF_INET;
            hint.sin_port = htons(HOST_PORT);
            inet_pton(AF_INET, HOST_ADDRESS, &hint.sin_addr);
            
            // Connect to server
            int ConnResult = connect(sock, (sockaddr*) &hint, sizeof(hint), 0);
            if (ConnResult == SOCKET_ERROR)
            {
                TCHAR ErrorOutput[] = TEXT("Can't connect to server, Err #\r\n");
                SendMessage(GameOutput, WM_SETTEXT, 0, (LPARAM) ErrorOutput);
                
                closesocket(sock);
                WSACleanup();
                break;
            }
            
            // Add text to the window. 
            SendMessage(GameOutput, WM_SETTEXT, 0, (LPARAM) OutputBytes); 
        } break; 
        
        case WM_SETFOCUS:
        {
            OutputDebugStringA("WM_SETFOCUS\n\r");
            SetFocus(GameOutput); 
            
            
            // Listen loop
            char buf[4096];
            while (true)
            {
                ZeroMemory(buf, sizeof(buf));
                int bytesReceived = recv(sock, buf, sizeof(buf), 0);
                if (bytesReceived > 0)
                {
                    SendMessage(GameOutput, WM_SETTEXT, 0, (LPARAM) buf);
                }
            }
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
