
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
            // send command thru socket
            win32_WriteStringToSocket(Socket.sock, GameState.GameInput, Command);
            
            // clear Command buffer
            memset(Command, 0, 256);
            
            // reset CommandIndex
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