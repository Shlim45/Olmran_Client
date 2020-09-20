
internal void
Win32PopulateMacroWindow(HWND MacroWindow, char *MacroBuffer, char *PlayerName)
{
    // Fill each text control with macros from GameState
    const int MACRO_SIZE = 256;
    char Macro[MACRO_SIZE];
    memset(Macro, 0, MACRO_SIZE);
    
    for (uint16 Index = 0; Index < MAX_MACROS; Index++)
    {
        strcpy_s(Macro, MACRO_SIZE, MacroBuffer + (Index * MACRO_SIZE));
        SetWindowTextA(GetDlgItem(MacroWindow,MacroIDs[Index]), Macro);
    }
    
    if (PlayerName)
    {
        char WindowTitle[30] = "";
        wsprintf(WindowTitle, "%s's Macros", PlayerName);
        SetWindowTextA(MacroWindow,WindowTitle);
    }
    else
        SetWindowTextA(MacroWindow,"Global Macros");
}

internal void
Win32CreateMacroWindow(HWND Window)
{
    DWORD dwStyle=WS_CHILD|WS_VISIBLE|WS_TABSTOP;
    HWND hCtl;
    
    HINSTANCE hIns = (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE);
    
    uint8 LabelWidth  = 75;
    uint8 LabelHeight = 25;
    
    uint16 EditWidth = 600;
    uint8 EditHeight = 25;
    
    uint8 LabelX = 10;
    uint8 EditX  = 100;
    uint16 PosY = 10;
    
    for (uint16 Index = 0; Index < MAX_MACROS; Index++)
    {
        hCtl=CreateWindowA("static",MacroLabels[Index],WS_CHILD|WS_VISIBLE,
                           LabelX,PosY,LabelWidth,LabelHeight,Window,(HMENU)-1,hIns,0);
        hCtl=CreateWindowExA(WS_EX_CLIENTEDGE,"edit","",dwStyle|ES_AUTOHSCROLL,
                             EditX,PosY,EditWidth,EditHeight,Window,(HMENU)MacroIDs[Index],hIns,0);
        SendMessageA(hCtl, EM_LIMITTEXT, 255, 0);
        PosY += 40;
    }
    
    hCtl=CreateWindowA("button","Save",dwStyle|BS_DEFPUSHBUTTON,245,PosY,120,30,Window,(HMENU)IDC_MACRO_SAVE,hIns,0);
    hCtl=CreateWindowA("button","Cancel",dwStyle|BS_PUSHBUTTON,380,PosY,120,30,Window,(HMENU)IDCANCEL,hIns,0);
    
    SetWindowTextA(Window,"Macros");
    SetFocus(GetDlgItem(Window,IDC_MACRO_F1));
}

internal void 
Win32AddMenus(HWND Window) {
    
    HMENU hMenubar;
    HMENU hMenuFile;
    HMENU hMenuEdit;
    HMENU hSubMenuMusic;
    HMENU hSubMenuMacros;
    
    hMenubar = CreateMenu();
    
    // File
    hMenuFile = CreateMenu();
    
    AppendMenuA(hMenuFile, MF_STRING, IDM_FILE_QUIT, "&Quit");
    
    AppendMenuA(hMenubar, MF_POPUP, (UINT_PTR) hMenuFile, "&File");
    
    // Edit
    hMenuEdit      = CreateMenu();
    hSubMenuMusic  = CreatePopupMenu();
    hSubMenuMacros = CreatePopupMenu();
    
    AppendMenuA(hMenuEdit, MF_STRING, IDM_EDIT_ECHO, "&Local Echo");
    AppendMenuA(hMenuEdit, MF_STRING, IDM_EDIT_PERSIST, "&Persist Command");
    AppendMenuA(hMenuEdit, MF_SEPARATOR, 0, NULL);
    
    // Edit->Music
    AppendMenuA(hMenuEdit, MF_STRING | MF_POPUP, (UINT_PTR) hSubMenuMusic, "&Music");
    AppendMenuA(hSubMenuMusic, MF_STRING, IDM_MUSIC_ENABLED, "Pla&y Music");
    AppendMenuA(hSubMenuMusic, MF_STRING, IDM_MUSIC_LOOP, "L&oop");
    AppendMenuA(hSubMenuMusic, MF_STRING, IDM_MUSIC_SHUFFLE, "&Shuffle");
    AppendMenuA(hSubMenuMusic, MF_STRING, IDM_MUSIC_DARK1, "Dark&1");
    AppendMenuA(hSubMenuMusic, MF_STRING, IDM_MUSIC_DARK2, "Dark&2");
    AppendMenuA(hSubMenuMusic, MF_STRING, IDM_MUSIC_DARK3, "Dark&3");
    AppendMenuA(hSubMenuMusic, MF_STRING, IDM_MUSIC_DARK4, "Dark&4");
    
    CheckMenuRadioItem(hSubMenuMusic, IDM_MUSIC_DARK1, IDM_MUSIC_DARK4, 
                       IDM_MUSIC_DARK2, MF_BYCOMMAND);
    
    // Edit->Macros
    AppendMenuA(hMenuEdit, MF_STRING | MF_POPUP, (UINT_PTR) hSubMenuMacros, "M&acros");
    AppendMenuA(hSubMenuMacros, MF_STRING, IDM_MACRO_PLAYER, "&Player");
    AppendMenuA(hSubMenuMacros, MF_STRING, IDM_MACRO_GLOBAL, "&Global");
    
    AppendMenuA(hMenubar, MF_POPUP, (UINT_PTR) hMenuEdit, "&Edit");
    
    SetMenu(Window, hMenubar);
}

