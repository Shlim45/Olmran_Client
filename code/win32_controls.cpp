
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
win32_UpdateClientTitle()
{
    char newTitle[256] = {};
    if (GameState.User.Account.LoggedIn)
    {
        strcat_s(newTitle, 256, "Olmran Client - ");
        strcat_s(newTitle, 256, GameState.User.Player.Name);
    }
    else
        strcat_s(newTitle, 256, "Olmran Client");
    SetWindowTextA(GameState.Window, newTitle);
}

internal void 
win32_AppendText(const HWND GameOutput, const char *newText)
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
win32_EchoCommand(const HWND GameOutput, const char *Command)
{
    GameState.CurrentColor = D_WHITE;
    win32_AppendText(GameState.GameOutput.Window, Command);
    GameState.CurrentColor = C_RESET;
}

internal HWND
CreateGameOutput(HWND hwndOwner,        // Dialog box handle.
                 int x, int y,          // Location.
                 int width, int height, // Dimensions.
                 HMENU controlId,       // Control ID.
                 HINSTANCE hinst)       // Application or DLL instance.
{
    LoadLibrary(TEXT("Riched32.dll"));
    
    HWND hwndOutput = CreateWindowExA(0, RICHEDIT_CLASS, NULL,
                                      ES_MULTILINE | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL, 
                                      x, y, width, height, 
                                      hwndOwner, controlId, hinst, NULL);
    
    // Listen for hot keys
    //OldOutputEditProc = (WNDPROC)SetWindowLongPtr(hwndOutput, GWLP_WNDPROC, (LONG_PTR)OutputEditProc);
    
    
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
                                     0, TEXT("EDIT"),   // predefined class 
                                     NULL,         // no window title 
                                     WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_LEFT, 
                                     0, 0, 0, 0,   // set size in WM_SIZE message 
                                     hwndOwner,         // parent window 
                                     controlId,   // edit control ID 
                                     hinst, 
                                     NULL);        // pointer not needed 
    
    //OldInputEditProc = (WNDPROC)SetWindowLongPtr(hwndInput, GWLP_WNDPROC, (LONG_PTR)InputEditProc);
    
    return hwndInput;
}
