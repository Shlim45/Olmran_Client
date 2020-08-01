
internal void
CycleThroughUserInputHistory(int8 Direction)
{
    if (GameState.CommandHistory.CurrentPosition < 0 && Direction > 0)
    {
        memset(GameState.CommandHistory.CurrentCommand, 0, GameState.CommandHistory.CurrentSize);
        
        GetWindowTextA(GameState.GameInput.Window,
                       GameState.CommandHistory.CurrentCommand,
                       GameState.CommandHistory.CurrentSize);
        
    }
    else if (GameState.CommandHistory.CurrentPosition == 0 && Direction < 0)
    {
        bool32 Success = SetWindowTextA(GameState.GameInput.Window,
                                        GameState.CommandHistory.CurrentCommand);
        if (Success)
        {
            memset(GameState.CommandHistory.CurrentCommand, 0, GameState.CommandHistory.CurrentSize);
            
            GameState.CommandHistory.CurrentPosition--;
            
        }
    }
    
    if (Direction > 0 
        && GameState.CommandHistory.CurrentPosition < (GameState.CommandHistory.NumberOfCommands-1))
    {
        // cycle up
        GameState.CommandHistory.CurrentPosition++;
        
        SetWindowTextA(GameState.GameInput.Window,
                       GameState.CommandHistory.Commands + (GameState.CommandHistory.CurrentPosition * GameState.CommandHistory.BufferSize));
    }
    else if (Direction < 0 && GameState.CommandHistory.CurrentPosition > -1)
    {
        // cycle down
        GameState.CommandHistory.CurrentPosition--;
        
        SetWindowTextA(GameState.GameInput.Window,
                       GameState.CommandHistory.Commands + (GameState.CommandHistory.CurrentPosition * GameState.CommandHistory.BufferSize));
    }
    
    // Set caret to end
    int index = GetWindowTextLengthA( GameState.GameInput.Window );
    SendMessage( GameState.GameInput.Window, EM_SETSEL, (WPARAM)index, (LPARAM)index );
}

internal void
UpdateCommandHistory()
{
    for (int Slot = GameState.CommandHistory.NumberOfCommands - 1;
         Slot >= 0;
         Slot--)
    {
        // clear this buffer slot
        memset(GameState.CommandHistory.Commands + (GameState.CommandHistory.BufferSize * Slot),
               0, GameState.CommandHistory.BufferSize);
        
        // copy previous slot to this slot
        strcpy_s(GameState.CommandHistory.Commands + (GameState.CommandHistory.BufferSize * Slot),
                 GameState.CommandHistory.BufferSize,
                 GameState.CommandHistory.Commands + (GameState.CommandHistory.BufferSize * (Slot-1)));
        
        if (Slot == 0)
        {
            // This copies the CurrentCommand buffer into the Commands[0] buffer.
            strcpy_s(GameState.CommandHistory.Commands, GameState.CommandHistory.BufferSize,
                     GameState.CommandHistory.CurrentCommand);
            
            // Clear CurrentCommand buffer
            memset(GameState.CommandHistory.CurrentCommand, 0, GameState.CommandHistory.CurrentSize);
            
            // Reset position
            GameState.CommandHistory.CurrentPosition = -1;
        }
    }
}