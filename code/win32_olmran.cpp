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

#if 1
/* https://stackoverflow.com/questions/16329007/win32-appending-text-to-an-edit-control */
internal void 
win32_AppendText(const HWND GameOutput, const char *newText)
{
    CHARFORMAT cf;
    memset( &cf, 0, sizeof cf );
    cf.cbSize = sizeof cf;
    cf.dwMask = CFM_COLOR;
    cf.crTextColor = RGB(186,218,85);// <----- the color of the text
    SendMessageA( GameOutput, EM_SETCHARFORMAT, (LPARAM)SCF_SELECTION, (LPARAM) &cf);
    
    CHARRANGE cr;
    cr.cpMin = -1;
    cr.cpMax = -1;
    SendMessageA(GameOutput, EM_EXSETSEL, 0, (LPARAM)&cr);
    SendMessageA(GameOutput, EM_REPLACESEL, 0, (LPARAM)newText);
}
#endif

internal uint32
win32_SendInputThroughSocket(SOCKET s, const char *buf, int len, int flags)
{
    uint32 iResult;
    iResult = send( s, buf, (int)strlen(buf), 0 );
    if (iResult == SOCKET_ERROR) {
        OutputDebugStringA("send failed with error\n");
        closesocket(s);
        WSACleanup();
        return 1;
    }
    return iResult;
}

internal HWND
CreateGameOutput(HWND hwndOwner,        // Dialog box handle.
                 int x, int y,          // Location.
                 int width, int height, // Dimensions.
                 HMENU controlId,       // Control ID.
                 HINSTANCE hinst)       // Application or DLL instance.
{
    LoadLibrary(TEXT("Riched32.dll"));
    
    HWND hwndEdit= CreateWindowExA(0, RICHEDIT_CLASS, TEXT("Game Output"),
                                   ES_MULTILINE | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL, 
                                   x, y, width, height, 
                                   hwndOwner, controlId, hinst, 0);
    
    // Set Background Color
    SendMessageA( hwndEdit, EM_SETBKGNDCOLOR, 0, RGB(50,50,50) );
    
    return hwndEdit;
}

global_variable WNDPROC oldEditProc;

LRESULT CALLBACK InputEditProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_RETURN:
                {
                    // TODO(jon):  Send input
                    char inputBuf[512] = {0};
                    int inputLength = GetWindowTextA(               // calls GetWindowTextA and returns length (not including \0)
                                                     GameState.GameInput,       // A handle to the edit control.
                                                     (LPSTR) inputBuf,          // A pointer to the buffer that will receive the text.
                                                     GameState.GameInputBufferLength// The maximum number of characters to copy to the buffer, including the NULL terminator.
                                                     );
                    win32_SendInputThroughSocket(Socket.sock, inputBuf, 512, 0);
                    SetWindowTextA(GameState.GameInput, "");
                    return 0;
                } break;
                case VK_ESCAPE:
                {
                    SetWindowTextA(GameState.GameInput, "");
                    return 0;
                } break;
                default:
                {
                    return CallWindowProc(oldEditProc, wnd, msg, wParam, lParam);
                }
            }
        } break;
        default:
        return CallWindowProc(oldEditProc, wnd, msg, wParam, lParam);
    }
}

internal HWND
CreateGameInput(HWND hwndOwner, HMENU controlId, HINSTANCE hinst)
{
    HWND hwndInput = CreateWindowExA(
                                     0, TEXT("EDIT"),   // predefined class 
                                     NULL,         // no window title 
                                     WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_LEFT, 
                                     0, 0, 0, 0,   // set size in WM_SIZE message 
                                     hwndOwner,         // parent window 
                                     controlId,   // edit control ID 
                                     hinst, 
                                     NULL);        // pointer not needed 
    oldEditProc = (WNDPROC)SetWindowLongPtr(hwndInput, GWLP_WNDPROC, (LONG_PTR)InputEditProc);
    
    return hwndInput;
}

