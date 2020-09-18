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

internal void
Win32CreateMacroWindow(HWND Window)
{
    //char* szBuffer[]={"Ordinary Box","Box Of Junk","Beer Crate","Wine Box","Candy Box"};
    DWORD dwStyle=WS_CHILD|WS_VISIBLE|WS_TABSTOP;
    HWND hCtl;
    
    HINSTANCE hIns = (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE);
    
    uint8 LabelWidth  = 75;
    uint8 LabelHeight = 25;
    
    uint8 EditWidth  = 255;
    uint8 EditHeight = 25;
    
    uint8 LabelX = 10;
    uint8 EditX  = 100;
    uint16 PosY = 10;
    
    for (uint16 Index = 0; Index < MAX_MACROS; Index++)
    {
        hCtl=CreateWindowA("static",MacroLabels[Index],WS_CHILD|WS_VISIBLE,LabelX,PosY,LabelWidth,LabelHeight,Window,(HMENU)-1,hIns,0);
        hCtl=CreateWindowExA(WS_EX_CLIENTEDGE,"edit","",dwStyle,EditX,PosY,EditWidth,EditHeight,Window,(HMENU)MacroIDs[Index],hIns,0);
        SendMessageA(hCtl, EM_LIMITTEXT, (WPARAM)255, 0);
        PosY += 40;
    }
    
    hCtl=CreateWindowA("button","Save",dwStyle,245,PosY,120,30,Window,(HMENU)IDC_MACRO_SAVE,hIns,0);
    hCtl=CreateWindowA("button","Cancel",dwStyle,380,PosY,120,30,Window,(HMENU)IDC_MACRO_CANCEL,hIns,0);
    
    SetWindowTextA(Window,"Global Macros");
    SetFocus(GetDlgItem(Window,IDC_MACRO_F1));
}

internal void
UpdateGlobalMacros(HWND MacroWindow)
{
    const int MACRO_SIZE = 256;
    char Macro[MACRO_SIZE];
    memset(Macro, 0, MACRO_SIZE);
    
    memset(GameState.GlobalMacros.Macros, 0, GameState.GlobalMacros.BufferSize);
    
    // iterate over each MacroID and update GameState
    for (int Index = 0; Index < MAX_MACROS; Index++)
    {
        GetWindowTextA(GetDlgItem(MacroWindow,MacroIDs[Index]), Macro, MACRO_SIZE);
        strcpy_s(GameState.GlobalMacros.Macros + (Index * MACRO_SIZE), MACRO_SIZE, Macro);
    }
}

LRESULT CALLBACK
Win32MacroWindowCallback(HWND   Window,
                         UINT   Message,
                         WPARAM WParam,
                         LPARAM LParam)
{
    LRESULT Result = 0;
    HBRUSH hbrWhite = {}, hbrGray = {};
    
    switch(Message)
    {
        case WM_CREATE:
        {
            hbrWhite = (HBRUSH) GetStockObject(WHITE_BRUSH); 
            hbrGray  = (HBRUSH) GetStockObject(GRAY_BRUSH); 
            
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
                    UpdateGlobalMacros(Window);
                    ShowWindow(Window, SW_HIDE);
                } break;
                
                case IDC_MACRO_CANCEL:
                {
                    // reset state
                    ShowWindow(Window, SW_HIDE);
                } break;
            }
            
        } break;
        
#if 0
        case WM_ERASEBKGND: 
        {
            HDC hdc = (HDC) WParam; 
            RECT rc;
            GetClientRect(Window, &rc); 
            SetMapMode(hdc, MM_ANISOTROPIC); 
            SetWindowExtEx(hdc, 100, 100, NULL); 
            SetViewportExtEx(hdc, rc.right, rc.bottom, NULL); 
            FillRect(hdc, &rc, hbrWhite); 
            
            for (int i = 0; i < 13; i++) 
            { 
                int x = (i * 40) % 100; 
                int y = ((i * 40) / 100) * 20; 
                SetRect(&rc, x, y, x + 20, y + 20); 
                FillRect(hdc, &rc, hbrGray); 
            } 
            return 1L; 
        } break;
