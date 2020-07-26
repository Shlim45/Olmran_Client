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
ANSITest(char *strbuf)
{
    if (strlen(strbuf) > 0)
    {
        uint16 aPos = 0;    // current char position in strbuf
        uint16 aIndex = 0;  // index of next Escape sequence
        uint16 mIndex = 0;  // index of "m" terminating Escape sequence
        local_persist char tmpbuf[512] = {0};
        char *head;
        head = strbuf;
        bool32 stillSearching = true;  // true until no more Escape sequences
        uint16 n = 0;
        bool32 processing = false;
        uint16 strLength = (uint16) strlen(strbuf);
        
        while (stillSearching && n < strLength && n != 512)
        {
            if (strbuf[n] == '\u001B')
            {
                aIndex = n;
                int advanceAmount = mIndex == 0 ? aIndex : (aIndex-(mIndex+1));
                strncpy_s(tmpbuf,head,advanceAmount);
                win32_AppendText(GameState.GameOutput, tmpbuf);
                head += advanceAmount; // do after
                aPos = aIndex;
                processing = true;
                memset(tmpbuf, 0, 512);
            }
            else if (strbuf[n] == 'm' && processing)
            {
                // TODO(jon): handle CRESET not ending in a 0
                mIndex = n;
                strncpy_s(tmpbuf,head,mIndex+1-aPos);
                GetANSIColor(tmpbuf);
                head += strlen(tmpbuf);
                memset(tmpbuf, 0, 512);
                processing = false;
            }
            else if (strbuf[n] == '\0')
                stillSearching = false;
            n++;
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
        
        if (aPos == 0)
        {
            win32_AppendText(GameState.GameOutput, strbuf);
        }
        else if (!processing)
        {
            strncpy_s(tmpbuf,head,strlen(head));
            win32_AppendText(GameState.GameOutput, tmpbuf);
            memset(tmpbuf, 0, 512);
        }
    }
}