internal void 
win32_CloseSocket()
{
    closesocket(Socket.sock);
    WSACleanup();
    Socket.status = 0;
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
        OutputDebugStringA("Can't start WinSock, Err #");
        return WSResult;
    }
    
    // Create Socket
    Socket.sock = socket(AF_INET, SOCK_STREAM, 0);
    if (Socket.sock == INVALID_SOCKET)
    {
        OutputDebugStringA("Can't create socket, Err #");
        return INVALID_SOCKET;
    }
    
    // Fill in hint structure
    Socket.addr.sin_family = AF_INET;
    Socket.addr.sin_port = htons(HOST_PORT);
    inet_pton(AF_INET, HOST_ADDRESS, &Socket.addr.sin_addr);
    
    // Connect to server
    if (connect(Socket.sock, (sockaddr*) &Socket.addr, sizeof(Socket.addr)) == SOCKET_ERROR)
    {
        OutputDebugStringA("Can't connect to server, Err #");
        
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
    local_persist HWND GameInput;
    
    switch(Message)
    {
        case WM_CREATE:
        {
            // Create Edit Control
            GameOutput = CreateGameOutput(Window, 0, 0, 0, 0, (HMENU) ID_EDITCHILD,
                                          (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE));
            GameState.GameOutput = GameOutput;
            
            // Create Input Control
            GameInput = CreateGameInput(Window, (HMENU) ID_INPUTCHILD, (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE));
            GameState.GameInput = GameInput;
        } break; 
        
        case WM_SETFOCUS:
        {
            // OutputDebugStringA("WM_SETFOCUS\n\r");
            SetFocus(GameOutput); 
        } break; 
        
        case WM_SIZE:
        {
            // Make the static control the size of the window's client 
            OutputDebugStringA("WM_SIZE\n\r");
            
            MoveWindow(GameState.GameOutput, 
                       5, 5,                  // starting x- and y-coordinates 
                       LOWORD(LParam)-10,     // width of client area 
                       HIWORD(LParam)-40,     // height of client area 
                       TRUE);                 // repaint window 
            
            MoveWindow(GameState.GameInput, 
                       5, HIWORD(LParam)-30,  // starting x- and y-coordinates 
                       LOWORD(LParam)-10,     // width of client area 
                       20,                    // height of client area 
                       TRUE);                 // repaint window 
            
            SetFocus(GameState.GameInput);
            
#if 0
            // TODO(jon):  Handle input
            void Edit_GetText(               // calls GetWindowTextA and returns length (not including \0)
                              hwndCtl,       // A handle to the edit control.
                              lpch,          // A pointer to the buffer that will receive the text.
                              cchMax         // The maximum number of characters to copy to the buffer, including the NULL terminator.
                              );
            
            // TODO(jon): input history
            void Edit_SetText(
                              hwndCtl,
                              lpsz           // A pointer to a null-terminated string to be set as the text in the control.
                              );
#endif
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
            // OutputDebugStringA("WM_ACTIVATEAPP\n\r");
            
        } break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            uint32 KeyCode = (uint32) WParam;
            switch (KeyCode)
            {
                case VK_MBUTTON:
                case VK_END:
                {
                    // TODO(jon):  middle mouse button / end key, scroll game output to EOF
                } break;
                case VK_ESCAPE:
                {
                    // TODO(jon):  Clear input bar
                } break;
                case VK_PRIOR:
                {
                    // TODO(jon):  page up
                } break;
                case VK_NEXT:
                {
                    // TODO(jon):  page down
                } break;
                case VK_UP:
                {
                    // TODO(jon):  up arrow, cycle through input history
                } break;
                case VK_DOWN:
                {
                    // TODO(jon):  down arrow, cycle through input history
                } break;
                case VK_NUMPAD0:
                {
                    // TODO(jon):  NUMPAD 0 - 9
                } break;
            }
            
        } break;
        
        default:
        {
            //            OutputDebugStringA("default case\n\r");
            Result = DefWindowProc(Window, Message, WParam, LParam);
        }
    }
    
    return Result;
}

