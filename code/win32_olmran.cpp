/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jonathan Hawranko $
   $Notice: (C) Copyright 2020 by Jonathan Hawranko. All Rights Reserved. $
   ======================================================================== */

#include <iostream>
#include <string.h>
#include "olmran_platform.h"

#include <ws2tcpip.h>
#include <windows.h>
#include <richedit.h>

//#include "olmran_telnet.h"
#include "win32_olmran.h"

// Telnet

const char TN_BELL = static_cast<char>(7);

const char TN_EOR = static_cast<char>(239);
const char TN_SE = static_cast<char>(240);
const char TN_NOP = static_cast<char>(241);
const char TN_DM = static_cast<char>(242);
const char TN_B = static_cast<char>(243);
const char TN_IP = static_cast<char>(244);
const char TN_AO = static_cast<char>(245);
const char TN_AYT = static_cast<char>(246);
const char TN_EC = static_cast<char>(247);
const char TN_EL = static_cast<char>(248);
const char TN_GA = static_cast<char>(249);
const char TN_SB = static_cast<char>(250);
const char TN_WILL = static_cast<char>(251);
const char TN_WONT = static_cast<char>(252);
const char TN_DO = static_cast<char>(253);
const char TN_DONT = static_cast<char>(254);
const char TN_IAC = static_cast<char>(255);

const char TNSB_IS = 0;
const char TNSB_SEND = 1;


const char OPT_ECHO = 1;
const char OPT_STATUS = 5;
const char OPT_TIMING_MARK = 6;
const char OPT_TERMINAL_TYPE = 24;
const char OPT_EOR = 25;
const char OPT_NAWS = 31;
const char OPT_MSDP = 69; // http://tintin.sourceforge.net/msdp/
const char OPT_MSSP = static_cast<char>(70); // https://tintin.sourceforge.io/protocols/mssp/
const char OPT_COMPRESS = 85;
const char OPT_COMPRESS2 = 86;
const char OPT_MSP = 90;
const char OPT_MXP = 91;
const char OPT_102 = 102;
const char OPT_ATCP = static_cast<char>(200);
const char OPT_GMCP = static_cast<char>(201);

const char MSSP_VAR = 1;
const char MSSP_VAL = 2;

const char MSDP_VAR = 1;
const char MSDP_VAL = 2;
const char MSDP_TABLE_OPEN = 3;
const char MSDP_TABLE_CLOSE = 4;
const char MSDP_ARRAY_OPEN = 5;
const char MSDP_ARRAY_CLOSE = 6;


void processTelnetCommand(const std::string &command)
{
    char ch = command[1];
    
    char *_type;
    switch ((uint8)ch) {
        case 239:
        _type = "TN_EOR";
        break;
        case 249:
        _type = "TN_GA";
        break;
        case 250:
        _type = "SB";
        break;
        case 251:
        _type = "WILL";
        break;
        case 252:
        _type = "WONT";
        break;
        case 253:
        _type = "DO";
        break;
        case 254:
        _type = "DONT";
        break;
        case 255:
        _type = "IAC";
        break;
        default:
        _type="";
        //_type = QString::number((quint8)ch);
        break;
    }
    if (command.size() > 2) {
        std::cout << "SERVER sent telnet (" << command.size() << " bytes):" << _type << " + " << command[2] << std::endl;
    } else {
        std::cout << "SERVER sent telnet (" << command.size() << " bytes):" << _type << std::endl;
    }
}
/* https://stackoverflow.com/questions/16329007/win32-appending-text-to-an-edit-control */
internal void 
win32_AppendText(const HWND GameOutput, TCHAR *newText)
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

internal HWND
CreateRichEdit(HWND hwndOwner,        // Dialog box handle.
               int x, int y,          // Location.
               int width, int height, // Dimensions.
               HMENU controlId,       // Control ID.
               HINSTANCE hinst)       // Application or DLL instance.
{
    LoadLibrary(TEXT("Riched32.dll"));
    
    HWND hwndEdit= CreateWindowEx(0, RICHEDIT_CLASS, TEXT("Game Output"),
                                  ES_MULTILINE | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP, 
                                  x, y, width, height, 
                                  hwndOwner, controlId, hinst, NULL);
    
    return hwndEdit;
}



internal void 
win32_CloseSocket()
{
    closesocket(Socket.sock);
    WSACleanup();
    Socket.status = 0;
    //Socket = {};
}