internal void
Win32UpdateMenus()
{
    HMENU hMenu = GetMenu(GameState.Window);
    HMENU hSubMenuMusic = GetSubMenu(hMenu, 1);
    
    // Update state from config settings
    if (GameState.User.Account.Flags & FLAG_ECHO)
        CheckMenuItem(hMenu, IDM_EDIT_ECHO, MF_CHECKED);  
    if (GameState.User.Account.Flags & FLAG_PERSIST)
        CheckMenuItem(hMenu, IDM_EDIT_PERSIST, MF_CHECKED);
    if (GameState.User.Account.Flags & FLAG_MUSIC)
        CheckMenuItem(hSubMenuMusic, IDM_MUSIC_ENABLED, MF_CHECKED);
    if (GameState.User.Account.Flags & FLAG_LOOP)
        CheckMenuItem(hSubMenuMusic, IDM_MUSIC_LOOP, MF_CHECKED);
    if (GameState.User.Account.Flags & FLAG_SHUFFLE)
        CheckMenuItem(hSubMenuMusic, IDM_MUSIC_SHUFFLE, MF_CHECKED);
}

internal void
Win32HandlePlayMusic()
{
    HMENU hMenu = GetMenu(GameState.Window);
    HMENU hSubMenuMusic = GetSubMenu(hMenu, 1);
    
    UINT MusicFile = GetMenuState(hSubMenuMusic, IDM_MUSIC_DARK1, MF_BYCOMMAND);
    if (MusicFile & MF_CHECKED)
    {
        Win32PlayMIDIFile(GameState.MIDIDevice, GameState.Window, "audio/dark1.mid");
        return;
    }
    
    MusicFile = GetMenuState(hSubMenuMusic, IDM_MUSIC_DARK2, MF_BYCOMMAND);
    if (MusicFile & MF_CHECKED)
    {
        Win32PlayMIDIFile(GameState.MIDIDevice, GameState.Window, "audio/dark2.mid");
        return;
    }
    
    MusicFile = GetMenuState(hSubMenuMusic, IDM_MUSIC_DARK3, MF_BYCOMMAND);
    if (MusicFile & MF_CHECKED)
    {
        Win32PlayMIDIFile(GameState.MIDIDevice, GameState.Window, "audio/dark3.mid");
        return;
    }
    
    MusicFile = GetMenuState(hSubMenuMusic, IDM_MUSIC_DARK4, MF_BYCOMMAND);
    if (MusicFile & MF_CHECKED)
    {
        Win32PlayMIDIFile(GameState.MIDIDevice, GameState.Window, "audio/dark4.mid");
    }
}

internal bool32
Win32LoadAssets()
{
    // TODO(jon):  Do i need to persist this??
    GameState.Display.Bitmap = (HBITMAP) LoadImageA(NULL, "images/control.BMP", IMAGE_BITMAP,
                                                    0, 0, LR_LOADFROMFILE| LR_DEFAULTSIZE);
    GameState.Display.PortraitBitmap = (HBITMAP) LoadImageA(NULL, "images/portraits.BMP", IMAGE_BITMAP, 
                                                            0, 0, LR_LOADFROMFILE| LR_DEFAULTSIZE);
    GameState.Display.ControlSpritesBitmap = (HBITMAP) LoadImageA(NULL, "images/control_sprites.BMP", IMAGE_BITMAP, 
                                                                  0, 0, LR_LOADFROMFILE| LR_DEFAULTSIZE);
    
    return (GameState.Display.Bitmap && 
            GameState.Display.PortraitBitmap && 
            GameState.Display.ControlSpritesBitmap);
}

