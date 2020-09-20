
internal void
HandleMacroString(char *MacroString)
{
    // Split into string array
    // Send all commands ending in &
    char Command[256];
    memset(Command, 0, 256);
    
    uint8 Index = 0;
    uint8 CommandIndex = 0;
    while (MacroString[Index] != '\0' && Index < 256)
    {
        if (MacroString[Index] == '&' || Index == 255)
        {
            Win32WriteStringToSocket(Socket.sock, GameState.GameInput, Command);
            
            memset(Command, 0, 256);
            CommandIndex = 0;
        }
        else
        {
            // copy char to Command
            Command[CommandIndex] = MacroString[Index];
            ++CommandIndex;
        }
        ++Index;
    }
    
    // Fill input bar otherwise
    if (CommandIndex > 0)
    {
        SetWindowTextA(GameState.GameInput.Window, Command);
        SetCaretToEnd(GameState.GameInput.Window);
    }
}

internal bool32
GetMacroString(char *Macro, char *Buffer, uint16 BufferSize)
{
    uint8 MacroSlot = 0;
    bool32 ValidMacro = false;
    
    for (uint8 Index = 0; Index < MAX_MACROS; Index++)
    {
        if (strcmp(Macro, MacroLabels[Index]) == 0)
        {
            ValidMacro = true;
            MacroSlot = Index;
            break;
        }
    }
    
    if (ValidMacro)
    {
        char *MacroBuffer = GameState.Macros.Player.MacroBuffer + (GameState.Macros.MacroSize * MacroSlot);
        
        // use global if no player macro
        if (strlen(MacroBuffer) == 0)
            MacroBuffer = GameState.Macros.Global.MacroBuffer + (GameState.Macros.MacroSize * MacroSlot);
        
        // Copy macro slot to Buffer
        strcpy_s(Buffer, BufferSize, MacroBuffer);
    }
    
    return ValidMacro;
}

internal void
HandleFunctionKey(uint32 VKCode, bool32 ShiftKey)
{
    if (!GameState.User.Player.LoggedIn)
        return;
    
    bool32 ValidMacro = false;
    char Macro[256];
    memset(Macro, 0, 256);
    
    switch (VKCode)
    {
        case VK_F1:
        {
            ValidMacro = GetMacroString(ShiftKey ? "Shift+F1" : "F1", Macro, 256);
        } break;
        
        case VK_F2:
        {
            ValidMacro = GetMacroString(ShiftKey ? "Shift+F2" : "F2", Macro, 256);
        } break;
        
        case VK_F3:
        {
            ValidMacro = GetMacroString(ShiftKey ? "Shift+F3" : "F3", Macro, 256);
        } break;
        
        case VK_F4:
        {
            ValidMacro = GetMacroString(ShiftKey ? "Shift+F4" : "F4", Macro, 256);
        } break;
        
        case VK_F5:
        {
            ValidMacro = GetMacroString(ShiftKey ? "Shift+F5" : "F5", Macro, 256);
        } break;
        
        case VK_F6:
        {
            ValidMacro = GetMacroString(ShiftKey ? "Shift+F6" : "F6", Macro, 256);
        } break;
        
        case VK_F7:
        {
            ValidMacro = GetMacroString(ShiftKey ? "Shift+F7" : "F7", Macro, 256);
        } break;
        
        case VK_F8:
        {
            ValidMacro = GetMacroString(ShiftKey ? "Shift+F8" : "F8", Macro, 256);
        } break;
        
        case VK_F9:
        {
            ValidMacro = GetMacroString(ShiftKey ? "Shift+F9" : "F9", Macro, 256);
        } break;
        
        case VK_F10:
        {
            ValidMacro = GetMacroString(ShiftKey ? "Shift+F10" : "F10", Macro, 256);
        } break;
        
        case VK_F11:
        {
            ValidMacro = GetMacroString(ShiftKey ? "Shift+F11" : "F11", Macro, 256);
        } break;
        
        case VK_F12:
        {
            ValidMacro = GetMacroString(ShiftKey ? "Shift+F12" : "F12", Macro, 256);
        } break;
    }
    
    if (ValidMacro)
        HandleMacroString(Macro);
}

