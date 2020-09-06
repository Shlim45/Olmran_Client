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
    HWND GameOutput = {};
    HWND GameInput = {};
    HWND GameControl = {};
    HWND PlayerInfo = {};
    
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
            
            // Create Static Control for Health Bars / Portrait / Player Info Background Image
            GameControl = CreateWindowExA(0, TEXT("STATIC"), NULL,
                                          WS_VISIBLE | WS_CHILD | SS_BITMAP,
                                          0, 0, 0, 0, Window, (HMENU)ID_CONTROLBACKGROUND, (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE), NULL);
            
            if (GameControl)
                PlayerInfo = CreateWindowExA(0, TEXT("STATIC"), NULL,
                                             WS_VISIBLE | WS_CHILD | SS_BITMAP,
                                             10, 14, 89, 126, GameControl, (HMENU)ID_CONTROLPLAYER, (HINSTANCE) GetWindowLongPtr(GameControl, GWLP_HINSTANCE), NULL);
            
            GameState.Display = {};
            //HWND Health;
            //HWND Portrait;
            GameState.Display.Control = GameControl;
            GameState.Display.PlayerInfo = PlayerInfo;
            
            
            // load image and display it
            // TODO(jon):  Do i need to persist this??
            GameState.Display.Bitmap = (HBITMAP) LoadImageA(NULL, "images/control.BMP", IMAGE_BITMAP,
                                                            0, 0, LR_LOADFROMFILE| LR_DEFAULTSIZE);
            
            if (GameState.Display.Bitmap)
                SendDlgItemMessage(Window, ID_CONTROLBACKGROUND, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)GameState.Display.Bitmap);
            
            GameState.Display.PlayerInfoBitmap = (HBITMAP) LoadImageA(NULL, "images/player_info.BMP", IMAGE_BITMAP,
                                                                      0, 0, LR_LOADFROMFILE| LR_DEFAULTSIZE);
            
            if (GameState.Display.PlayerInfoBitmap)
                SendDlgItemMessage(GameControl, ID_CONTROLPLAYER, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)GameState.Display.PlayerInfoBitmap);
        } break;
        
        case WM_PAINT:
        {
            
#if 0
            GetUpdateRect(Window, &rToPaint, FALSE);
            RECT rToPaint;
            if (!GetUpdateRect(Window, &rToPaint, FALSE))
                break;    //No regions to update, leave procedure
#endif
            if (GameState.User.Player.Name)
                win32_UpdateClient();
            
            Result = DefWindowProc(Window, Message, WParam, LParam);
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
                       HIWORD(LParam)-190,     // height of client area 
                       TRUE);                 // repaint window 
            
            MoveWindow(GameState.GameInput.Window, 
                       5, HIWORD(LParam)-180,  // starting x- and y-coordinates 
                       LOWORD(LParam)-10,     // width of client area 
                       20,                    // height of client area 
                       TRUE);                 // repaint window 
            
            MoveWindow(GameState.Display.Control,
                       5, HIWORD(LParam)-150,  // starting x- and y-coordinates 
                       450,                    // width of client area 
                       143,                    // height of client area 
                       TRUE);                  // repaint window 
            /*
            MoveWindow(GameState.Display.PlayerInfo,
                       116, HIWORD(LParam)-164,// starting x- and y-coordinates 
                       89,                     // width of client area 
                       126,                    // height of client area 
                       TRUE);                  // repaint window 
            */
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
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        }
    }
    
    return Result;
}

internal void
Win32HandleKeyboardInput(MSG *Message)
{
    uint32 VKCode = (uint32)Message->wParam;
    bool32 WasDown = ((Message->lParam & (1 << 30)) != 0);
    bool32 IsDown = ((Message->lParam & (1 << 31)) == 0);
    
    if (VKCode == VK_BACK || VKCode == VK_DELETE)
    {
        TranslateMessage(Message);
        DispatchMessageA(Message);
    }
    else if (WasDown != IsDown)
    {
        bool32 AltKeyWasDown = ((Message->lParam & (1 << 29)) != 0);
        if ((VKCode == VK_F4) && AltKeyWasDown)
        {
            GlobalRunning = false;
            return;
        }
        
        if (Message->message == WM_KEYDOWN)
        {
            // TODO(jon):  no numpad/numlock option
            if (VKCode == VK_NUMPAD0)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, "look");
            }
            else if (VKCode == VK_NUMPAD1)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak southwest" : "southwest");
            }
            else if (VKCode == VK_NUMPAD2)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak south" : "south");
            }
            else if (VKCode == VK_NUMPAD3)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak southeast" : "southeast");
            }
            else if (VKCode == VK_NUMPAD4)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak west" : "west");
            }
            else if (VKCode == VK_NUMPAD5 || VKCode == VK_CLEAR)
            {
                GameState.AutoSneak = !GameState.AutoSneak;
                win32_EchoCommand(GameState.GameOutput.Window, 
                                  GameState.AutoSneak 
                                  ? "Autosneak toggled ON.\n" 
                                  : "Autosneak toggled OFF.\n");
            }
            else if (VKCode == VK_NUMPAD6)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak east" : "east");
            }
            else if (VKCode == VK_NUMPAD7)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak northwest" : "northwest");
            }
            else if (VKCode == VK_NUMPAD8)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak north" : "north");
            }
            else if (VKCode == VK_NUMPAD9)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak northeast" : "northeast");
            }
            else if (VKCode == VK_ADD)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak down" : "down");
            }
            else if (VKCode == VK_SUBTRACT)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak up" : "up");
            }
            else if (VKCode == VK_MULTIPLY)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak genportal" : "go genportal");
            }
            else if (VKCode == VK_DIVIDE)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, "open genportal");
            }
            else if (VKCode == VK_DECIMAL)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, "hide");
            }
            else if (VKCode == VK_UP)
            {
                CycleThroughUserInputHistory(1);
            }
            else if (VKCode == VK_DOWN)
            {
                CycleThroughUserInputHistory(-1);
            }
            else if (VKCode == VK_TAB)
            {
                SetFocus(GameState.GameInput.Window);
            }
            else if (VKCode == VK_RETURN)
            {
                int inputLength = GetWindowTextA(GameState.GameInput.Window,
                                                 (LPSTR) GameState.CommandHistory.CurrentCommand,
                                                 GameState.CommandHistory.CurrentSize);
                if (inputLength > 0)
                {
                    UpdateCommandHistory();
                    win32_SendInputThroughSocket(Socket.sock, GameState);
                }
            }
            else if (VKCode == VK_ESCAPE)
            {
                SetWindowTextA(GameState.GameInput.Window, "");
                GameState.CommandHistory.CurrentPosition = -1;
            }
            else
            {
                TranslateMessage(Message);
                DispatchMessageA(Message);
            }
        }
        else
        {
            TranslateMessage(Message);
            DispatchMessageA(Message);
        }
    }
}

