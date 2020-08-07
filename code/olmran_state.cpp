#if 0
internal void
InitializeCommandHistory(game_state gState, char *buffer)
{
    gState.CommandHistory.Commands = buffer;
    gState.CommandHistory.BufferSize = 512;
    gState.CommandHistory.NumberOfCommands = 10;
    gState.CommandHistory.CurrentPosition = -1;
}
#endif

internal void
SetCaretToEnd(HWND TextBoxHandle)
{
    // NOTE(jon):  When finished, set caret to end of GameInput textbox.
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
        
        // copy previous slot to this slot
        strcpy_s(GameState.CommandHistory.Commands + (BufferSize * Slot),
                 BufferSize,
                 GameState.CommandHistory.Commands + (BufferSize * (Slot-1)));
        
        if (Slot == 0)
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