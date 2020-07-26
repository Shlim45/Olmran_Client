
/* https://stackoverflow.com/questions/16329007/win32-appending-text-to-an-edit-control */
internal void 
win32_AppendText(const HWND GameOutput, const char *newText)
{
    CHARFORMAT cf;
    memset( &cf, 0, sizeof cf );
    cf.cbSize = sizeof cf;
    cf.dwMask = CFM_COLOR;
    cf.crTextColor = RGB(186,218,85);// <----- the color of the text
    SendMessageA( GameOutput, EM_SETCHARFORMAT, (LPARAM)SCF_SELECTION, (LPARAM) &cf);
    
    CHARRANGE cr;
    cr.cpMin = -1;
    cr.cpMax = -1;
    SendMessageA(GameOutput, EM_EXSETSEL, 0, (LPARAM)&cr);
    SendMessageA(GameOutput, EM_REPLACESEL, 0, (LPARAM)newText);
}

internal HWND
CreateGameOutput(HWND hwndOwner,        // Dialog box handle.
                 int x, int y,          // Location.
                 int width, int height, // Dimensions.
                 HMENU controlId,       // Control ID.
                 HINSTANCE hinst)       // Application or DLL instance.
{
    LoadLibrary(TEXT("Riched32.dll"));
    
    HWND hwndEdit= CreateWindowExA(0, RICHEDIT_CLASS, NULL,
                                   ES_MULTILINE | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL, 
                                   x, y, width, height, 
                                   hwndOwner, controlId, hinst, NULL);
    
    // Set Background Color
    SendMessageA( hwndEdit, EM_SETBKGNDCOLOR, 0, RGB(50,50,50) );
    
    return hwndEdit;
}

global_variable WNDPROC oldEditProc;

LRESULT CALLBACK InputEditProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_RETURN:
                {
                    win32_SendInputThroughSocket(Socket.sock, GameState);
                    return 0;
                } break;
                
                case VK_ESCAPE:
                {
                    SetWindowTextA(GameState.GameInput, "");
                    return 0;
                } break;
                
                case VK_MBUTTON:
                case VK_END:
                {
                    // TODO(jon):  middle mouse button / end key, scroll game output to EOF
                } //break;
                
                case VK_PRIOR:
                {
                    // TODO(jon):  page up
                } //break;
                
                case VK_NEXT:
                {
                    // TODO(jon):  page down
                } //break;
                
                case VK_UP:
                {
                    // TODO(jon):  up arrow, cycle through input history
                } //break;
                
                case VK_DOWN:
                {
                    // TODO(jon):  down arrow, cycle through input history
                } //break;
                
                case VK_NUMPAD0:
                {
                    // TODO(jon):  NUMPAD 0 - 9
                } //break;
                
                default:
                {
                    return CallWindowProc(oldEditProc, wnd, msg, wParam, lParam);
                }
            }
        } break;
        default:
        return CallWindowProc(oldEditProc, wnd, msg, wParam, lParam);
    }
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
    
    // Set Background Color
    SendMessageA( hwndInput, EM_SETBKGNDCOLOR, 0, RGB(50,50,50) );
    
    CHARFORMAT cf;
    memset( &cf, 0, sizeof cf );
    cf.cbSize = sizeof cf;
    cf.dwMask = CFM_COLOR;
    cf.crTextColor = RGB(186,218,85);// <----- the color of the text
    SendMessageA( hwndInput, EM_SETCHARFORMAT, (LPARAM)SPF_SETDEFAULT, (LPARAM) &cf);
    
    
    oldEditProc = (WNDPROC)SetWindowLongPtr(hwndInput, GWLP_WNDPROC, (LONG_PTR)InputEditProc);
    
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