internal void
Win32ProcessPendingMessages()
{
    MSG Message;
    while (PeekMessageA(&Message, nullptr, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                Win32HandleKeyboardInput(&Message);
            } break;
            
            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            }
        }
    }
}

int CALLBACK
WinMain(
        HINSTANCE   Instance,
        HINSTANCE   PrevInstance,
        LPSTR       CommandLine,
        int         ShowCode)
{
	win32_state Win32State = {};
    
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
            
            // Set starting Text Color
            GameState.CurrentColor = C_RESET;
            
            // Set autosneak
            GameState.AutoSneak = false;
            
            GameState.isInitialized = true;
            
#if OLMRAN_INTERNAL
			LPVOID BaseAddress = ((LPVOID)Terabytes(2));
#else
			LPVOID BaseAddress = 0;
#endif
			game_memory GameMemory = {};
			GameMemory.PermanentStorageSize = Megabytes(64);
			GameMemory.TransientStorageSize = Gigabytes(1);
			//GameMemory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
			//GameMemory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
			//GameMemory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
			
			Win32State.TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
			Win32State.GameMemoryBlock = VirtualAlloc(BaseAddress, (size_t)Win32State.TotalSize,
													  MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			GameMemory.PermanentStorage = Win32State.GameMemoryBlock;
			GameMemory.TransientStorage = ((uint8 *)GameMemory.PermanentStorage + 
										   GameMemory.PermanentStorageSize);
            if (GameMemory.PermanentStorage && GameMemory.TransientStorage)
			{
                if (!GameMemory.IsInitialized)
                {
                    GameState.GameInput.BufferLength = 512;
                    //GameState.GameOutput.BufferLength = 4096;
                    
                    GameState.CommandHistory.BufferSize = 512;
                    GameState.CommandHistory.NumberOfCommands = 10;
                    GameState.CommandHistory.CurrentPosition = -1;
                    
                    GameState.CommandHistory.CurrentSize = 512;
                    
                    GameState.GMCP.BufferSize = 1024;
                    
                    GameState.User.Account = {};
                    GameState.User.Player = {};
                    
                    uint32 accum = 0;
                    
                    GameState.GameInput.Buffer = (char *) GameMemory.TransientStorage;
                    accum += GameState.GameInput.BufferLength;
                    /*
                    GameState.GameOutput.Buffer = (char *) (&GameMemory.TransientStorage + accum);
                    accum += GameState.GameOutput.BufferLength;
                    */
                    GameState.CommandHistory.Commands = (char *) ((uint8 *) GameMemory.TransientStorage + accum);
                    accum += (GameState.CommandHistory.BufferSize * GameState.CommandHistory.NumberOfCommands);
                    
                    GameState.CommandHistory.CurrentCommand = (char *) ((uint8 *) GameMemory.TransientStorage + accum);
                    accum += GameState.CommandHistory.BufferSize;
                    
                    GameState.GMCP.BufferIn = (char *) ((uint8 *) GameMemory.TransientStorage + accum);
                    accum += GameState.GMCP.BufferSize;
                    
                    GameState.GMCP.BufferOut = (char *) ((uint8 *) GameMemory.TransientStorage + accum);
                    accum += GameState.GMCP.BufferSize;
                    
                    GameState.MIDIDevice = (midi_device *) ((uint8 *) GameMemory.TransientStorage + accum);
                    accum += sizeof(midi_device);
                    
                    GameMemory.IsInitialized = true;
                }
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
                    Win32ProcessPendingMessages();
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
            // TODO(jon): Logging (failed to init memory)
        }
    }
    else
    {
        // TODO(jon):  Logging (failed)
    }
    
    return 0;
}
