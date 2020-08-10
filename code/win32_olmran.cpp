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

#include "win32_olmran.h"

#include "win32_audio.cpp"
#include "olmran_gmcp.cpp"
#include "olmran_telnet.cpp"
#include "ansi.cpp"
#include "olmran_state.cpp"
#include "win32_sockets.cpp"
#include "win32_controls.cpp"

LRESULT CALLBACK
win32_MainWindowCallback(HWND   Window,
                         UINT   Message,
                         WPARAM WParam,
                         LPARAM LParam)
{
    LRESULT Result = 0;
    HWND GameOutput;
    HWND GameInput;
    
    switch(Message)
    {
        case WM_CREATE:
        {
            // Create Edit Control
            GameOutput = CreateGameOutput(Window, 0, 0, 0, 0, (HMENU) ID_EDITCHILD,
                                          (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE));
            GameState.GameOutput = {};
            GameState.GameOutput.Window = GameOutput;
            
            // Create Input Control
            GameInput = CreateGameInput(Window, (HMENU) ID_INPUTCHILD, (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE));
            GameState.GameInput = {};
            GameState.GameInput.Window = GameInput;
            
            // Set starting Text Color
            GameState.CurrentColor = C_RESET;
            
            // Set autosneak
            GameState.AutoSneak = false;
            
        } break; 
        
        case WM_SETFOCUS:
        {
            SetFocus(GameState.GameInput.Window); 
        } break; 
        
        case WM_SIZE:
        {
            // Make the control the size of the window's client 
            MoveWindow(GameState.GameOutput.Window, 
                       5, 5,                  // starting x- and y-coordinates 
                       LOWORD(LParam)-10,     // width of client area 
                       HIWORD(LParam)-40,     // height of client area 
                       TRUE);                 // repaint window 
            
            MoveWindow(GameState.GameInput.Window, 
                       5, HIWORD(LParam)-30,  // starting x- and y-coordinates 
                       LOWORD(LParam)-10,     // width of client area 
                       20,                    // height of client area 
                       TRUE);                 // repaint window 
            
            SetFocus(GameState.GameInput.Window);
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
            SetFocus(GameState.GameInput.Window);
        } break;
        
        case MM_MCINOTIFY:
        {
            OutputDebugStringA("MM_MCINOTIFY:  MIDI Playback stopped.");
            GameState.MIDIDevice->IsPlaying = false;
        } break;
        default:
        {
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
    WNDCLASSA WindowClass = {};
    // TODO(jon):  Check if any of this is needed.
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = win32_MainWindowCallback;
    WindowClass.hInstance = Instance;
    //    WindowClass.hIcon = ;
    //    WindowClass.hCursor = ;
    WindowClass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
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
            
            // TODO(jon):  Pool these VirtualAllocs together.
            GameState.GameInput.BufferLength = 512;
            GameState.GameInput.Buffer = (char *)VirtualAlloc(0, GameState.GameInput.BufferLength, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            GameState.CommandHistory.BufferSize = 512;
            GameState.CommandHistory.NumberOfCommands = 10;
            GameState.CommandHistory.CurrentPosition = -1;
            GameState.CommandHistory.Commands = (char *)VirtualAlloc(0, GameState.CommandHistory.BufferSize * GameState.CommandHistory.NumberOfCommands, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            GameState.CommandHistory.CurrentSize = 512;
            GameState.CommandHistory.CurrentCommand = (char *)VirtualAlloc(0, GameState.CommandHistory.CurrentSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            GameState.GMCP.BufferSize = 1024;
            GameState.GMCP.BufferOut = (char *)VirtualAlloc(0, GameState.GMCP.BufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            GameState.GMCP.BufferIn = (char *)VirtualAlloc(0, GameState.GMCP.BufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            GameState.User.Account = {};
            GameState.User.Player = {};
            
            GameState.MIDIDevice = (midi_device *)VirtualAlloc(0, sizeof(midi_device), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            DWORD ThreadID;
            HANDLE SocketListenThreadHandle;
            if (win32_InitAndConnectSocket()==0)
            {
                OutputDebugStringA("Socket Connected\r\n");
                
                TelnetInit(Telnet);
                
                char *Param = "Socket listening.\r\n";
                
                SocketListenThreadHandle = CreateThread(0, 0, SocketListenThreadProc, Param, 0, &ThreadID);
            }
            else
            {
                win32_AppendText(GameState.GameOutput.Window, TEXT("Could not connect to server.\r\n"));
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
