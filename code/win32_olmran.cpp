/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jonathan Hawranko $
   $Notice: (C) Copyright 2020 by Jonathan Hawranko. All Rights Reserved. $
   ======================================================================== */

#include "olmran_platform.h"

#include <ws2tcpip.h>
#include <windows.h>
#include <richedit.h>

#include "olmran_telnet.h"
#include "win32_olmran.h"

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
                                  ES_MULTILINE | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | WS_VSCROLL | ES_AUTOVSCROLL, 
                                  x, y, width, height, 
                                  hwndOwner, controlId, hinst, 0);
    
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
GameOutputEditStreamCallback(DWORD_PTR dwCookie, 
                             LPBYTE lpBuff,
                             LONG cb, 
                             PLONG pcb)
{
    if (dwCookie) 
    {
        processTelnetCommand((const char *)dwCookie);
        //win32_AppendText(GameState.GameOutput, (char *)dwCookie);
        
        const char *psBuffer = (const char*)dwCookie;
        
        //String smaller than buffer
        if( strlen(psBuffer) < cb ) 
        {
            *pcb = (long) strlen(psBuffer);
            memcpy(lpBuff,psBuffer,*pcb);
            psBuffer = "";
            *pcb = (long) strlen(psBuffer);
        }
        //String larger than buffer
        else
        {
            *pcb = cb;
            memcpy(lpBuff,psBuffer,*pcb);
            memset(&psBuffer,0,*pcb-1);
            //text.erase(0,*pcb-1);
        }
        
        return 0;
        
        /*
        //if (cb < strlen(psBuffer)) cb = (long) strlen(psBuffer);
        
        for (int i=0;i<cb;i++)
        {
            *(lpBuff+i) = psBuffer[i];
        }
        
        //*pcb = cb;
        
        //*psBuffer = psBuffer->Mid(cb);
        
        return 0;
*/
    }
    
    return (DWORD)-1;
    
}

internal void
win32_StreamToGameOutput(const char *buf, int bufLength)
{
    EDITSTREAM es = { 0 };
    es.pfnCallback = GameOutputEditStreamCallback;
    // NOTE(jon):  The cookie should be the new data, while lpBuff
    // should be GameOutputBuffer (i believe?)
    //es.dwCookie    = (DWORD_PTR)GameState.GameOutputBuffer;
    es.dwCookie = (DWORD_PTR)buf;
    if (SendMessage(GameState.GameOutput, EM_STREAMIN, SF_TEXT, (LPARAM)&es) && es.dwError == 0) 
    {
        OutputDebugStringA("Stream success!\n");
    }
    else if (es.dwError != 0)
        OutputDebugStringA("ew.dwError != 0\n");
    
}

internal void
ProcessInputFromSocket(char *recvBuf)
{
    char tmpBuf[4096] = {0};
    uint16 recvIndex = 0;
    uint16 tmpIndex = 0;
    while (recvBuf[recvIndex] != 0)
    {
        // NOTE(jon):  Filtering telnet commands
        
        if(recvBuf[recvIndex] == TN_GA)
        { OutputDebugStringA("GA\n\r"); }
        else if(recvBuf[recvIndex] == TN_NOP)
        { OutputDebugStringA("NOP\n\r"); }
        else if(recvBuf[recvIndex] == TN_SE)
        { OutputDebugStringA("SE\n\r"); }
        else if(recvBuf[recvIndex] == TN_EOR)
        { OutputDebugStringA("EOR\n\r"); }
        else if (recvBuf[recvIndex] == TN_IAC)
        { OutputDebugStringA("IAC\n\r"); }
        else if(recvBuf[recvIndex] == TN_WILL)
        { OutputDebugStringA("WILL\n\r"); }
        else if(recvBuf[recvIndex] == TN_WONT)
        { OutputDebugStringA("WONT\n\r"); }
        else if(recvBuf[recvIndex] == TN_DO)
        { OutputDebugStringA("DO\n\r"); }
        else if(recvBuf[recvIndex] == TN_DONT)
        { OutputDebugStringA("DONT\n\r"); }
        else if(recvBuf[recvIndex] == OPT_ECHO)
        { OutputDebugStringA("ECHO\n\r"); }
        else if(recvBuf[recvIndex] == OPT_STATUS)
        { OutputDebugStringA("STATUS\n\r"); }
        else if(recvBuf[recvIndex] == OPT_TIMING_MARK)
        { OutputDebugStringA("TIMING_MARK\n\r"); }
        else if(recvBuf[recvIndex] == OPT_TERMINAL_TYPE)
        { OutputDebugStringA("TERMINAL_TYPE\n\r"); }
        else if(recvBuf[recvIndex] == OPT_EOR)
        { OutputDebugStringA("EOR\n\r"); }
        else if(recvBuf[recvIndex] == OPT_NAWS)
        { OutputDebugStringA("NAWS\n\r"); }
        else if(recvBuf[recvIndex] == OPT_COMPRESS)
        { OutputDebugStringA("COMPRESS\n\r"); }
        else if(recvBuf[recvIndex] == OPT_COMPRESS2)
        { OutputDebugStringA("COMPRESS2\n\r"); }
        else if(recvBuf[recvIndex] == OPT_MSP)
        { OutputDebugStringA("MSP\n\r"); }
        else if(recvBuf[recvIndex] == OPT_MXP)
        { OutputDebugStringA("MXP\n\r"); }
        else if(recvBuf[recvIndex] == OPT_102)
        { OutputDebugStringA("102\n\r"); }
        else if(recvBuf[recvIndex] == OPT_ATCP)
        { OutputDebugStringA("ATCP\n\r"); }
        else if(recvBuf[recvIndex] == OPT_GMCP)
        { OutputDebugStringA("GMCP\n\r"); }
        else
        {
            tmpBuf[tmpIndex] = recvBuf[recvIndex];
            tmpIndex++;
        }
        recvIndex++;
    }
    
    memcpy(GameState.GameOutputBuffer, tmpBuf, GameState.GameOutputBufferLength);
    
    // TODO(jon):  Remove this when streamin is fixed.
    if(strlen(tmpBuf)>0)
        win32_AppendText(GameState.GameOutput, GameState.GameOutputBuffer);
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
                ProcessInputFromSocket(GameState.GameOutputBuffer);
                win32_StreamToGameOutput(GameState.GameOutputBuffer, GameState.GameOutputBufferLength);
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
