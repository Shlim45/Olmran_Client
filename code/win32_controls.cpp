
internal void 
win32_AppendText(const HWND GameOutput, const char *newText)
{
    if (strlen(newText) == 0)
        return;
    
    LRESULT start_lines, end_lines;
    start_lines = SendMessage(GameOutput, EM_GETLINECOUNT,0,0);
    
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
    
    end_lines = SendMessage(GameOutput, EM_GETLINECOUNT,0,0);
    SendMessage(GameOutput, EM_LINESCROLL, 0, end_lines - start_lines);
}

internal void
win32_EchoCommand(const HWND GameOutput, const char *Command)
{
    GameState.CurrentColor = D_WHITE;
    win32_AppendText(GameState.GameOutput.Window, Command);
    GameState.CurrentColor = C_RESET;
}

global_variable WNDPROC OldInputEditProc;
global_variable WNDPROC OldOutputEditProc;

LRESULT HandleHotKeys(WNDPROC DefaultWindowProc, HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_CHAR:
        {
            switch ( wParam )
            {
                case VK_RETURN:
                {
                    win32_SendInputThroughSocket(Socket.sock, GameState);
                    return 0;
                } break;
                
                case VK_ESCAPE:
                {
                    SetWindowTextA(GameState.GameInput.Window, "");
                    return 0;
                } break;
                
                
                default:
                return CallWindowProc(DefaultWindowProc, Window, Message, wParam, lParam);
            }
        } break;
        
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            bool32 Numlock = (GetKeyState(VK_NUMLOCK) & 1)!=0;
            bool32 KeyBeingPressed = !(lParam & (1<<31));
            bool32 KeyWasDown = lParam & (1<<30);
            switch (wParam)
            {
                // handled in WM_CHAR listener
                case VK_RETURN:
                case VK_ESCAPE:
                { return 0; } break;
                
                case VK_MBUTTON:
                case VK_END:
                {
                    if (wParam == VK_END && !Numlock && KeyBeingPressed && !KeyWasDown)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak southwest" : "southwest");
                    // TODO(jon):  middle mouse button / end key, scroll game output to last line
                    return 0;
                } break;
                
                case VK_PRIOR:
                {
                    // TODO(jon):  page up
                    if (!Numlock && KeyBeingPressed && !KeyWasDown)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak northeast" : "northeast");
                    return 0;
                } break;
                
                case VK_NEXT:
                {
                    // TODO(jon):  page down
                    if (!Numlock && KeyBeingPressed && !KeyWasDown)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak southeast" : "southeast");
                    return 0;
                } break;
                
                case VK_UP:
                {
                    // TODO(jon):  up arrow, cycle through input history
                    if (!Numlock && KeyBeingPressed && !KeyWasDown)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak north" : "north");
                    return 0;
                } break;
                
                case VK_DOWN:
                {
                    // TODO(jon):  down arrow, cycle through input history
                    if (!Numlock && KeyBeingPressed && !KeyWasDown)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak south" : "south");
                    return 0;
                } break;
                
                case VK_LEFT:
                {
                    if (!Numlock && KeyBeingPressed && !KeyWasDown)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak west" : "west");
                    return 0;
                } break;
                
                case VK_RIGHT:
                {
                    if (!Numlock && KeyBeingPressed && !KeyWasDown)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak east" : "east");
                    return 0;
                } break;
                
                case VK_HOME:
                {
                    if (!Numlock && KeyBeingPressed && !KeyWasDown)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak northwest" : "northwest");
                    return 0;
                } break;
                
                case VK_INSERT:
                {
                    if (!Numlock && KeyBeingPressed && !KeyWasDown)
                        win32_WriteStringToSocket(Socket.sock, GameState, "look");
                    return 0;
                } break;
                
                case VK_DELETE:
                {
                    if (!Numlock && KeyBeingPressed && !KeyWasDown)
                        win32_WriteStringToSocket(Socket.sock, GameState, "hide");
                    return 0;
                } break;
                
                case VK_NUMPAD0:
                {
                    if (Message == WM_KEYDOWN)
                        win32_WriteStringToSocket(Socket.sock, GameState, "look");
                    return 0;
                } break;
                
                case VK_NUMPAD1:
                {
                    if (Message == WM_KEYDOWN)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak southwest" : "southwest");
                    return 0;
                } break;
                
                case VK_NUMPAD2:
                {
                    if (Message == WM_KEYDOWN)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak south" : "south");
                    return 0;
                } break;
                
                case VK_NUMPAD3:
                {
                    if (Message == WM_KEYDOWN)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak southeast" : "southeast");
                    return 0;
                } break;
                
                case VK_NUMPAD4:
                {
                    if (Message == WM_KEYDOWN)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak west" : "west");
                    return 0;
                } break;
                
                case VK_NUMPAD5:
                case VK_CLEAR:
                {
                    if (Message == WM_KEYDOWN)
                    {
                        GameState.AutoSneak = !GameState.AutoSneak;
                        win32_EchoCommand(GameState.GameOutput.Window, 
                                          GameState.AutoSneak 
                                          ? "Autosneak toggled ON.\n" 
                                          : "Autosneak toggled OFF.\n");
                    }
                    
                    return 0;
                } break;
                
                case VK_NUMPAD6:
                {
                    if (Message == WM_KEYDOWN)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak east" : "east");
                    return 0;
                } break;
                
                case VK_NUMPAD7:
                {
                    if (Message == WM_KEYDOWN)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak northwest" : "northwest");
                    return 0;
                } break;
                
                case VK_NUMPAD8:
                {
                    if (Message == WM_KEYDOWN)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak north" : "north");
                    return 0;
                } break;
                
                case VK_NUMPAD9:
                {
                    if (Message == WM_KEYDOWN)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak northeast" : "northeast");
                    return 0;
                } break;
                
                case VK_ADD:
                {
                    if (Message == WM_KEYDOWN)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak down" : "down");
                    return 0;
                } break;
                
                case VK_SUBTRACT:
                {
                    if (Message == WM_KEYDOWN)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak up" : "up");
                    return 0;
                } break;
                
                case VK_MULTIPLY:
                {
                    if (Message == WM_KEYDOWN)
                        win32_WriteStringToSocket(Socket.sock, GameState, GameState.AutoSneak ? "sneak genportal" : "go genportal");
                    return 0;
                } break;
                
                case VK_DIVIDE:
                {
                    if (Message == WM_KEYDOWN)
                        win32_WriteStringToSocket(Socket.sock, GameState, "open genportal");
                    return 0;
                } break;
                
                case VK_DECIMAL:
                {
                    if (Message == WM_KEYDOWN)
                        win32_WriteStringToSocket(Socket.sock, GameState, "hide");
                    return 0;
                } break;
                
                default:
                {
                    return CallWindowProc(DefaultWindowProc, Window, Message, wParam, lParam);
                }
            }
        } break;
        default:
        return CallWindowProc(DefaultWindowProc, Window, Message, wParam, lParam);
    }
}