internal int64 
win32_InitAndConnectSocket()
{
    // Initialize WinSock
    WSAData data;
    WORD ver = MAKEWORD(2,2);
    int WSResult = WSAStartup(ver, &data);
    
    if (WSResult != 0)
    {
        // "Can't start WinSock, Err #"
        return WSResult;
    }
    
    // Create Socket
    Socket.sock = socket(AF_INET, SOCK_STREAM, 0);
    if (Socket.sock == INVALID_SOCKET)
    {
        // "Can't create socket, Err #"
        return INVALID_SOCKET;
    }
    
    // Fill in hint structure
    Socket.addr.sin_family = AF_INET;
    Socket.addr.sin_port = htons(HOST_PORT);
    inet_pton(AF_INET, HOST_ADDRESS, &Socket.addr.sin_addr);
    
    // Connect to server
    if (connect(Socket.sock, (sockaddr*) &Socket.addr, sizeof(Socket.addr)) == SOCKET_ERROR)
    {
        // "Can't connect to server, Err #"
        
        win32_CloseSocket();
        return SOCKET_ERROR;
    }
    
    Socket.status = 1;
    return 0;
}

LRESULT CALLBACK
win32_MainWindowCallback(HWND   Window,
                         UINT   Message,
                         WPARAM WParam,
                         LPARAM LParam)
{
    LRESULT Result = 0;
    local_persist HWND GameOutput;
    
    switch(Message)
    {
        case WM_CREATE:
        {
            // Create Edit Control
            GameOutput = CreateRichEdit(Window, 0, 0, 0, 0, (HMENU) ID_EDITCHILD,
                                        (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE));
            GameState.GameOutput = GameOutput;
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
            GlobalRunning = false;
        } break;
        
        case WM_CLOSE:
        {
            // TODO(jon):  Handle this with a message to the user?
            GlobalRunning = false;
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

internal DWORD CALLBACK 
GameOutputEditStreamCallback(DWORD_PTR bytesReceived, 
                             LPBYTE lpBuff,
                             LONG cb, 
                             PLONG pcb)
{
    if (bytesReceived) 
    {
        processTelnetCommand((char *)bytesReceived);
        win32_AppendText(GameState.GameOutput, (char *)bytesReceived);
        return 0;
    }
    
    return (DWORD)-1;
}

DWORD WINAPI 
SocketListenThreadProc(LPVOID lpParameter)
{
    if (GameState.isInitialized)
    {
        int iResult;
        
        // Receive until the peer closes the connection
        do {
            iResult = recv(Socket.sock, GameState.GameOutputBuffer, GameState.GameOutputBufferLength, 0);
            if ( iResult > 0 )
            {
                OutputDebugStringA("Bytes received: >0\n");//, iResult);
                EDITSTREAM es = { 0 };
                es.pfnCallback = GameOutputEditStreamCallback;
                es.dwCookie    = (DWORD_PTR)GameState.GameOutputBuffer;
                if (SendMessage(GameState.GameOutput, EM_STREAMIN, SF_TEXT, (LPARAM)&es) && es.dwError == 0) 
                {
                    OutputDebugStringA("Stream success!\n");
                }
                else if (es.dwError != 0)
                    OutputDebugStringA("ew.dwError != 0\n");
            }
            else if ( iResult == 0 )
                OutputDebugStringA("Connection closed\n");
            else
                OutputDebugStringA("recv failed: ?\n");//, WSAGetLastError());
            
        } while( iResult > 0 );
        
        // cleanup
        closesocket(Socket.sock);
        WSACleanup();
    }
    return 0;
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
    WindowClass.lpfnWndProc = win32_MainWindowCallback;
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
            GameState.Window = WindowHandle;
            GameState.isInitialized = true;
            
            local_persist char recvbuf[4096];
            GameState.GameOutputBuffer = recvbuf;
            GameState.GameOutputBufferLength = 4096;
            
            TCHAR OutputBytes[] = TEXT("This is the MUD Client for Olmran\r\n")
                TEXT("There are many like it, but this one is special!\r\n")
                TEXT("I leared C++ while creating it... maybe?");
            
            DWORD ThreadID;
            HANDLE SocketListenThreadHandle;
            if (win32_InitAndConnectSocket()==0)
            {
                win32_AppendText(GameState.GameOutput, OutputBytes);
                OutputDebugStringA("Socket Connected\r\n");
                
                char *Param = "Socket listening.\r\n";
                
                SocketListenThreadHandle = CreateThread(0, 0, SocketListenThreadProc, Param, 0, &ThreadID);
            }
            else
            {
                win32_AppendText(GameState.GameOutput, TEXT("Could not connect to server.\r\n"));
                OutputDebugStringA("Error in win32_InitAndConnectSocket()");
                SocketListenThreadHandle = 0;
            }
            
            GlobalRunning = true;
            while (GlobalRunning)
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
            // NOTE(jon):  Is this necessary?  Windows might clean it up itself.
            win32_CloseSocket();
            if (SocketListenThreadHandle) { CloseHandle(SocketListenThreadHandle); }
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
