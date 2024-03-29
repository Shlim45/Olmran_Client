#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif
#include <VersionHelpers.h>

/*********************************************************
* FUNCTION: GetNumScrollLines
* Returns : UINT:  Number of scroll lines where WHEEL_PAGESCROLL 
*           indicates to scroll a page at a time.
*********************************************************/
internal UINT 
GetNumScrollLines()
{
    UINT ucNumLines=3;  // 3 is the default
    
    if (IsWindowsXPOrGreater())
        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ucNumLines, 0);
    
    return ucNumLines;
}

internal void
Win32CreateWindow(WNDCLASSA *WindowClass, WNDPROC WindowProc, 
                  const char *WindowClassName, const char *WindowTitle, 
                  HINSTANCE Instance, HWND *Handle,
                  uint32 ClassStyle, uint32 WindowStyle,
                  int Width, int Height, int xPos, int yPos)
{
    WindowClass->style = ClassStyle;
    WindowClass->lpfnWndProc = WindowProc;
    WindowClass->hInstance = Instance;
    WindowClass->hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    WindowClass->lpszClassName = WindowClassName;
    
    if(RegisterClass(WindowClass))
    {
        HWND WindowHandle =
            CreateWindowExA(
                            0,
                            WindowClass->lpszClassName,
                            WindowTitle,
                            WindowStyle,
                            xPos,
                            yPos,
                            Width,
                            Height,
                            0,
                            0,
                            Instance,
                            0);
        
        if(WindowHandle)
            *Handle = WindowHandle;
    }
    
}

internal void
HandleUserLogin()
{
    if (GameState.User.Account.Flags & FLAG_LOGGEDIN)
        return;
    
    char Username[51];
    char Password[51];
    memset(Username, 0, 51);
    memset(Password, 0, 51);
    
    int unLength = GetWindowTextA(GetDlgItem(GameState.SubWindows.Login,IDC_LOGIN_USERNAME), Username, 50);
    int pwLength = GetWindowTextA(GetDlgItem(GameState.SubWindows.Login,IDC_LOGIN_PASSWORD), Password, 50);
    
    if (unLength < 3 || pwLength < 3)
    {
        MessageBoxA(GameState.SubWindows.Login,
                    "Please enter a valid username and password.",
                    "Fields Required",
                    MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL
                    );
        return;
    }
    
    if (Username && Password)
    {
        Win32WriteStringToSocket(Socket.sock, GameState.GameInput, Username);
        Win32WriteStringToSocket(Socket.sock, GameState.GameInput, Password);
    }
}

LRESULT CALLBACK
Win32LoginWindowCallback(HWND   Window,
                         UINT   Message,
                         WPARAM WParam,
                         LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_CREATE:
        {
            Win32CreateLoginWindow(Window);
        } break;
        
        case WM_SHOWWINDOW:
        case WM_ACTIVATEAPP:
        {
            BringWindowToTop(Window);
        } break;
        
        case WM_CLOSE:
        case WM_DESTROY:
        {
            ShowWindow(Window, SW_HIDE);
        } break;
        
        case DM_GETDEFID: break;
        case DM_SETDEFID: break;
        
        default: Result = DefWindowProcA(Window, Message, WParam, LParam);
    }
    
    return Result;
}

LRESULT CALLBACK
Win32MacroWindowCallback(HWND   Window,
                         UINT   Message,
                         WPARAM WParam,
                         LPARAM LParam)
{
    //HDC hdc; 
    //PAINTSTRUCT ps; 
    SCROLLINFO si;
    local_persist int xClient; // width of client area 
    local_persist int yClient; // height of client area 
    local_persist int yChar;   // v scroll unit
    local_persist int yPos;    // v scroll position
    
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_CREATE:
        {
            Win32CreateMacroWindow(Window);
            yChar = 40;
        } break;
        
        case WM_SIZE: 
        {
            // Retrieve the dimensions of the client area. 
            yClient = HIWORD (LParam); 
            xClient = LOWORD (LParam); 
            
            // Set the vertical scrolling range and page size
            si.cbSize = sizeof(si); 
            si.fMask  = SIF_RANGE | SIF_PAGE; 
            si.nMin   = 0; 
            si.nMax   = MAX_MACROS + 1; 
            si.nPage  = yClient / yChar; 
            SetScrollInfo(Window, SB_VERT, &si, TRUE); 
        } break; 
        
        case WM_VSCROLL:
        {
            // Get all the vertial scroll bar information.
            si.cbSize = sizeof (si);
            si.fMask  = SIF_ALL;
            GetScrollInfo (Window, SB_VERT, &si);
            
            // Save the position for comparison later on.
            yPos = si.nPos;
            switch (LOWORD (WParam))
            {
                
                // User clicked the HOME keyboard key.
                case SB_TOP:
                si.nPos = si.nMin;
                break;
                
                // User clicked the END keyboard key.
                case SB_BOTTOM:
                si.nPos = si.nMax;
                break;
                
                // User clicked the top arrow.
                case SB_LINEUP:
                si.nPos -= 1;
                break;
                
                // User clicked the bottom arrow.
                case SB_LINEDOWN:
                si.nPos += 1;
                break;
                
                // User clicked the scroll bar shaft above the scroll box.
                case SB_PAGEUP:
                si.nPos -= si.nPage;
                break;
                
                // User clicked the scroll bar shaft below the scroll box.
                case SB_PAGEDOWN:
                si.nPos += si.nPage;
                break;
                
                // User dragged the scroll box.
                case SB_THUMBTRACK:
                si.nPos = si.nTrackPos;
                break;
                
                default:
                break; 
            }
            
            // Set the position and then retrieve it.  Due to adjustments
            // by Windows it may not be the same as the value set.
            si.fMask = SIF_POS;
            SetScrollInfo (Window, SB_VERT, &si, TRUE);
            GetScrollInfo (Window, SB_VERT, &si);
            
            // If the position has changed, scroll window and update it.
            if (si.nPos != yPos)
            {                    
                ScrollWindow(Window, 0, yChar * (yPos - si.nPos), NULL, NULL);
                UpdateWindow (Window);
            }
            
        } break;
        
        case WM_MOUSEWHEEL:
        {
            // Get all the vertial scroll bar information.
            si.cbSize = sizeof (si);
            si.fMask  = SIF_ALL;
            GetScrollInfo (Window, SB_VERT, &si);
            
            int yScroll = (GET_WHEEL_DELTA_WPARAM(WParam) / WHEEL_DELTA) * GetNumScrollLines();
            
            // Save the position for comparison later on.
            yPos = si.nPos;
            
            // Set the position and then retrieve it.  Due to adjustments
            // by Windows it may not be the same as the value set.
            si.nPos -= yScroll;
            si.fMask = SIF_POS;
            SetScrollInfo (Window, SB_VERT, &si, TRUE);
            GetScrollInfo (Window, SB_VERT, &si);
            
            // If the position has changed, scroll window and update it.
            if (si.nPos != yPos)
            {                    
                ScrollWindow(Window, 0, yChar * (yPos - si.nPos), NULL, NULL);
                UpdateWindow (Window);
            }
            
        } break;
        
        case WM_PAINT :
        {
            Win32HandlePaint(Window, 0);
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
                
                case IDCANCEL:
                {
                    ShowWindow(Window, SW_HIDE);
                } break;
            }
            
        } break;
        
        case WM_CLOSE:
        case WM_DESTROY:
        {
            ShowWindow(Window, SW_HIDE);
        } break;
        
        case DM_GETDEFID: break;
        case DM_SETDEFID: break;
        
        default:
        {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        }
    }
    
    return Result;
}