internal void
Win32UpdateVitals(HWND Window)
{
    BITMAP bm;
    PAINTSTRUCT ps;
    
    HDC hdc = BeginPaint(Window, &ps);
    if (GameState.Display.ControlSpritesBitmap)
    {
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMem, GameState.Display.ControlSpritesBitmap);
        
        GetObject(GameState.Display.ControlSpritesBitmap, sizeof(bm), &bm);
        
        if (GameState.User.Player.LoggedIn)
        {
            float healthPct = (GameState.User.Player.Vitals.Health / 
                               (GameState.User.Player.Vitals.MaxHealth * 1.0f));
            float fatPct = (GameState.User.Player.Vitals.Fat / 
                            (GameState.User.Player.Vitals.MaxFat * 1.0f));
            float powerPct = (GameState.User.Player.Vitals.Power / 
                              (GameState.User.Player.Vitals.MaxPower * 1.0f));
            
            int Height, Shift;
            
            // health
            Height = (int) (112 * healthPct);
            if (Height > 112) Height = 112;
            Shift = 112 - Height;
            
            BitBlt(hdc,       // device context to draw to
                   0, Shift,      // destination (x, y)
                   13, Height,   // width, height to draw
                   hdcMem,    // source bitmap
                   1, (91+Shift),     // source (x, y)
                   SRCCOPY);  // raster-operation code
            
            // fat
            Height = (int) (112 * fatPct);
            if (Height > 112) Height = 112;
            Shift = 112 - Height;
            
            BitBlt(hdc,       // device context to draw to
                   24, Shift,      // destination (x, y)
                   13, Height,   // width, height to draw
                   hdcMem,    // source bitmap
                   15, (91+Shift),     // source (x, y)
                   SRCCOPY);  // raster-operation code
            
            // power
            Height = (int) (112 * powerPct);
            if (Height > 112) Height = 112;
            Shift = 112 - Height;
            
            BitBlt(hdc,       // device context to draw to
                   48, Shift,      // destination (x, y)
                   13, Height,   // width, height to draw
                   hdcMem,    // source bitmap
                   29, (91+Shift),     // source (x, y)
                   SRCCOPY);  // raster-operation code
        }
        
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
    }
    
    EndPaint(Window, &ps);
}

internal void
Win32UpdatePortrait(HWND Window)
{
    BITMAP bm;
    PAINTSTRUCT ps;
    
    HDC hdc = BeginPaint(Window, &ps);
    if (GameState.Display.PortraitBitmap)
    {
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMem, GameState.Display.PortraitBitmap);
        
        GetObject(GameState.Display.PortraitBitmap, sizeof(bm), &bm);
        
        if (GameState.User.Player.LoggedIn)
            BitBlt(hdc, 0, 0, 111, 126, hdcMem, 
                   (111 * GameState.User.Player.PortraitCoords.X),
                   (126 * GameState.User.Player.PortraitCoords.Y), 
                   SRCCOPY);
        else // force the blank copy
            BitBlt(hdc, 0, 0, 111, 126, hdcMem, (111 * 3), (126 * 10), SRCCOPY);
        
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
    }
    
    EndPaint(Window, &ps);
}

internal void
Win32UpdateActionTimer(HWND Window)
{
    BITMAP bm;
    PAINTSTRUCT ps;
    
    HDC hdc = BeginPaint(Window, &ps);
    if (GameState.Display.ControlSpritesBitmap)
    {
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmOld = 
            (HBITMAP) SelectObject(hdcMem, GameState.Display.ControlSpritesBitmap);
        
        GetObject(GameState.Display.ControlSpritesBitmap, sizeof(bm), &bm);
        
        if (GameState.User.Player.LoggedIn)
        {
            float timerPct = (GameState.User.Player.ActionTimer / 5000.0f);
            
            if (timerPct > 0)
            {
                int Height = (int) (46 * timerPct);
                if (Height > 46) Height = 46;
                int Shift = 46 - Height;
                
                BitBlt(hdc,       // device context to draw to
                       0, Shift,      // destination (x, y)
                       9, Height,   // width, height to draw
                       hdcMem,    // source bitmap
                       43, (157+Shift),     // source (x, y)
                       SRCCOPY);  // raster-operation code
            }
            else
            {
                // draw with height of 0?
                KillTimer(GameState.Display.ActionTimer, ID_ACTIONTIMER); 
            }
        }
        
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
    }
    
    EndPaint(Window, &ps);
}

