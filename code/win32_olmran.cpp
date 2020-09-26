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
#include "olmran_file_io.cpp"
#include "olmran_client_features.cpp"
#include "win32_windows.cpp"

internal void
Win32HandleKeyboardInput(MSG *Message)
{
    uint32 VKCode = (uint32)Message->wParam;
    bool32 WasDown = ((Message->lParam & (1 << 30)) != 0);
    bool32 IsDown = ((Message->lParam & (1 << 31)) == 0);
    bool32 Numlock = (GetKeyState(VK_NUMLOCK) & 1)!=0;
    bool32 AltKeyWasDown = ((Message->lParam & (1 << 29)) != 0);
    bool32 ShiftKeyWasDown = (GetKeyState(VK_SHIFT) & 0x8000)!=0;
    bool32 CtrlKeyWasDown = (GetKeyState(VK_CONTROL) & 0x8000)!=0;
    
    // TODO(jon):  Numlock should not be the toggle for handling keyboards
    // without a numpad.  Should be a config option, but is a bandaid for now.
    
    if (VKCode == VK_BACK || (Numlock && VKCode == VK_DELETE))
    {
        TranslateMessage(Message);
        DispatchMessageA(Message);
    }
    else if (WasDown != IsDown)
    {
        if ((VKCode == VK_F4) && AltKeyWasDown)
        {
            SendMessageA(GameState.Window, WM_CLOSE, 0, 0);
            //GlobalRunning = false;
            return;
        }
        
        if (Message->message == WM_KEYDOWN)
        {
            // TODO(jon):  no numpad/numlock option
            if (VKCode == VK_NUMPAD0 || (!Numlock && VKCode == VK_INSERT))
            {
                Win32WriteStringToSocket(Socket.sock, GameState.GameInput, "look");
            }
            else if (VKCode == VK_NUMPAD1 || (!Numlock && VKCode == VK_END))
            {
                Win32WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                         GameState.AutoSneak ? "sneak southwest" : "southwest");
            }
            else if (VKCode == VK_NUMPAD2 || (!Numlock && VKCode == VK_DOWN))
            {
                Win32WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                         GameState.AutoSneak ? "sneak south" : "south");
            }
            else if (VKCode == VK_NUMPAD3 || (!Numlock && VKCode == VK_NEXT))
            {
                Win32WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                         GameState.AutoSneak ? "sneak southeast" : "southeast");
            }
            else if (VKCode == VK_NUMPAD4 || (!Numlock && VKCode == VK_LEFT))
            {
                Win32WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                         GameState.AutoSneak ? "sneak west" : "west");
            }
            else if (VKCode == VK_NUMPAD5 || (!Numlock && VKCode == VK_CLEAR))
            {
                GameState.AutoSneak = !GameState.AutoSneak;
                Win32EchoCommand(GameState.GameOutput.Window, 
                                 GameState.AutoSneak 
                                 ? "Autosneak toggled ON.\n" 
                                 : "Autosneak toggled OFF.\n");
            }
            else if (VKCode == VK_NUMPAD6 || (!Numlock && VKCode == VK_RIGHT))
            {
                Win32WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                         GameState.AutoSneak ? "sneak east" : "east");
            }
            else if (VKCode == VK_NUMPAD7 || (!Numlock && VKCode == VK_HOME))
            {
                Win32WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                         GameState.AutoSneak ? "sneak northwest" : "northwest");
            }
            else if (VKCode == VK_NUMPAD8 || (!Numlock && VKCode == VK_UP))
            {
                Win32WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                         GameState.AutoSneak ? "sneak north" : "north");
            }
            else if (VKCode == VK_NUMPAD9 || (!Numlock && VKCode == VK_PRIOR))
            {
                Win32WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                         GameState.AutoSneak ? "sneak northeast" : "northeast");
            }
            else if (VKCode == VK_ADD)
            {
                Win32WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                         GameState.AutoSneak ? "sneak down" : "down");
            }
            else if (VKCode == VK_SUBTRACT)
            {
                Win32WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                         GameState.AutoSneak ? "sneak up" : "up");
            }
            else if (VKCode == VK_MULTIPLY)
            {
                Win32WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                         GameState.AutoSneak ? "sneak genportal" : "go genportal");
            }
            else if (VKCode == VK_DIVIDE)
            {
                Win32WriteStringToSocket(Socket.sock, GameState.GameInput, "open genportal");
            }
            else if (VKCode == VK_DECIMAL || (!Numlock && VKCode == VK_DELETE))
            {
                Win32WriteStringToSocket(Socket.sock, GameState.GameInput, "hide");
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
                    Win32SendInputThroughSocket(Socket.sock, GameState);
                }
            }
            else if (VKCode == VK_ESCAPE)
            {
                SetWindowTextA(GameState.GameInput.Window, "");
                GameState.CommandHistory.CurrentPosition = -1;
            }
            else if (CtrlKeyWasDown &&
                     (VKCode != 'c' && VKCode != 'C' && VKCode != 'v' && VKCode != 'V') &&
                     ((VKCode >= 'a' && VKCode <= 'z') || (VKCode >= 'A' && VKCode <= 'Z')))
            {
                HandleCtrlKey(VKCode, ShiftKeyWasDown);
            }
            else if (VKCode >= VK_F1 && VKCode <= VK_F24)
            {
                HandleFunctionKey(VKCode, ShiftKeyWasDown);
            }
            else
            {
                TranslateMessage(Message);
                DispatchMessageA(Message);
            }
        }
        else if (Message->message == WM_SYSKEYDOWN && !AltKeyWasDown && VKCode == VK_F10)
        {
            HandleFunctionKey(VKCode, ShiftKeyWasDown);
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
        if (IsDialogMessage(GameState.SubWindows.Macros, &Message)) 
        {
            /* Already handled by dialog manager */
        } 
        else
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
}