internal bool32
SetConfigSetting(char *Setting, bool32 Value)
{
    uint16 Flag = 0;
    if (strcmp("ECHO", Setting) == 0)
        Flag = FLAG_ECHO;
    else if (strcmp("PERSIST", Setting) == 0)
        Flag = FLAG_PERSIST;
    else if (strcmp("MUSIC", Setting) == 0)
        Flag = FLAG_MUSIC;
    else if (strcmp("LOOP", Setting) == 0)
        Flag = FLAG_LOOP;
    else if (strcmp("SHUFFLE", Setting) == 0)
        Flag = FLAG_SHUFFLE;
    else
    {
        OutputDebugStringA("ERROR:  Invalid config setting\n");
        return false;
    }
    
    if (Value)
        GameState.User.Account.Flags |= Flag;
    else
        GameState.User.Account.Flags &= ~(Flag);
    
    return true;
}

internal bool32
SetMacroSetting(char *Macro, char *Value, char *MacroBuffer)
{
    uint8 MacroSlot = 0;
    bool32 ValidMacro = false;
    
    for (uint8 Index = 0; Index < MAX_MACROS; Index++)
    {
        if (strcmp(Macro, MacroLabels[Index]) == 0)
        {
            ValidMacro = true;
            MacroSlot = Index;
            break;
        }
    }
    
    if (ValidMacro)
    {
        // Clear this macro slot
        memset(
               MacroBuffer + (GameState.Macros.MacroSize * MacroSlot),
               0, GameState.Macros.MacroSize);
        
        // Copy to macro slot, or leave cleared for empty values
        if (Value)
            strcpy_s(
                     MacroBuffer + (GameState.Macros.MacroSize * MacroSlot),
                     GameState.Macros.MacroSize, Value);
        
    }
    
    return ValidMacro;
}

internal void
LoadConfigSettings()
{
    char ReadBuffer[Kilobytes(64)];
    uint16 ReadIndex     = 0;
    char Setting[16];
    uint16 SettingIndex  = 0;
    char Macro[256];
    uint16 MacroIndex    = 0;
    bool32 Comment       = false;
    bool32 ProcessMacro  = false;
    uint8 MacroSlot = 0;
    
    uint8 ConfigSettingCount = 0;
    
    ReadFromFile("olmran.cfg", ReadBuffer);
    
    memset(Setting, 0, 16);
    memset(Macro, 0, 256);
    
    while (ReadBuffer[ReadIndex])
    {
        Comment = (ReadBuffer[ReadIndex] == '#');
        if (Comment)
        {
            while ((ReadBuffer[ReadIndex])!='\n' && (ReadBuffer[ReadIndex])!='\0')
            {
                ReadIndex++;
                continue;
            }
            Comment = false;
            ReadIndex++;
            continue;
        }
        
        if (ReadBuffer[ReadIndex] == '\n' && !ProcessMacro)
        {
            // Skip blank lines
            ++ReadIndex;
            continue;
        }
        
        if (ConfigSettingCount < SETTINGS_COUNT)
        {
            // handle config setting
            if (ReadBuffer[ReadIndex] == '=')
            {
                if (SetConfigSetting(Setting, ((ReadBuffer[ReadIndex+1]) == 'Y')))
                {
                    ++ConfigSettingCount;
                }
                
                memset(Setting, 0, 16);
                SettingIndex = 0;
                // Skip past Value and Newline
                ReadIndex += 2;
            }
            else
            {
                Setting[SettingIndex] = ReadBuffer[ReadIndex];
                ++SettingIndex;
            }
        }
        else
        {
            // handle macro
            if (ReadBuffer[ReadIndex] == '=')
            {
                ProcessMacro = true;
            }
            else if (ProcessMacro)
            {
                if (ReadBuffer[ReadIndex] != '\n')
                {
                    Macro[MacroIndex] = ReadBuffer[ReadIndex];
                    ++MacroIndex;
                }
                else
                {
                    SetMacroSetting(Setting, Macro, GameState.Macros.Global.MacroBuffer);
                    
                    memset(Setting, 0, 16);
                    SettingIndex = 0;
                    
                    memset(Macro, 0, 256);
                    MacroIndex = 0;
                    ProcessMacro = false;
                }
            }
            else
            {
                Setting[SettingIndex] = ReadBuffer[ReadIndex];
                ++SettingIndex;
            }
        }
        ++ReadIndex;
    }
}