#endif
        
        case WM_PAINT:
        {
            //Win32HandlePaint(GameState.Window, 0);
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
win32_MainWindowCallback(HWND   Window,
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
            GameControl = CreateWindowExA(0, TEXT("STATIC"), NULL,
                                          WS_VISIBLE | WS_CHILD | SS_EDITCONTROL | SS_CENTER,
                                          0, 0, 0, 0, Window, (HMENU)ID_CONTROLBACKGROUND, 
                                          (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE), NULL);
            
            if (GameControl)
            {
                Vitals = CreateWindowExA(0, TEXT("STATIC"), NULL, WS_CHILD | SS_BITMAP,
                                         10, 8, 61, 112, GameControl, (HMENU)ID_CONTROLVITALS, 
                                         (HINSTANCE) GetWindowLongPtr(GameControl, GWLP_HINSTANCE), NULL);
                
                Portrait = CreateWindowExA(0, TEXT("STATIC"), NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP,
                                           89, 9, 111, 126, GameControl, (HMENU)ID_CONTROLPORTRAIT, 
                                           (HINSTANCE) GetWindowLongPtr(GameControl, GWLP_HINSTANCE), NULL);
                
                PlayerInfo = CreateWindowExA(0, TEXT("STATIC"), NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP,
                                             174, 14, 106, 126, GameControl, (HMENU)ID_CONTROLPLAYER, 
                                             (HINSTANCE) GetWindowLongPtr(GameControl, GWLP_HINSTANCE), NULL);
                
                Compass = CreateWindowExA(0, TEXT("STATIC"), NULL, WS_CHILD | SS_BITMAP,
                                          108, 115, 330, 21, GameControl, (HMENU)ID_CONTROLCOMPASS, 
                                          (HINSTANCE) GetWindowLongPtr(GameControl, GWLP_HINSTANCE), NULL);
                
                ActionTimer = CreateWindowExA(0, TEXT("STATIC"), NULL, WS_CHILD | SS_BITMAP,
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
            //Win32HandlePaint(GameState.SubWindows.Macros, 0);
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
                    SendMessageA(Window, WM_CLOSE, 0, 0);
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
                        
                        win32_StopMIDIPlayback(GameState.MIDIDevice);
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
                        win32_PlayMIDIFile(GameState.MIDIDevice, GameState.Window, "audio/dark1.mid");
                    
                } break;
                
                case IDM_MUSIC_DARK2:
                {
                    CheckMenuRadioItem(hSubMenuMusic, IDM_MUSIC_DARK1, IDM_MUSIC_DARK4, 
                                       IDM_MUSIC_DARK2, MF_BYCOMMAND);
                    
                    UINT Play = GetMenuState(hSubMenuMusic, IDM_MUSIC_ENABLED, MF_BYCOMMAND); 
                    if (Play == MF_CHECKED)
                        win32_PlayMIDIFile(GameState.MIDIDevice, GameState.Window, "audio/dark2.mid");
                    
                } break;
                
                case IDM_MUSIC_DARK3:
                {
                    CheckMenuRadioItem(hSubMenuMusic, IDM_MUSIC_DARK1, IDM_MUSIC_DARK4, 
                                       IDM_MUSIC_DARK3, MF_BYCOMMAND);
                    
                    UINT Play = GetMenuState(hSubMenuMusic, IDM_MUSIC_ENABLED, MF_BYCOMMAND); 
                    if (Play == MF_CHECKED)
                        win32_PlayMIDIFile(GameState.MIDIDevice, GameState.Window, "audio/dark3.mid");
                    
                } break;
                
                case IDM_MUSIC_DARK4:
                {
                    CheckMenuRadioItem(hSubMenuMusic, IDM_MUSIC_DARK1, IDM_MUSIC_DARK4, 
                                       IDM_MUSIC_DARK4, MF_BYCOMMAND);
                    
                    UINT Play = GetMenuState(hSubMenuMusic, IDM_MUSIC_ENABLED, MF_BYCOMMAND); 
                    if (Play == MF_CHECKED)
                        win32_PlayMIDIFile(GameState.MIDIDevice, GameState.Window, "audio/dark4.mid");
                    
                } break;
                
                case IDM_MACRO_PLAYER:
                { } break;
                
                case IDM_MACRO_GLOBAL:
                {
                    ShowWindow(GameState.SubWindows.Macros, SW_SHOW);
                } break;
            }
            
        } break;
        
        case WM_DESTROY:
        {
            // TODO(jon):  Handle this as an error, recreate window?
            DeleteObject(GameState.Display.Bitmap);
            DeleteObject(GameState.Display.PortraitBitmap);
            DeleteObject(GameState.Display.ControlSpritesBitmap);
            GlobalRunning = false;
        } break;
        
        case WM_CLOSE:
        {
            // TODO(jon):  Send quit&y&
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
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, "look");
            }
            else if (VKCode == VK_NUMPAD1 || (!Numlock && VKCode == VK_END))
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                          GameState.AutoSneak ? "sneak southwest" : "southwest");
            }
            else if (VKCode == VK_NUMPAD2 || (!Numlock && VKCode == VK_DOWN))
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                          GameState.AutoSneak ? "sneak south" : "south");
            }
            else if (VKCode == VK_NUMPAD3 || (!Numlock && VKCode == VK_NEXT))
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                          GameState.AutoSneak ? "sneak southeast" : "southeast");
            }
            else if (VKCode == VK_NUMPAD4 || (!Numlock && VKCode == VK_LEFT))
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, 
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
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                          GameState.AutoSneak ? "sneak east" : "east");
            }
            else if (VKCode == VK_NUMPAD7 || (!Numlock && VKCode == VK_HOME))
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                          GameState.AutoSneak ? "sneak northwest" : "northwest");
            }
            else if (VKCode == VK_NUMPAD8 || (!Numlock && VKCode == VK_UP))
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                          GameState.AutoSneak ? "sneak north" : "north");
            }
            else if (VKCode == VK_NUMPAD9 || (!Numlock && VKCode == VK_PRIOR))
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                          GameState.AutoSneak ? "sneak northeast" : "northeast");
            }
            else if (VKCode == VK_ADD)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                          GameState.AutoSneak ? "sneak down" : "down");
            }
            else if (VKCode == VK_SUBTRACT)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                          GameState.AutoSneak ? "sneak up" : "up");
            }
            else if (VKCode == VK_MULTIPLY)
            {
                win32_WriteStringToSocket(Socket.sock, GameState.GameInput, 
                                          GameState.AutoSneak ? "sneak genportal" : "go genportal");
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
    WindowClass.hIcon = (HICON) LoadImageA(NULL, "images/olmran.ico", IMAGE_ICON,
                                           0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
    //    WindowClass.hCursor = ;
    //WindowClass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    WindowClass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
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
                    CreateWindowEx(
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
                {
                    //SetParent(MacroWindowHandle,WindowHandle);
                    GameState.SubWindows.Macros = MacroWindowHandle;
                }
            }
            
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
                    
                    GameState.GlobalMacros.NumberOfMacros = 12;
                    GameState.GlobalMacros.MacroSize = 256;
                    GameState.GlobalMacros.BufferSize = (12 * GameState.GlobalMacros.MacroSize);
                    
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
                    
                    GameState.GlobalMacros.Macros = (char *) ((uint8 *) GameMemory.TransientStorage + accum);
                    accum += (GameState.GlobalMacros.MacroSize * GameState.GlobalMacros.NumberOfMacros);
                    
                    
                    GameMemory.IsInitialized = true;
                }
                DWORD ThreadID;
                HANDLE SocketListenThreadHandle;
                if (win32_InitAndConnectSocket()==0)
                {
                    OutputDebugStringA("Socket Connected\n");
                    
                    TelnetInit(Telnet);
                    
                    char *Param = "Socket listening.\n";
                    
                    SocketListenThreadHandle = 
                        CreateThread(0, 0, SocketListenThreadProc, Param, 0, &ThreadID);
                }
                else
                {
                    Win32AppendText(GameState.GameOutput.Window, TEXT("Could not connect to server.\n"));
                    OutputDebugStringA("Error in win32_InitAndConnectSocket()\n");
                    SocketListenThreadHandle = 0;
                }
                
                LoadConfigSettings();
                Win32UpdateMenus();
                
                GlobalRunning = true;
                while (GlobalRunning)
                {
                    Win32ProcessPendingMessages();
                }
                SaveUserSettings();
                // NOTE(jon):  Is this necessary?  Windows might clean it up itself.
                win32_CloseSocket();
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
