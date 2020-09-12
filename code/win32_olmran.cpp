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

internal bool32
Win32LoadAssets()
{
    // TODO(jon):  Do i need to persist this??
    GameState.Display.Bitmap = (HBITMAP) LoadImageA(NULL, "images/control.BMP", IMAGE_BITMAP,
                                                    0, 0, LR_LOADFROMFILE| LR_DEFAULTSIZE);
    GameState.Display.PortraitBitmap = (HBITMAP) LoadImageA(NULL, "images/portraits.BMP", IMAGE_BITMAP, 
                                                            0, 0, LR_LOADFROMFILE| LR_DEFAULTSIZE);
    GameState.Display.ControlSpritesBitmap = (HBITMAP) LoadImageA(NULL, "images/control_sprites_2.BMP", IMAGE_BITMAP, 
                                                                  0, 0, LR_LOADFROMFILE| LR_DEFAULTSIZE);
    
    return (GameState.Display.Bitmap && 
            GameState.Display.PortraitBitmap && 
            GameState.Display.ControlSpritesBitmap);
}

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
    HWND Portrait = {};
    HWND PlayerInfo = {};
    HWND Compass = {};
    
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
                                          WS_VISIBLE | WS_CHILD | SS_EDITCONTROL | SS_CENTER,
                                          0, 0, 0, 0, Window, (HMENU)ID_CONTROLBACKGROUND, (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE), NULL);
            
            if (GameControl)
            {
                // a portrait is 111 x 126 (93 x 121)
                Portrait = CreateWindowExA(0, TEXT("STATIC"), NULL,
                                           WS_VISIBLE | WS_CHILD | SS_BITMAP,
                                           89, 9, 111, 126, GameControl, (HMENU)ID_CONTROLPORTRAIT, (HINSTANCE) GetWindowLongPtr(GameControl, GWLP_HINSTANCE), NULL);
                
                PlayerInfo = CreateWindowExA(0, TEXT("STATIC"), NULL,
                                             WS_VISIBLE | WS_CHILD | SS_BITMAP,
                                             174, 14, 106, 126, GameControl, (HMENU)ID_CONTROLPLAYER, (HINSTANCE) GetWindowLongPtr(GameControl, GWLP_HINSTANCE), NULL);
                
                Compass = CreateWindowExA(0, TEXT("STATIC"), NULL,
                                          WS_CHILD | SS_BITMAP, // WS_VISIBLE
                                          108, 115, 330, 21, GameControl, (HMENU)ID_CONTROLCOMPASS, (HINSTANCE) GetWindowLongPtr(GameControl, GWLP_HINSTANCE), NULL);
                GameState.Display = {};
                //HWND Health;
                GameState.Display.Control = GameControl;
                GameState.Display.Portrait = Portrait;
                GameState.Display.PlayerInfo = PlayerInfo;
                GameState.Display.Compass = Compass;
            }
            
            if (!Win32LoadAssets())
                win32_AppendText(GameState.GameOutput.Window, "Failed to load asset(s).\n");
            
        } break;
        
        case WM_PAINT:
        {
            Win32HandlePaint(GameState.Window, 0);
            Win32HandlePaint(GameState.Display.Control, GameState.Display.Bitmap);
            Win32UpdatePortrait(GameState.Display.Portrait);
            Win32UpdatePlayerInfo(GameState.Display.PlayerInfo);
            Win32UpdateCompass(GameState.Display.Compass);
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
            
            MoveWindow(GameState.Display.Portrait,
                       89, 9,                // starting x- and y-coordinates 
                       111,                     // width of client area 
                       126,                    // height of client area 
                       TRUE);                  // repaint window 
            
            MoveWindow(GameState.Display.PlayerInfo,
                       174, 14,                // starting x- and y-coordinates 
                       106,                     // width of client area 
                       126,                    // height of client area 
                       TRUE);                  // repaint window 
            
            MoveWindow(GameState.Display.Compass,
                       330, 21,                // starting x- and y-coordinates 
                       108,                     // width of client area 
                       115,                    // height of client area 
                       TRUE);                  // repaint window 
            
            SetFocus(GameState.GameInput.Window);
        } break;
        
        case WM_DESTROY:
        {
            // TODO(jon):  Handle this as an error, recreate window?
            DeleteObject(GameState.Display.Bitmap);
            DeleteObject(GameState.Display.PlayerInfo);
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
    bool32 Numlock = (GetKeyState(VK_NUMLOCK) & 1)!=0;
    
    // TODO(jon):  Numlock should not be the toggle for handling keyboards
    // without a numpad.  Should be a config option, but is a bandaid for now.
    
    if (VKCode == VK_BACK || (Numlock && VKCode == VK_DELETE))
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
            if (VKCode == VK_NUMPAD0 || (!Numlock && VKCode == VK_INSERT))
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, "look");
            }
            else if (VKCode == VK_NUMPAD1 || (!Numlock && VKCode == VK_END))
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak southwest" : "southwest");
            }
            else if (VKCode == VK_NUMPAD2 || (!Numlock && VKCode == VK_DOWN))
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak south" : "south");
            }
            else if (VKCode == VK_NUMPAD3 || (!Numlock && VKCode == VK_NEXT))
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak southeast" : "southeast");
            }
            else if (VKCode == VK_NUMPAD4 || (!Numlock && VKCode == VK_LEFT))
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak west" : "west");
            }
            else if (VKCode == VK_NUMPAD5 || (!Numlock && VKCode == VK_CLEAR))
            {
                GameState.AutoSneak = !GameState.AutoSneak;
                win32_EchoCommand(GameState.GameOutput.Window, 
                                  GameState.AutoSneak 
                                  ? "Autosneak toggled ON.\n" 
                                  : "Autosneak toggled OFF.\n");
            }
            else if (VKCode == VK_NUMPAD6 || (!Numlock && VKCode == VK_RIGHT))
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak east" : "east");
            }
            else if (VKCode == VK_NUMPAD7 || (!Numlock && VKCode == VK_HOME))
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak northwest" : "northwest");
            }
            else if (VKCode == VK_NUMPAD8 || (!Numlock && VKCode == VK_UP))
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, GameState.AutoSneak ? "sneak north" : "north");
            }
            else if (VKCode == VK_NUMPAD9 || (!Numlock && VKCode == VK_PRIOR))
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
            else if (VKCode == VK_DECIMAL || (!Numlock && VKCode == VK_DELETE))
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
                    OutputDebugStringA("Socket Connected\n");
                    
                    TelnetInit(Telnet);
                    
                    char *Param = "Socket listening.\n";
                    
                    SocketListenThreadHandle = CreateThread(0, 0, SocketListenThreadProc, Param, 0, &ThreadID);
                }
                else
                {
                    win32_AppendText(GameState.GameOutput.Window, TEXT("Could not connect to server.\n"));
                    OutputDebugStringA("Error in win32_InitAndConnectSocket()\n");
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