internal VOID CALLBACK 
ActionTimerProc( 
                HWND hwnd,        // handle to window for timer messages 
                UINT message,     // WM_TIMER message 
                UINT idTimer,     // timer identifier 
                DWORD dwTime)     // current system time 
{
    Win32UpdateActionTimer(GameState.Display.ActionTimer);
    RedrawWindow(GameState.Window, 0, 0, RDW_INVALIDATE);
    
    if (GameState.User.Player.ActionTimer <= 0)
        KillTimer(hwnd, idTimer);
    else if (GameState.User.Player.ActionTimer > 100)
        GameState.User.Player.ActionTimer -= 100;
    else
        GameState.User.Player.ActionTimer = 0;
    
}

internal void
Win32StartActionTimer()
{
    KillTimer(GameState.Display.ActionTimer, ID_ACTIONTIMER); 
    
    UINT_PTR uResult;         // SetTimer's return value 
    
    uResult = SetTimer(GameState.Display.ActionTimer, // handle to main window 
                       ID_ACTIONTIMER,                // timer identifier 
                       100,                           // ms interval 
                       (TIMERPROC) ActionTimerProc);  // timer callback 
    
    if (uResult == 0) 
        OutputDebugStringA("ERROR: No timer is available.\n"); 
}

internal void
Win32UpdateCompass(HWND Window)
{
    BITMAP bm;
    PAINTSTRUCT ps;
    
    HDC hdc = BeginPaint(Window, &ps);
    if (GameState.Display.ControlSpritesBitmap)
    {
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMem, GameState.Display.ControlSpritesBitmap);
        
        GetObject(GameState.Display.ControlSpritesBitmap, sizeof(bm), &bm);
        
        /* NOTE(jon):
- EVERYTHING has 1px (alpha) padding
   - NWSE is 29x29, starts at (1, 31) on spritesheet
- NW, NE, SE, SW start at (43, 108) and are 16x16
- Up, Out, Down start at (1, 204) and are 10x10
*/
        
        if (GameState.User.Player.LoggedIn)
        {
            if (GameState.Room.Exits & DIR_SW)
            {
                BitBlt(hdc,       // device context to draw to
                       10, 57,    // destination (x, y)
                       16, 16,    // width, height to draw
                       hdcMem,    // source bitmap
                       94, 108,   // source (x, y)
                       SRCCOPY);  // raster-operation code
            }
            
            if (GameState.Room.Exits & DIR_S)
            {
                BitBlt(hdc,       // device context to draw to
                       27, 53,    // destination (x, y)
                       29, 29,    // width, height to draw
                       hdcMem,    // source bitmap
                       61, 31,    // source (x, y)
                       SRCCOPY);  // raster-operation code
            }
            
            if (GameState.Room.Exits & DIR_SE)
            {
                BitBlt(hdc,       // device context to draw to
                       57, 57,    // destination (x, y)
                       16, 16,    // width, height to draw
                       hdcMem,    // source bitmap
                       77, 108,   // source (x, y)
                       SRCCOPY);  // raster-operation code
            }
            
            if (GameState.Room.Exits & DIR_W)
            {
                BitBlt(hdc,       // device context to draw to
                       1, 27,     // destination (x, y)
                       29, 29,    // width, height to draw
                       hdcMem,    // source bitmap
                       31, 31,    // source (x, y)
                       SRCCOPY);  // raster-operation code
            }
            
            if (GameState.Room.Exits & DIR_E)
            {
                BitBlt(hdc,       // device context to draw to
                       54, 27,    // destination (x, y)
                       29, 29,    // width, height to draw
                       hdcMem,    // source bitmap
                       91, 31,    // source (x, y)
                       SRCCOPY);  // raster-operation code
            }
            
            if (GameState.Room.Exits & DIR_NW)
            {
                BitBlt(hdc,       // device context to draw to
                       10, 10,    // destination (x, y)
                       16, 16,    // width, height to draw
                       hdcMem,    // source bitmap
                       43, 108,   // source (x, y)
                       SRCCOPY);  // raster-operation code
            }
            
            if (GameState.Room.Exits & DIR_N)
            {
                BitBlt(hdc,       // device context to draw to
                       27, 0,     // destination (x, y)
                       29, 29,    // width, height to draw
                       hdcMem,    // source bitmap
                       1, 31,     // source (x, y)
                       SRCCOPY);  // raster-operation code
            }
            
            if (GameState.Room.Exits & DIR_NE)
            {
                BitBlt(hdc,       // device context to draw to
                       57, 10,    // destination (x, y)
                       16, 16,    // width, height to draw
                       hdcMem,    // source bitmap
                       60, 108,    // source (x, y)
                       SRCCOPY);  // raster-operation code
            }
            
            if (GameState.Room.Exits & DIR_U)
            {
                BitBlt(hdc,       // device context to draw to
                       97, 70,    // destination (x, y)
                       10, 10,    // width, height to draw
                       hdcMem,    // source bitmap
                       12, 204,   // source (x, y)
                       SRCCOPY);  // raster-operation code
            }
            
            if (GameState.Room.Exits & DIR_D)
            {
                BitBlt(hdc,       // device context to draw to
                       97, 105,   // destination (x, y)
                       10, 10,    // width, height to draw
                       hdcMem,    // source bitmap
                       12, 226,   // source (x, y)
                       SRCCOPY);  // raster-operation code
            }
        }
        
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
    }
    
    EndPaint(Window, &ps);
}

