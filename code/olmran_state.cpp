
internal void
SetCaretToEnd(HWND TextBoxHandle)
{
    int index = GetWindowTextLengthA( TextBoxHandle );
    SendMessage( TextBoxHandle, EM_SETSEL, (WPARAM)index, (LPARAM)index );
}

// TODO(jon):  Turn this in to a generic HWND text to buffer function
internal void
StoreCurrentCommand()
{
    memset(GameState.CommandHistory.CurrentCommand, 0, GameState.CommandHistory.CurrentSize);
    
    GetWindowTextA(GameState.GameInput.Window,
                   GameState.CommandHistory.CurrentCommand,
                   GameState.CommandHistory.CurrentSize);
}

internal bool32
RestorePreviousCommand()
{
    bool32 Success = SetWindowTextA(GameState.GameInput.Window,
                                    GameState.CommandHistory.CurrentCommand);
    if (Success)
        memset(GameState.CommandHistory.CurrentCommand, 0, GameState.CommandHistory.CurrentSize);
    
    return Success;
}

internal void
CycleThroughUserInputHistory(int8 Direction)
{
    int16 *pos = &GameState.CommandHistory.CurrentPosition;
    
    if (*pos < 0 && Direction > 0)
        StoreCurrentCommand();
    else if (*pos == 0 && Direction < 0)
    {
        bool32 Success = RestorePreviousCommand();
        if (Success)
            --*pos;
    }
    
    // NOTE(jon):  Cycling upward, thru spots 0-MAX
    if (Direction > 0 
        && *pos < (GameState.CommandHistory.NumberOfCommands-1))
    {
        // cycle up
        char *NewText = GameState.CommandHistory.Commands + ((*pos+1) * GameState.CommandHistory.BufferSize);
        
        if (NewText[0] != 0)
        {
            ++*pos;
            
            SetWindowTextA(GameState.GameInput.Window,
                           GameState.CommandHistory.Commands + (*pos * GameState.CommandHistory.BufferSize));
        }
    }
    // NOTE(jon):  Cycling downward, thru spots MAX-0
    else if (Direction < 0 && *pos > -1)
    {
        // cycle down
        --*pos;
        
        SetWindowTextA(GameState.GameInput.Window,
                       GameState.CommandHistory.Commands + (*pos * GameState.CommandHistory.BufferSize));
    }
    
    SetCaretToEnd( GameState.GameInput.Window );
}

internal void
UpdateCommandHistory()
{
    uint16 BufferSize = GameState.CommandHistory.BufferSize;
    for (int Slot = GameState.CommandHistory.NumberOfCommands - 1;
         Slot >= 0;
         Slot--)
    {
        // clear this buffer slot
        memset(GameState.CommandHistory.Commands + (BufferSize * Slot),
               0, BufferSize);
        
        if (Slot)
        {
            // copy previous slot to this slot
            strcpy_s(GameState.CommandHistory.Commands + (BufferSize * Slot),
                     BufferSize,
                     GameState.CommandHistory.Commands + (BufferSize * (Slot-1)));
        }
        else
        {
            // This copies the CurrentCommand buffer into the Commands[0] buffer.
            strcpy_s(GameState.CommandHistory.Commands, BufferSize,
                     GameState.CommandHistory.CurrentCommand);
            
            // Clear CurrentCommand buffer
            memset(GameState.CommandHistory.CurrentCommand, 0, GameState.CommandHistory.CurrentSize);
            
            // Reset position
            GameState.CommandHistory.CurrentPosition = -1;
        }
    }
}

// NOTE(jon):  Takes the text from the text controls on SubWindows.Macros 
//             and updates GameState.GlobalMacros.Macros
internal void
UpdateGlobalMacros(HWND MacroWindow)
{
    const int MACRO_SIZE = 256;
    char Macro[MACRO_SIZE];
    memset(Macro, 0, MACRO_SIZE);
    
    memset(GameState.Macros.Global.MacroBuffer, 0, GameState.Macros.BufferSize);
    
    // iterate over each MacroID and update GameState
    for (int Index = 0; Index < MAX_MACROS; Index++)
    {
        GetWindowTextA(GetDlgItem(MacroWindow,MacroIDs[Index]), Macro, MACRO_SIZE);
        strcpy_s(GameState.Macros.Global.MacroBuffer + (Index * MACRO_SIZE), MACRO_SIZE, Macro);
    }
}
