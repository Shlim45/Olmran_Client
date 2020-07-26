#include "ansi.h"

internal COLORREF
GetANSIColor(const char *ANSI)
{
    // black
    if (strcmp(ANSI, "\u001B[30m") == 0
        || strcmp(ANSI, "\u001B[0;30m") == 0
        || strcmp(ANSI, "\u001B[40m") == 0)
        return D_BLACK;
    else if (strcmp(ANSI, "\u001B[1;30m") == 0)
        return B_BLACK;
    
    // red
    else if (strcmp(ANSI, "\u001B[31m") == 0
             || strcmp(ANSI, "\u001B[0;31m") == 0
             || strcmp(ANSI, "\u001B[41m") == 0)
        return D_RED;
    else if (strcmp(ANSI, "\u001B[1;31m") == 0)
        return B_RED;
    
    // green
    else if (strcmp(ANSI, "\u001B[32m") == 0
             || strcmp(ANSI, "\u001B[0;32m") == 0
             || strcmp(ANSI, "\u001B[42m") == 0)
        return D_GREEN;
    else if (strcmp(ANSI, "\u001B[1;32m") == 0)
        return B_GREEN;
    
    // yellow
    else if (strcmp(ANSI, "\u001B[33m") == 0
             || strcmp(ANSI, "\u001B[0;33m") == 0
             || strcmp(ANSI, "\u001B[43m") == 0)
        return D_YELLOW;
    else if (strcmp(ANSI, "\u001B[1;33m") == 0)
        return B_YELLOW;
    
    // blue
    else if (strcmp(ANSI, "\u001B[34m") == 0
             || strcmp(ANSI, "\u001B[0;34m") == 0
             || strcmp(ANSI, "\u001B[44m") == 0)
        return D_BLUE;
    else if (strcmp(ANSI, "\u001B[1;34m") == 0)
        return B_BLUE;
    
    // magenta
    else if (strcmp(ANSI, "\u001B[35m") == 0
             || strcmp(ANSI, "\u001B[0;35m") == 0
             || strcmp(ANSI, "\u001B[45m") == 0)
        return D_MAGENTA;
    else if (strcmp(ANSI, "\u001B[1;35m") == 0)
        return B_MAGENTA;
    
    // cyan
    else if (strcmp(ANSI, "\u001B[36m") == 0
             || strcmp(ANSI, "\u001B[0;36m") == 0
             || strcmp(ANSI, "\u001B[46m") == 0)
        return D_CYAN;
    else if (strcmp(ANSI, "\u001B[1;36m") == 0)
        return B_CYAN;
    
    // white
    else if (strcmp(ANSI, "\u001B[37m") == 0
             || strcmp(ANSI, "\u001B[0;37m") == 0
             || strcmp(ANSI, "\u001B[47m") == 0)
        return D_WHITE;
    
    // C_RESET
    else if (strcmp(ANSI, "\u001B[0") == 0
             || strcmp(ANSI, "\u001B[0m") == 0
             || strcmp(ANSI, "\u001B[0;0m") == 0)
        return C_RESET;
    else
        return B_WHITE;
}

// NOTE(jon): takes the processed input from recv AFTER telnet filtered
internal void
ParseBufferForANSI(char *strbuf)
{
    if (strlen(strbuf) > 0)
    {
        uint16 aPos = 0;    // current char position in strbuf
        uint16 aIndex = 0;  // index of LAST Escape sequence
        uint16 mIndex = 0;  // index of "m" terminating Escape sequence
        const int tmpBufSize = Kilobytes(4);
        char tmpbuf[tmpBufSize] = {0};
        char *head;
        head = strbuf;
        bool32 stillSearching = true;  // true until no more Escape sequences
        bool32 processing = false;
        uint16 strLength = (uint16) strlen(strbuf);
        
        while (stillSearching && aPos < strLength && aPos != tmpBufSize)
        {
            if (strbuf[aPos] == '\u001B')
            {
                aIndex = aPos;
                int advanceAmount = mIndex == 0 ? aIndex : (aIndex-(mIndex+1));
                memset(tmpbuf, 0, tmpBufSize);
                strncpy_s(tmpbuf,head,advanceAmount);
                win32_AppendText(GameState.GameOutput.Window, tmpbuf);
                head += advanceAmount; // do after
                processing = true;
            }
            else if (processing && strbuf[aPos] == 'm')
            {
                mIndex = aPos;
                memset(tmpbuf, 0, tmpBufSize);
                strncpy_s(tmpbuf,head,mIndex+1-aIndex); 
                GameState.CurrentColor = GetANSIColor(tmpbuf);
                head += strlen(tmpbuf);
                processing = false;
            }
            else if(processing && strbuf[aPos] == '0')
            {
                // NOTE(jon):  Attempt at handling "\u001B[0"
                if (aPos < (tmpBufSize+1))
                {
                    if (strbuf[aPos+1] != ';' && strbuf[aPos+1] != 'm')
                    {
                        mIndex = aPos;
                        memset(tmpbuf, 0, tmpBufSize);
                        strncpy_s(tmpbuf,head,mIndex+1-aIndex); 
                        GameState.CurrentColor = GetANSIColor(tmpbuf);
                        head += strlen(tmpbuf);
                        processing = false;
                    }
                }
            }
            else if (strbuf[aPos] == '\0')
                stillSearching = false;
            aPos++;
#if 0
            mIndex = strstr(head, "m");  // find first escape
            if (mIndex==nullptr)
            {
                // TODO(jon):  This buffer ends halfway thru the ansi string!
                // handle
                stillSearching = false;
                continue;
            }
            else
            {
                strncpy(tmpbuf,head,mIndex+1);
            }
#endif
        }
        
        if (aIndex == 0)
        {
            win32_AppendText(GameState.GameOutput.Window, strbuf);
        }
        else if (!processing && strlen(head) != 0)
        {
            int copySize = mIndex+1-aPos;
            if (copySize > 0)
            {
                memset(tmpbuf, 0, tmpBufSize);
                strncpy_s(tmpbuf,head,copySize);
                win32_AppendText(GameState.GameOutput.Window, tmpbuf);
            }
        }
        else
        {
            // TODO(jon):  buffer ended in middle of processing ANSI
            OutputDebugStringA("ANSI: buffer ended without closing ansi sequence");
        }
    }
}