#if 0
internal DWORD CALLBACK
DebugEditStreamCallback(DWORD_PTR dwCookie, // data to stream in
                        LPBYTE lpBuff,      // fills this buffer with data to transfer into the rich edit control
                        LONG cb,            // Number of bytes to read or write.
                        PLONG pcb)
{
    /*
    The control calls the callback function repeatedly, transferring a portion of the data with each call. 
The control continues to call the callback function until one of the following conditions occurs:
    
    - The callback function returns a nonzero value.
    - The callback function returns zero in the * pcb parameter.
    - An error occurs that prevents the rich edit control from transferring data into or out of itself. Examples are out-of-memory situations, failure of a system function, or an invalid character in the read buffer.
    - For a stream-in operation, the RTF code contains data specifying the end of an RTF block.
    - For a stream-in operation on a single-line edit control, the callback reads in an end-of-paragraph character (CR, LF, VT, LS, or PS).
    */
    
    if (dwCookie)
    {
        // TODO(jon):  Copy contents of dwCookie into lpBuff, if it will fit
        const char *inBuf = (char *) dwCookie;
        long bufLen = (long) strlen(inBuf);
        
        if ( bufLen > 0 )
        {
            //String smaller than buffer
            if ( bufLen < cb ) 
            {
                *pcb = bufLen;
                memcpy(lpBuff,inBuf,*pcb);
                inBuf = "";
                *pcb = 0;
            }
            //String larger than buffer
            else
            {
                *pcb = cb;
                memcpy(lpBuff,inBuf,*pcb);
                memset(&inBuf,0,*pcb-1);
            }
        }
        return 0;                           // Returns zero to indicate success.
    }
    return (DWORD) -1;                      // Returns a nonzero value to indicate an error.
}
#endif