internal void
Win32ResizeGameOutput()
{
    RECT ClientRect = {};
    if (GetClientRect(GameState.Window, &ClientRect))
    {
        bool32 ChatWindow = (GameState.User.Account.Flags & FLAG_CHAT);
        uint16 ChatHeight = 150;
        
        MoveWindow(GameState.GameOutput.Window, 
                   5, 5 + (ChatWindow ? ChatHeight : 0), // starting x- and y-coordinates 
                   ClientRect.right-10,     // width of client area 
                   ClientRect.bottom-(190 + (ChatWindow ? ChatHeight : 0)),    // height of client area 
                   TRUE);                 // repaint window 
    }
}

internal void
Win32HandleGameResize(uint16 NewWidth, uint16 NewHeight)
{
    bool32 ChatWindow = (GameState.User.Account.Flags & FLAG_CHAT);
    uint16 ChatHeight = 150;
    // Make the control the size of the window's client 
    MoveWindow(GameState.GameOutput.Window, 
               5, 5 + (ChatWindow ? ChatHeight : 0), // starting x- and y-coordinates 
               NewWidth-10,     // width of client area 
               NewHeight-(190 + (ChatWindow ? ChatHeight : 0)),    // height of client area 
               TRUE);                 // repaint window 
    
    MoveWindow(GameState.GameChat.Window, 
               5, 5,                  // starting x- and y-coordinates 
               NewWidth-10,     // width of client area 
               ChatHeight,                   // height of client area 
               TRUE);                 // repaint window 
    
    MoveWindow(GameState.GameInput.Window, 
               5, NewHeight-180,
               NewWidth-10,
               20,
               TRUE);
    
    MoveWindow(GameState.Display.Control,
               5, NewHeight-150,
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
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND   Window,
                        UINT   Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    LRESULT Result   = 0;
    
    HWND GameOutput  = {};
    HWND GameInput   = {};
    HWND GameChat    = {};
    HWND GameControl = {};
    HWND Vitals      = {};
    HWND Portrait    = {};
    HWND PlayerInfo  = {};
    HWND Compass     = {};
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
            
            // Create Game Chat
            GameChat = CreateGameOutput(Window, 0, 0, 0, 0, (HMENU) ID_CHATCHILD,
                                        (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE));
            GameState.GameChat = {};
            GameState.GameChat.Window = GameChat;
            
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
            
            SetFocus(GameState.GameInput.Window);
            
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
            Win32HandleGameResize(LOWORD(LParam), HIWORD(LParam));
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
                
                case IDM_EDIT_CHAT:
                {
                    UINT state = GetMenuState(hMenu, IDM_EDIT_CHAT, MF_BYCOMMAND); 
                    
                    if (state == MF_CHECKED) 
                    {
                        GameState.User.Account.Flags &= ~FLAG_CHAT;
                        Win32ResizeGameOutput();
                        ShowWindow(GameState.GameChat.Window, SW_HIDE);
                        CheckMenuItem(hMenu, IDM_EDIT_CHAT, MF_UNCHECKED);
                    } else 
                    {
                        GameState.User.Account.Flags |= FLAG_CHAT;
                        Win32ResizeGameOutput();
                        ShowWindow(GameState.GameChat.Window, SW_SHOW);
                        CheckMenuItem(hMenu, IDM_EDIT_CHAT, MF_CHECKED);  
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