internal void
LoadPlayerMacros(char *PlayerName)
{
    char ReadBuffer[Kilobytes(64)];
    uint16 ReadIndex     = 0;
    char Setting[16];
    uint16 SettingIndex  = 0;
    char Macro[256];
    uint16 MacroIndex    = 0;
    bool32 Comment       = false;
    bool32 ProcessMacro  = false;
    uint8 MacroSlot = 0;
    
    char FileName[30] = "";
    wsprintf(FileName, "%s.cfg", PlayerName);
    ReadFromFile(FileName, ReadBuffer);
    
    memset(Setting, 0, 16);
    memset(Macro, 0, 256);
    
    while (ReadBuffer[ReadIndex])
    {
        Comment = (ReadBuffer[ReadIndex] == '#');
        if (Comment)
        {
            while ((ReadBuffer[ReadIndex])!='\n' && (ReadBuffer[ReadIndex])!='\0')
            {
                ReadIndex++;
                continue;
            }
            Comment = false;
            ReadIndex++;
            continue;
        }
        
        if (ReadBuffer[ReadIndex] == '\n' && !ProcessMacro)
        {
            // Skip blank lines
            ++ReadIndex;
            continue;
        }
        
        // handle macro
        if (ReadBuffer[ReadIndex] == '=')
        {
            ProcessMacro = true;
        }
        else if (ProcessMacro)
        {
            if (ReadBuffer[ReadIndex] != '\n')
            {
                Macro[MacroIndex] = ReadBuffer[ReadIndex];
                ++MacroIndex;
            }
            else
            {
                SetMacroSetting(Setting, Macro, GameState.Macros.Player.MacroBuffer);
                
                memset(Setting, 0, 16);
                SettingIndex = 0;
                
                memset(Macro, 0, 256);
                MacroIndex = 0;
                ProcessMacro = false;
            }
        }
        else
        {
            Setting[SettingIndex] = ReadBuffer[ReadIndex];
            ++SettingIndex;
        }
        ++ReadIndex;
    }
}

internal void
SaveConfigSettings(char *Buffer, uint32 BufferSize)
{
    const char *HashLine = "##################################################\n";
    const char *InnerHash = "##########";
    
    strcat_s(Buffer, BufferSize, HashLine);
    strcat_s(Buffer, BufferSize, InnerHash);
    strcat_s(Buffer, BufferSize, "Client Configuration Settings ");
    strcat_s(Buffer, BufferSize, InnerHash);
    strcat_s(Buffer, BufferSize, "\n");
    strcat_s(Buffer, BufferSize, HashLine);
    strcat_s(Buffer, BufferSize, "\n");
    
    strcat_s(Buffer, BufferSize, "ECHO=");
    if (GameState.User.Account.Flags & FLAG_ECHO)
        strcat_s(Buffer, BufferSize, "Y\n");
    else
        strcat_s(Buffer, BufferSize, "N\n");
    
    strcat_s(Buffer, BufferSize, "PERSIST=");
    if (GameState.User.Account.Flags & FLAG_PERSIST)
        strcat_s(Buffer, BufferSize, "Y\n");
    else
        strcat_s(Buffer, BufferSize, "N\n");
    
    strcat_s(Buffer, BufferSize, "MUSIC=");
    if (GameState.User.Account.Flags & FLAG_MUSIC)
        strcat_s(Buffer, BufferSize, "Y\n");
    else
        strcat_s(Buffer, BufferSize, "N\n");
    
    strcat_s(Buffer, BufferSize, "LOOP=");
    if (GameState.User.Account.Flags & FLAG_LOOP)
        strcat_s(Buffer, BufferSize, "Y\n");
    else
        strcat_s(Buffer, BufferSize, "N\n");
    
    strcat_s(Buffer, BufferSize, "SHUFFLE=");
    if (GameState.User.Account.Flags & FLAG_SHUFFLE)
        strcat_s(Buffer, BufferSize, "Y\n");
    else
        strcat_s(Buffer, BufferSize, "N\n");
}