#if 0
internal DWORD CALLBACK 
GameOutputEditStreamCallback(DWORD_PTR dwCookie, 
                             LPBYTE lpBuff,
                             LONG cb, 
                             PLONG pcb)
{
    if (dwCookie) 
    {
        processTelnetCommand((const char *)dwCookie);
#if 1
        win32_AppendText(GameState.GameOutput, (char *)dwCookie);
#endif
        
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
#endif

#if 0
internal void
win32_StreamToGameOutput(const char *buf, int bufLength)
{
    EDITSTREAM es = { 0 };
#if 0
    es.pfnCallback = GameOutputEditStreamCallback;
#else
    es.pfnCallback = DebugEditStreamCallback;
#endif
    // NOTE(jon):  The cookie should be the new data, while lpBuff
    // should be GameOutputBuffer (i believe?)
    //es.dwCookie    = (DWORD_PTR)GameState.GameOutputBuffer;
    es.dwCookie = (DWORD_PTR)buf;
    if (strlen(buf) > 0)
    {
        if (SendMessageA(GameState.GameOutput, EM_STREAMIN, SF_TEXT|SFF_SELECTION, (LPARAM)&es) && es.dwError == 0) 
        {
            OutputDebugStringA("Stream success!\n");
        }
        else if (es.dwError != 0)
            OutputDebugStringA("ew.dwError != 0\n");
    }
    
}
#endif

internal void
ProcessInputFromSocket(char *recvBuf)
{
#if 1
    local_persist TCHAR tmpBuf[4096];
    memset(tmpBuf,0,4096);
    uint16 recvIndex = 0;
    uint16 tmpIndex = 0;
    if (strlen(recvBuf) == 0)
        return;
    while (recvBuf[recvIndex] != 0)
    {
        // NOTE(jon):  Filtering telnet commands
        
        if (recvBuf[recvIndex] >= ' ' && recvBuf[recvIndex] <= '~')
        {
            // valid char
            tmpBuf[tmpIndex] = recvBuf[recvIndex];
            tmpIndex++;
        }
        else if (recvBuf[recvIndex] >= 9 && recvBuf[recvIndex] <= 13)
        {
            // valid formatting
            if (recvBuf[recvIndex] != 13) // prevent double spacing (convert from CRLF to LF)
            {
                tmpBuf[tmpIndex] = recvBuf[recvIndex];
                tmpIndex++;
            }
        }
        else
        {
            switch (recvBuf[recvIndex])
            {
                case TN_GA:
                { OutputDebugStringA("GA\n\r"); } break;
                case TN_NOP:
                { OutputDebugStringA("NOP\n\r"); } break;
                case TN_SE:
                { OutputDebugStringA("SE\n\r"); } break;
                case TN_EOR:
                { OutputDebugStringA("EOR\n\r"); } break;
                case TN_IAC:
                { OutputDebugStringA("IAC "); } break;
                case TN_WILL:
                { OutputDebugStringA("WILL "); } break;
                case TN_WONT:
                { OutputDebugStringA("WONT "); } break;
                case TN_DO:
                { OutputDebugStringA("DO "); } break;
                case TN_DONT:
                { OutputDebugStringA("DONT "); } break;
                case OPT_ECHO:
                { OutputDebugStringA("ECHO\n\r"); } break;
                case OPT_STATUS:
                { OutputDebugStringA("STATUS\n\r"); } break;
                case OPT_TIMING_MARK:
                { OutputDebugStringA("TIMING_MARK\n\r"); } break;
                case OPT_TERMINAL_TYPE:
                { OutputDebugStringA("TERMINAL_TYPE\n\r"); } break;
                case OPT_EOR:
                { OutputDebugStringA("EOR\n\r"); } break;
                case OPT_NAWS:
                { OutputDebugStringA("NAWS\n\r"); } break;
                case OPT_ATCP:
                { OutputDebugStringA("ATCP\n\r"); } break;
                case OPT_GMCP:
                { OutputDebugStringA("GMCP\n\r"); } break;
                default:
                OutputDebugStringA("\nDEFAULT: " + (char)recvBuf[recvIndex]);
            }
        }
        recvIndex++;
    }
    memset(recvBuf, 0, 4096);
    memcpy(recvBuf, tmpBuf, sizeof(tmpBuf));
#endif
    
    // TODO(jon):  Remove this when streamin is fixed.
    if( strlen(recvBuf)>0 )
        win32_AppendText( GameState.GameOutput, recvBuf );
}

DWORD WINAPI 
SocketListenThreadProc(LPVOID lpParameter)
{
    if (GameState.isInitialized)
    {
        int iResult;
        local_persist char recvbuf[4096];
        memset(recvbuf, 0, 4096);
        
        // Receive until the peer closes the connection
        do {
            iResult = recv(Socket.sock, recvbuf, 4096, 0);
            if ( iResult > 0 )
            {
                OutputDebugStringA("Bytes received: >0\n");//, iResult);
                if (strlen(recvbuf) > 0)
                {
                    ProcessInputFromSocket(recvbuf);
#if 0
                    win32_StreamToGameOutput(recvbuf, 4096);
#endif
                }
            }
            else if ( iResult == 0 )
                OutputDebugStringA("Connection closed\n");
            else
            {
                int error = WSAGetLastError();
                OutputDebugStringA("recv failed: ?\n");//, WSAGetLastError());
            }
            
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
    WNDCLASSA WindowClass = {};
    // TODO(jon):  Check if any of this is needed.
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = win32_MainWindowCallback;
    WindowClass.hInstance = Instance;
    //    WindowClass.hIcon = ;
    //    WindowClass.hCursor = ;
    //    WindowClass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    //    WindowClass.lpszMenuName = ;
    WindowClass.lpszClassName = "OlmranWindowClass";
    
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
            
            local_persist char sendbuf[512];
            GameState.GameInputBuffer = sendbuf;
            GameState.GameInputBufferLength = 512;
            
            DWORD ThreadID;
            HANDLE SocketListenThreadHandle;
            if (win32_InitAndConnectSocket()==0)
            {
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
                BOOL MessageResult = GetMessageA(&Message, 0, 0, 0);
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