LRESULT CALLBACK OutputEditProc(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
    // NOTE(jon):  SetFocus here is buggy.  GameInput will accept keyboard input,
    // but the cursor will stay within GameOutput, and cannot be moved out of it.
    //if (Message == WM_KEYDOWN && wParam <= 90) // maybe the wParam check?  excludes keys after z
    //SetFocus(GameState.GameInput.Window);
    return HandleHotKeys(OldOutputEditProc, Window, Message, wParam, lParam);
}

LRESULT CALLBACK InputEditProc(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
    return HandleHotKeys(OldInputEditProc, Window, Message, wParam, lParam);
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
    OldOutputEditProc = (WNDPROC)SetWindowLongPtr(hwndOutput, GWLP_WNDPROC, (LONG_PTR)OutputEditProc);
    
    
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
    
    OldInputEditProc = (WNDPROC)SetWindowLongPtr(hwndInput, GWLP_WNDPROC, (LONG_PTR)InputEditProc);
    
    return hwndInput;
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
        if (SendMessageA(GameState.GameOutput, EM_STREAMIN, SF_TEXT|SFF_SELECTION, (LPARAM)&es) && es.dwError == 0) 
        {
            OutputDebugStringA("Stream success!\n");
        }
        else if (es.dwError != 0)
            OutputDebugStringA("ew.dwError != 0\n");
    }
    
}
#endif