internal void
SaveGlobalMacros(char *Buffer, uint32 BufferSize)
{
    const char *HashLine = "##################################################\n";
    const char *InnerHash = "##########";
    
    strcat_s(Buffer, BufferSize, "\n");
    strcat_s(Buffer, BufferSize, HashLine);
    strcat_s(Buffer, BufferSize, InnerHash);
    strcat_s(Buffer, BufferSize, "        Global Macros         ");
    strcat_s(Buffer, BufferSize, InnerHash);
    strcat_s(Buffer, BufferSize, "\n");
    strcat_s(Buffer, BufferSize, HashLine);
    strcat_s(Buffer, BufferSize, "\n");
    
    char Macro[256];
    for (uint16 Index = 0; Index < MAX_MACROS; Index++)
    {
        memset(Macro, 0, 256);
        strcpy_s(Macro, 256,
                 GameState.Macros.Global.MacroBuffer + (Index * GameState.Macros.MacroSize));
        strcat_s(Buffer, BufferSize, MacroLabels[Index]);
        strcat_s(Buffer, BufferSize, "=");
        strcat_s(Buffer, BufferSize, Macro);
        strcat_s(Buffer, BufferSize, "\n");
    }
}

internal void
SavePlayerMacros(char *Buffer, uint32 BufferSize)
{
    const char *HashLine = "##################################################\n";
    const char *InnerHash = "##########";
    
    strcat_s(Buffer, BufferSize, "\n");
    strcat_s(Buffer, BufferSize, HashLine);
    strcat_s(Buffer, BufferSize, InnerHash);
    strcat_s(Buffer, BufferSize, "        Player Macros         ");
    strcat_s(Buffer, BufferSize, InnerHash);
    strcat_s(Buffer, BufferSize, "\n");
    strcat_s(Buffer, BufferSize, HashLine);
    strcat_s(Buffer, BufferSize, "\n");
    
    char Macro[256];
    for (uint16 Index = 0; Index < MAX_MACROS; Index++)
    {
        memset(Macro, 0, 256);
        strcpy_s(Macro, 256,
                 GameState.Macros.Player.MacroBuffer + (Index * GameState.Macros.MacroSize));
        strcat_s(Buffer, BufferSize, MacroLabels[Index]);
        strcat_s(Buffer, BufferSize, "=");
        strcat_s(Buffer, BufferSize, Macro);
        strcat_s(Buffer, BufferSize, "\n");
    }
}

internal void
SavePlayerSettings(char *PlayerName)
{
    if (GameState.User.Player.LoggedIn)
    {
        char FileName[30] = "";
        char Buffer[Kilobytes(64)];
        memset(Buffer, 0, Kilobytes(64));
        
        SavePlayerMacros(Buffer, Kilobytes(64));
        
        wsprintf(FileName, "%s.cfg", PlayerName);
        WriteToFile(FileName, Buffer);
    }
}

internal void
SaveUserSettings()
{
    char Buffer[Kilobytes(64)];
    memset(Buffer, 0, Kilobytes(64));
    
    SaveConfigSettings(Buffer, Kilobytes(64));
    SaveGlobalMacros(Buffer, Kilobytes(64));
    
    WriteToFile("olmran.cfg", Buffer);
}