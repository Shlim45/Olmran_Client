
internal void
HandleMacroString(char *MacroString)
{
    // e&e&n&e&look 
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
            win32_WriteStringToSocket(Socket.sock, GameState.GameInput, Command);
            
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

internal void
HandleFunctionKey(uint32 VKCode)
{
    switch (VKCode)
    {
        case VK_F1:
        {
            HandleMacroString("n&w&w&n&n&n&w&w&n&e&e&n&n&n&w&n&e&n&e&e&n&e&go tower&w&bandage&look me&wave ");
        } break;
    }
}

internal void
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
        return;
    }
    
    if (Value)
        GameState.User.Account.Flags |= Flag;
    else
        GameState.User.Account.Flags &= ~(Flag);
}

internal void
LoadConfigSettings()
{
    char ReadBuffer[Kilobytes(64)];
    char Setting[16];
    uint16 SettingIndex = 0;
    uint16 ReadIndex   = 0;
    bool32 Comment     = false;
    
    ReadFromFile("olmran.cfg", ReadBuffer);
    
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
        
        if (ReadBuffer[ReadIndex] == '\n')
        {
            // Skip blank lines
            ++ReadIndex;
            continue;
        }
        
        if (ReadBuffer[ReadIndex] == '=')
        {
            SetConfigSetting(Setting, ((ReadBuffer[ReadIndex+1]) == 'Y'));
            
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
SaveMacroSettings(char *Buffer, uint32 BufferSize)
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
                 GameState.GlobalMacros.Macros + (Index * GameState.GlobalMacros.MacroSize));
        strcat_s(Buffer, BufferSize, MacroLabels[Index]);
        strcat_s(Buffer, BufferSize, "=");
        strcat_s(Buffer, BufferSize, Macro);
        strcat_s(Buffer, BufferSize, "\n");
    }
    
    strcat_s(Buffer, BufferSize, "\n");
    strcat_s(Buffer, BufferSize, HashLine);
    strcat_s(Buffer, BufferSize, InnerHash);
    strcat_s(Buffer, BufferSize, "        Player Macros         ");
    strcat_s(Buffer, BufferSize, InnerHash);
    strcat_s(Buffer, BufferSize, "\n");
    strcat_s(Buffer, BufferSize, HashLine);
    strcat_s(Buffer, BufferSize, "\n");
}

internal void
SaveUserSettings()
{
    char Buffer[Kilobytes(64)];
    memset(Buffer, 0, Kilobytes(64));
    
    SaveConfigSettings(Buffer, Kilobytes(64));
    SaveMacroSettings(Buffer, Kilobytes(64));
    
    WriteToFile("olmran.cfg", Buffer);
}