internal void
ShutdownClient(HANDLE SocketListenThreadHandle)
{
    SaveUserSettings();
    Win32StopMIDIPlayback(GameState.MIDIDevice);
    // NOTE(jon):  Is this necessary?  Windows might clean it up itself.
    Win32CloseSocket();
    
    if (SocketListenThreadHandle)
        CloseHandle(SocketListenThreadHandle);
    
    KillTimer(GameState.Display.ActionTimer, ID_ACTIONTIMER);
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
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hIcon = (HICON) LoadImageA(NULL, "images/olmran.ico", IMAGE_ICON,
                                           0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
    //    WindowClass.hCursor = ;
    WindowClass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    WindowClass.lpszClassName = "OlmranWindowClass";
    
    if(RegisterClass(&WindowClass))
    {
        HWND WindowHandle =
            CreateWindowExA(
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
            
            // Create other windows
            
            WNDCLASSA MacroWindowClass = {};
            MacroWindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
            MacroWindowClass.lpfnWndProc = Win32MacroWindowCallback;
            MacroWindowClass.hInstance = Instance;
            MacroWindowClass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
            MacroWindowClass.lpszClassName = "MacroWindowClass";
            
            if(RegisterClass(&MacroWindowClass))
            {
                int MacroWidth = 800;
                int MacroHeight = 600;
                int xPos = (GetSystemMetrics(SM_CXSCREEN) - MacroWidth)/2;
                int yPos = (GetSystemMetrics(SM_CYSCREEN) - MacroHeight)/2;
                
                HWND MacroWindowHandle =
                    CreateWindowExA(
                                    0,
                                    MacroWindowClass.lpszClassName,
                                    "Macro Window",
                                    WS_OVERLAPPEDWINDOW | WS_VSCROLL,
                                    xPos,
                                    yPos,
                                    MacroWidth,
                                    MacroHeight,
                                    0,
                                    0,
                                    Instance,
                                    0);
                
                if(MacroWindowHandle)
                    GameState.SubWindows.Macros = MacroWindowHandle;
            }
            
            InitializeGameState(&GameState);
#if OLMRAN_INTERNAL
			LPVOID BaseAddress = ((LPVOID)Terabytes(2));
#else
			LPVOID BaseAddress = 0;
#endif
			game_memory GameMemory = {};
			GameMemory.PermanentStorageSize = Megabytes(64);
			GameMemory.TransientStorageSize = Gigabytes(1);
			
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
                    InitializeGameMemory(&GameState, &GameMemory);
                }
                
                DWORD ThreadID;
                HANDLE SocketListenThreadHandle;
                
                InitializeSocketConnection(&GameState, &SocketListenThreadHandle, &ThreadID);
                
                LoadConfigSettings();
                Win32UpdateMenus();
                
                if (!(GameState.User.Account.Flags & FLAG_CHAT))
                    ShowWindow(GameState.GameChat.Window, SW_HIDE);
                
                GlobalRunning = true;
                while (GlobalRunning)
                {
                    Win32ProcessPendingMessages();
                }
                
                ShutdownClient(SocketListenThreadHandle);
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
