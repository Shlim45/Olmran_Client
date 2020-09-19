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

LRESULT CALLBACK
Win32MacroWindowCallback(HWND   Window,
                         UINT   Message,
                         WPARAM WParam,
                         LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_CREATE:
        {
            Win32CreateMacroWindow(Window);
        } break;
        
        case WM_SHOWWINDOW:
        case WM_ACTIVATEAPP:
        {
            BringWindowToTop(Window);
        } break;
        
        // TODO(jon):  Listen for enter keys, either insert '&' or
        // HandleMacroSave();
        
        case WM_COMMAND:
        {
            switch(LOWORD(WParam)) 
            {
                case IDC_MACRO_SAVE:
                {
                    // TODO(jon):  Better method of determining global vs player
                    char Title[30] = "";
                    GetWindowTextA(Window, Title, 30);
                    if (strcmp(Title, "Global Macros")==0)
                        UpdateMacroBuffer(Window, GameState.Macros.Global.MacroBuffer);
                    else
                        UpdateMacroBuffer(Window, GameState.Macros.Player.MacroBuffer);
                    
                    SaveUserSettings();
                    ShowWindow(Window, SW_HIDE);
                } break;
                
                case IDC_MACRO_CANCEL:
                {
                    ShowWindow(Window, SW_HIDE);
                } break;
            }
            
        } break;
        
        case WM_PAINT:
        {
            Win32HandlePaint(Window, 0);
        } break;
        
        case WM_CLOSE:
        case WM_DESTROY:
        {
            ShowWindow(Window, SW_HIDE);
        } break;
        
        default:
        {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        }
    }
    
    return Result;
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND   Window,
                        UINT   Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    LRESULT Result = 0;
    HWND GameOutput = {};
    HWND GameInput = {};
    HWND GameControl = {};
    HWND Vitals = {};
    HWND Portrait = {};
    HWND PlayerInfo = {};
    HWND Compass = {};
    HWND ActionTimer = {};
    
    local_persist HMENU hMenu;         // handle to main menu 
    local_persist HMENU hSubMenuMusic;
    
    switch(Message)
    {
        case WM_CREATE:
        {
            Win32AddMenus(Window);
            hMenu = GetMenu(Window);
            hSubMenuMusic = GetSubMenu(hMenu, 1);
            
            // Create Edit Control
            GameOutput = CreateGameOutput(Window, 0, 0, 0, 0, (HMENU) ID_EDITCHILD,
                                          (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE));
            GameState.GameOutput = {};
            GameState.GameOutput.Window = GameOutput;
            
            // Create Input Control
            GameInput = CreateGameInput(Window, (HMENU) ID_INPUTCHILD, 
                                        (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE));
            GameState.GameInput = {};
            GameState.GameInput.Window = GameInput;
            
            // Create Static Control for Health Bars / Portrait / Player Info Background Image
            GameControl = CreateWindowExA(0, "STATIC", NULL,
                                          WS_VISIBLE | WS_CHILD | SS_EDITCONTROL | SS_CENTER,
                                          0, 0, 0, 0, Window, (HMENU)ID_CONTROLBACKGROUND, 
                                          (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE), NULL);
            
            if (GameControl)
            {
                Vitals = CreateWindowExA(0, "STATIC", NULL, WS_CHILD | SS_BITMAP,
                                         10, 8, 61, 112, GameControl, (HMENU)ID_CONTROLVITALS, 
                                         (HINSTANCE) GetWindowLongPtr(GameControl, GWLP_HINSTANCE), NULL);
                
                Portrait = CreateWindowExA(0, "STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP,
                                           89, 9, 111, 126, GameControl, (HMENU)ID_CONTROLPORTRAIT, 
                                           (HINSTANCE) GetWindowLongPtr(GameControl, GWLP_HINSTANCE), NULL);
                
                PlayerInfo = CreateWindowExA(0, "STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP,
                                             174, 14, 106, 126, GameControl, (HMENU)ID_CONTROLPLAYER, 
                                             (HINSTANCE) GetWindowLongPtr(GameControl, GWLP_HINSTANCE), NULL);
                
                Compass = CreateWindowExA(0, "STATIC", NULL, WS_CHILD | SS_BITMAP,
                                          108, 115, 330, 21, GameControl, (HMENU)ID_CONTROLCOMPASS, 
                                          (HINSTANCE) GetWindowLongPtr(GameControl, GWLP_HINSTANCE), NULL);
                
                ActionTimer = CreateWindowExA(0, "STATIC", NULL, WS_CHILD | SS_BITMAP,
                                              305, 91, 9, 46, GameControl, (HMENU)ID_CONTROLTIMER, 
                                              (HINSTANCE) GetWindowLongPtr(GameControl, GWLP_HINSTANCE), NULL);
                
                GameState.Display = {};
                GameState.Display.Control = GameControl;
                GameState.Display.Vitals = Vitals;
                GameState.Display.Portrait = Portrait;
                GameState.Display.PlayerInfo = PlayerInfo;
                GameState.Display.Compass = Compass;
                GameState.Display.ActionTimer = ActionTimer;
            }
            
            if (!Win32LoadAssets())
                Win32AppendText(GameState.GameOutput.Window, "Failed to load asset(s).\n");
            
        } break;
        
        case WM_PAINT:
        {
            Win32HandlePaint(GameState.Window, 0);
            Win32HandlePaint(GameState.Display.Control, GameState.Display.Bitmap);
            Win32UpdateVitals(GameState.Display.Vitals);
            Win32UpdatePortrait(GameState.Display.Portrait);
            Win32UpdatePlayerInfo(GameState.Display.PlayerInfo);
            Win32UpdateCompass(GameState.Display.Compass);
            Win32UpdateActionTimer(GameState.Display.ActionTimer);
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
                       5, HIWORD(LParam)-180,
                       LOWORD(LParam)-10,
                       20,
                       TRUE);
            
            MoveWindow(GameState.Display.Control,
                       5, HIWORD(LParam)-150,
                       450,
                       143,
                       TRUE);
            
            MoveWindow(GameState.Display.Vitals,
                       10, 8,                  // starting x- and y-coordinates 
                       61,                     // width of client area 
                       112,                    // height of client area 
                       TRUE);                  // repaint window 
            
            MoveWindow(GameState.Display.Portrait,
                       89, 9,                  // starting x- and y-coordinates 
                       111,                    // width of client area 
                       126,                    // height of client area 
                       TRUE);                  // repaint window 
            
            MoveWindow(GameState.Display.PlayerInfo,
                       174, 14,                // starting x- and y-coordinates 
                       106,                    // width of client area 
                       126,                    // height of client area 
                       TRUE);                  // repaint window 
            
            MoveWindow(GameState.Display.Compass,
                       330, 21,                // starting x- and y-coordinates 
                       108,                    // width of client area 
                       115,                    // height of client area 
                       TRUE);                  // repaint window 
            
            MoveWindow(GameState.Display.ActionTimer,
                       305, 91,                // starting x- and y-coordinates 
                       9,                      // width of client area 
                       46,                     // height of client area 
                       TRUE);                  // repaint window 
            
            
            SetFocus(GameState.GameInput.Window);
        } break;
        
        case WM_COMMAND:
        {
            switch(LOWORD(WParam)) 
            {
                case IDM_FILE_QUIT:
                {
                    if (GameState.User.Player.LoggedIn)
                        HandleMacroString("quit&yes&");
                    else
                        GlobalRunning = false;
                } break;
                
                case IDM_EDIT_ECHO:
                {
                    UINT state = GetMenuState(hMenu, IDM_EDIT_ECHO, MF_BYCOMMAND); 
                    
                    if (state == MF_CHECKED) 
                    {
                        GameState.User.Account.Flags &= ~FLAG_ECHO;
                        CheckMenuItem(hMenu, IDM_EDIT_ECHO, MF_UNCHECKED);  
                    } else 
                    {
                        GameState.User.Account.Flags |= FLAG_ECHO;
                        CheckMenuItem(hMenu, IDM_EDIT_ECHO, MF_CHECKED);  
                    }
                } break;
                
                case IDM_EDIT_PERSIST:
                {
                    UINT state = GetMenuState(hMenu, IDM_EDIT_PERSIST, MF_BYCOMMAND); 
                    
                    if (state == MF_CHECKED) 
                    {
                        GameState.User.Account.Flags &= ~FLAG_PERSIST;
                        CheckMenuItem(hMenu, IDM_EDIT_PERSIST, MF_UNCHECKED);  
                    } else 
                    {
                        GameState.User.Account.Flags |= FLAG_PERSIST;
                        CheckMenuItem(hMenu, IDM_EDIT_PERSIST, MF_CHECKED);  
                    }
                } break;
                
                case IDM_MUSIC_ENABLED:
                {
                    UINT state = GetMenuState(hSubMenuMusic, IDM_MUSIC_ENABLED, MF_BYCOMMAND); 
                    
                    if (state == MF_CHECKED) 
                    {
                        GameState.User.Account.Flags &= ~FLAG_MUSIC;
                        CheckMenuItem(hSubMenuMusic, IDM_MUSIC_ENABLED, MF_UNCHECKED); 
                        
                        Win32StopMIDIPlayback(GameState.MIDIDevice);
                    } else 
                    {
                        GameState.User.Account.Flags |= FLAG_MUSIC;
                        CheckMenuItem(hSubMenuMusic, IDM_MUSIC_ENABLED, MF_CHECKED);
                        
                        Win32HandlePlayMusic();
                    }
                } break;
                
                case IDM_MUSIC_LOOP:
                {
                    UINT state = GetMenuState(hSubMenuMusic, IDM_MUSIC_LOOP, MF_BYCOMMAND); 
                    
                    if (state == MF_CHECKED) 
                    {
                        GameState.User.Account.Flags &= ~FLAG_LOOP;
                        CheckMenuItem(hSubMenuMusic, IDM_MUSIC_LOOP, MF_UNCHECKED);  
                    } else 
                    {
                        GameState.User.Account.Flags |= FLAG_LOOP;
                        CheckMenuItem(hSubMenuMusic, IDM_MUSIC_LOOP, MF_CHECKED);  
                    }
                } break;
                
                case IDM_MUSIC_SHUFFLE:
                {
                    UINT state = GetMenuState(hSubMenuMusic, IDM_MUSIC_SHUFFLE, MF_BYCOMMAND); 
                    
                    if (state == MF_CHECKED) 
                    {
                        GameState.User.Account.Flags &= ~FLAG_SHUFFLE;
                        CheckMenuItem(hSubMenuMusic, IDM_MUSIC_SHUFFLE, MF_UNCHECKED);  
                    } else 
                    {
                        GameState.User.Account.Flags |= FLAG_SHUFFLE;
                        CheckMenuItem(hSubMenuMusic, IDM_MUSIC_SHUFFLE, MF_CHECKED);  
                    }
                } break;
                
                case IDM_MUSIC_DARK1:
                {
                    CheckMenuRadioItem(hSubMenuMusic, IDM_MUSIC_DARK1, IDM_MUSIC_DARK4, 
                                       IDM_MUSIC_DARK1, MF_BYCOMMAND);
                    
                    UINT Play = GetMenuState(hSubMenuMusic, IDM_MUSIC_ENABLED, MF_BYCOMMAND); 
                    if (Play == MF_CHECKED)
                        Win32PlayMIDIFile(GameState.MIDIDevice, GameState.Window, "audio/dark1.mid");
                    
                } break;
                
                case IDM_MUSIC_DARK2:
                {
                    CheckMenuRadioItem(hSubMenuMusic, IDM_MUSIC_DARK1, IDM_MUSIC_DARK4, 
                                       IDM_MUSIC_DARK2, MF_BYCOMMAND);
                    
                    UINT Play = GetMenuState(hSubMenuMusic, IDM_MUSIC_ENABLED, MF_BYCOMMAND); 
                    if (Play == MF_CHECKED)
                        Win32PlayMIDIFile(GameState.MIDIDevice, GameState.Window, "audio/dark2.mid");
                    
                } break;
                
                case IDM_MUSIC_DARK3:
                {
                    CheckMenuRadioItem(hSubMenuMusic, IDM_MUSIC_DARK1, IDM_MUSIC_DARK4, 
                                       IDM_MUSIC_DARK3, MF_BYCOMMAND);
                    
                    UINT Play = GetMenuState(hSubMenuMusic, IDM_MUSIC_ENABLED, MF_BYCOMMAND); 
                    if (Play == MF_CHECKED)
                        Win32PlayMIDIFile(GameState.MIDIDevice, GameState.Window, "audio/dark3.mid");
                    
                } break;
                
                case IDM_MUSIC_DARK4:
                {
                    CheckMenuRadioItem(hSubMenuMusic, IDM_MUSIC_DARK1, IDM_MUSIC_DARK4, 
                                       IDM_MUSIC_DARK4, MF_BYCOMMAND);
                    
                    UINT Play = GetMenuState(hSubMenuMusic, IDM_MUSIC_ENABLED, MF_BYCOMMAND); 
                    if (Play == MF_CHECKED)
                        Win32PlayMIDIFile(GameState.MIDIDevice, GameState.Window, "audio/dark4.mid");
                    
                } break;
                
                case IDM_MACRO_PLAYER:
                {
                    if (GameState.User.Player.LoggedIn)
                    {
                        Win32PopulateMacroWindow(GameState.SubWindows.Macros, GameState.Macros.Player.MacroBuffer, GameState.User.Player.Name);
                        ShowWindow(GameState.SubWindows.Macros, SW_SHOW);
                    }
                } break;
                
                case IDM_MACRO_GLOBAL:
                {
                    Win32PopulateMacroWindow(GameState.SubWindows.Macros, GameState.Macros.Global.MacroBuffer, 0);
                    ShowWindow(GameState.SubWindows.Macros, SW_SHOW);
                } break;
            }
            
        } break;
        
        case WM_DESTROY:
        {
            DeleteObject(GameState.Display.Bitmap);
            DeleteObject(GameState.Display.PortraitBitmap);
            DeleteObject(GameState.Display.ControlSpritesBitmap);
            GlobalRunning = false;
        } break;
        
        case WM_CLOSE:
        {
            if (GameState.User.Player.LoggedIn)
                HandleMacroString("quit&yes&");
            else
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
            else if (VKCode >= VK_F1 && VKCode <= VK_F24)
            {
                HandleFunctionKey(VKCode);
            }
            else
            {
                TranslateMessage(Message);
                DispatchMessageA(Message);
            }
        }
        else if (Message->message == WM_SYSKEYDOWN && !AltKeyWasDown && VKCode == VK_F10)
        {
            HandleFunctionKey(VKCode);
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
                
                GlobalRunning = true;
                while (GlobalRunning)
                {
                    Win32ProcessPendingMessages();
                }
                SaveUserSettings();
                // NOTE(jon):  Is this necessary?  Windows might clean it up itself.
                Win32CloseSocket();
                if (SocketListenThreadHandle) { CloseHandle(SocketListenThreadHandle); }
                KillTimer(GameState.Display.ActionTimer, ID_ACTIONTIMER);
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