internal void 
Win32UpdatePlayerInfo(HWND Window)
{
    // Set up the device context for drawing.
    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(Window, &ps);
    
    // Create font
    long lfHeight = -MulDiv(10, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    HFONT hf = 
        CreateFontA(lfHeight, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
    HFONT hfOld = (HFONT) SelectObject(hDC, hf);
    
    // Calculate the dimensions of the 4 equal rectangles.
    RECT rcWindow;
    GetClientRect(Window, &rcWindow);
    
    // Draw the text into the center of each of the rectangles.
    SetBkMode(hDC, TRANSPARENT);
    SetBkColor(hDC, RGB(0, 0, 0));   // black
    
    char pInfo[256] = "";
    memset(pInfo, 0, 256);
    if (GameState.User.Player.LoggedIn)
        wsprintf(pInfo, "%s\nLvl %d %s\n%s of\n%s\n%d\nexp for Lvl %d", 
                 GameState.User.Player.Name, 
                 GameState.User.Player.Level, 
                 GameState.User.Player.Class, 
                 GameState.User.Player.Title, 
                 GameState.User.Player.Guild, 
                 GameState.User.Player.ExpTNL, 
                 GameState.User.Player.Level + 1);
    
    DrawTextA(hDC, pInfo, -1, &rcWindow, 
              DT_EDITCONTROL | DT_CENTER | DT_TOP | DT_NOCLIP | DT_WORDBREAK);
    
    // Cleanup
    SelectObject(hDC, hfOld);
    DeleteObject(hf);
    DeleteObject(hfOld);
    
    EndPaint(Window, &ps);
}

internal void
Win32HandlePaint(HWND Window, HBITMAP Bitmap)
{
    // NOTE(jon):  Draw the background of the control
    BITMAP bm;
    PAINTSTRUCT ps;
    
    HDC hdc = BeginPaint(Window, &ps);
    if (Bitmap)
    {
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMem, Bitmap);
        
        GetObject(Bitmap, sizeof(bm), &bm);
        
        BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
        
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
    }
    
    EndPaint(Window, &ps);
}

internal void
Win32UpdateClientTitle()
{
    char newTitle[256] = {};
    
    if (GameState.User.Player.LoggedIn)
        wsprintf(newTitle, "Olmran Client - %s", GameState.User.Player.Name);
    else
        strcat_s(newTitle, 256, "Olmran Client");
    
    SetWindowTextA(GameState.Window, newTitle);
}

internal void
Win32HandlePlayerLogin()
{
    GameState.User.Player.LoggedIn = true;
    // TODO(jon):  Start this elsewhere?  Need on player logon
    Win32UpdateClientTitle();
    ShowWindow(GameState.Display.Vitals, SW_SHOW);
    ShowWindow(GameState.Display.Compass, SW_SHOW);
    ShowWindow(GameState.Display.ActionTimer, SW_SHOW);
    
    RedrawWindow(GameState.Window, 0, 0, RDW_INVALIDATE);
    
    LoadPlayerMacros(GameState.User.Player.Name);
    
    if (GameState.User.Account.Flags & FLAG_MUSIC)
        Win32HandlePlayMusic();
}

internal void
Win32HandlePlayerLogoff()
{
    SavePlayerSettings(GameState.User.Player.Name);
    memset(&GameState.User.Player, 0, sizeof(GameState.User.Player));
    memset(GameState.Macros.Player.MacroBuffer, 0, GameState.Macros.BufferSize);
    
    Win32UpdateClientTitle();
    ShowWindow(GameState.Display.Vitals, SW_HIDE);
    ShowWindow(GameState.Display.Compass, SW_HIDE);
    ShowWindow(GameState.Display.ActionTimer, SW_HIDE);
    
    RedrawWindow(GameState.Window, 0, 0, RDW_INVALIDATE);
    Win32StopMIDIPlayback(GameState.MIDIDevice);
}

internal void 
Win32AppendText(const HWND GameOutput, const char *newText)
{
    if (strlen(newText) == 0)
        return;
    
    LRESULT start_lines, end_lines;
    start_lines = SendMessageA(GameOutput, EM_GETLINECOUNT,0,0);
    
    CHARFORMATA cf;
    memset( &cf, 0, sizeof cf );
    cf.cbSize = sizeof cf;
    cf.dwMask = CFM_COLOR;
    cf.crTextColor = GameState.CurrentColor;
    SendMessageA( GameOutput, EM_SETCHARFORMAT, (LPARAM)SCF_SELECTION, (LPARAM) &cf);
    
    CHARRANGE cr;
    cr.cpMin = -1;
    cr.cpMax = -1;
    SendMessageA(GameOutput, EM_EXSETSEL, 0, (LPARAM)&cr);
    SendMessageA(GameOutput, EM_REPLACESEL, 0, (LPARAM)newText);
    
    end_lines = SendMessageA(GameOutput, EM_GETLINECOUNT,0,0);
    SendMessageA(GameOutput, EM_LINESCROLL, 0, end_lines - start_lines);
}

internal void
Win32EchoCommand(const HWND GameOutput, const char *Command)
{
    GameState.CurrentColor = D_WHITE;
    Win32AppendText(GameState.GameOutput.Window, Command);
    Win32AppendText(GameState.GameOutput.Window, "\n");
    GameState.CurrentColor = C_RESET;
}

internal HWND
CreateGameOutput(HWND hwndOwner,        // Dialog box handle.
                 int x, int y,          // Location.
                 int width, int height, // Dimensions.
                 HMENU controlId,       // Control ID.
                 HINSTANCE hinst)       // Application or DLL instance.
{
    LoadLibraryA("Riched32.dll");
    
    HWND hwndOutput = CreateWindowExA(0, RICHEDIT_CLASS, NULL,
                                      ES_MULTILINE | ES_READONLY | WS_VISIBLE | 
                                      WS_CHILD | WS_BORDER | WS_VSCROLL, 
                                      x, y, width, height, 
                                      hwndOwner, controlId, hinst, NULL);
    
    // Set Background Color
    SendMessageA( hwndOutput, EM_SETBKGNDCOLOR, 0, RGB(50,50,50) );
    
    // Set Font
    HFONT Font; 
    
    // Retrieve a handle to the variable stock font.  
    Font = (HFONT)GetStockObject(OEM_FIXED_FONT); 
    SendMessageA(hwndOutput, WM_SETFONT, (WPARAM)Font, TRUE);
    
    return hwndOutput;
}

internal HWND
CreateGameInput(HWND hwndOwner, HMENU controlId, HINSTANCE hinst)
{
    HWND hwndInput = CreateWindowExA(
                                     0, "EDIT",   // predefined class 
                                     NULL,         // no window title 
                                     WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL, 
                                     0, 0, 0, 0,   // set size in WM_SIZE message 
                                     hwndOwner,         // parent window 
                                     controlId,   // edit control ID 
                                     hinst, 
                                     NULL);        // pointer not needed 
    
    SendMessageA(hwndInput, EM_LIMITTEXT, 254, 0);
    
    return hwndInput;
}
