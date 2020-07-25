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
    /*
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
*/
#if 0
    // Append new text at end of log
	int iOriginalLength = GetWindowTextLength(GameOutput); 
	SendMessage(GameOutput, EM_SETSEL, WPARAM(iOriginalLength), LPARAM(iOriginalLength));
	SendMessage(GameOutput, EM_REPLACESEL, WPARAM(FALSE), LPARAM(newText));
    
	// Get new length and select the text between the original end and the new end
	int iNumLinesMinusOne = int(SendMessage(GameOutput, EM_GETLINECOUNT, 0, 0)) - 1;
	iOriginalLength -= iNumLinesMinusOne;
	int iNewLength = GetWindowTextLength(GameOutput) - iNumLinesMinusOne; 
	SendMessage(GameOutput, EM_SETSEL, WPARAM(iOriginalLength), LPARAM(iNewLength));
    
	// Set colour of selected text 
    CHARFORMAT cf;
	cf.crTextColor = RGB(200,50,50);
	SendMessage(GameOutput, EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&cf));
#endif
    CHARFORMAT cf;
    memset( &cf, 0, sizeof cf );
    cf.cbSize = sizeof cf;
    cf.dwMask = CFM_COLOR;
    cf.crTextColor = RGB(55,200,100);// <----- the color of the text
    SendMessage( GameOutput, EM_SETCHARFORMAT, (LPARAM)SCF_SELECTION, (LPARAM) &cf);
    /*SendMessage(hwnd, EM_SETSEL, 0, -1);
    SendMessage(hwnd, EM_SETSEL, -1, -1);
    SendMessage(hwnd, EM_REPLACESEL, 0, (LPARAM)msg.c_str());*/
    CHARRANGE cr;
    cr.cpMin = -1;
    cr.cpMax = -1;
    SendMessage(GameOutput, EM_EXSETSEL, 0, (LPARAM)&cr);
    SendMessage(GameOutput, EM_REPLACESEL, 0, (LPARAM)newText);
}
#endif

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
        if (SendMessage(GameState.GameOutput, EM_STREAMIN, SF_TEXT|SFF_SELECTION, (LPARAM)&es) && es.dwError == 0) 
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
#if 0
    local_persist TCHAR tmpBuf[4096];
    memset(tmpBuf,0,4096);
    uint16 recvIndex = 0;
    uint16 tmpIndex = 0;
    if (strlen(recvBuf) == 0)
        return;
    while (recvBuf[recvIndex] != 0)
    {
        // NOTE(jon):  Filtering telnet commands
#if 0
        if (recvBuf[recvIndex] >= TN_EOR && recvBuf[recvIndex] <= TN_IAC)
        {
            char tCommand[4] = { 0 };
            for (int tIndex = 0; tIndex < 4; tIndex++)
            {
                if (recvBuf[recvIndex+tIndex] == '\0')
                    break;
                tCommand[tIndex] = recvBuf[recvIndex+tIndex];
            }
            processTelnetCommand(tCommand);
        }
#endif
        
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
            case OPT_COMPRESS:
            { OutputDebugStringA("COMPRESS\n\r"); } break;
            case OPT_COMPRESS2:
            { OutputDebugStringA("COMPRESS2\n\r"); } break;
            case OPT_MSP:
            { OutputDebugStringA("MSP\n\r"); } break;
            case OPT_MXP:
            { OutputDebugStringA("MXP\n\r"); } break;
            case OPT_102:
            { OutputDebugStringA("102\n\r"); } break;
            case OPT_ATCP:
            { OutputDebugStringA("ATCP\n\r"); } break;
            case OPT_GMCP:
            { OutputDebugStringA("GMCP\n\r"); } break;
            default:
            {
                tmpBuf[tmpIndex] = recvBuf[recvIndex];
                tmpIndex++;
            }
        }
        recvIndex++;
    }
    memcpy(recvBuf, tmpBuf, sizeof(tmpBuf));
#endif
    
    // TODO(jon):  Remove this when streamin is fixed.
#if 1
    if( strlen(recvBuf)>0 )
        win32_AppendText( GameState.GameOutput, recvBuf );
#endif
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
#if 0
                win32_AppendText(GameState.GameOutput, OutputBytes);
#endif
                OutputDebugStringA("Socket Connected\r\n");
                
                char *Param = "Socket listening.\r\n";
                
                SocketListenThreadHandle = CreateThread(0, 0, SocketListenThreadProc, Param, 0, &ThreadID);
            }
            else
            {
#if 0
                win32_AppendText(GameState.GameOutput, TEXT("Could not connect to server.\r\n"));
#endif
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
