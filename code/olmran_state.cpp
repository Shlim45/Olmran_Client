
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
        int Success = SetWindowTextA(GameState.GameInput.Window,
                                     GameState.CommandHistory.CurrentCommand);
        if (Success)
        {
            memset(GameState.CommandHistory.CurrentCommand, 0, GameState.CommandHistory.CurrentSize);
        }
    }
    
    if (Direction > 0 
        && GameState.CommandHistory.CurrentPosition < (GameState.CommandHistory.NumberOfCommands-1))
    {
        // cycle up
        GameState.CommandHistory.CurrentPosition++;
        
        int Success = SetWindowTextA(GameState.GameInput.Window,
                                     GameState.CommandHistory.Commands);
        if (Success)
        {
            //memset(GameState.CommandHistory.CurrentCommand, 0, GameState.CommandHistory.CurrentSize);
        }
    }
    else if (Direction < 0 && GameState.CommandHistory.CurrentPosition > -1)
    {
        // cycle down
        GameState.CommandHistory.CurrentPosition--;
    }
}

internal void
UpdateCommandHistory()
{
    // NOTE(jon):  This clears the entire command history, which is currently only 1.
    memset(GameState.CommandHistory.Commands, 0, GameState.CommandHistory.BufferSize * GameState.CommandHistory.NumberOfCommands);
    
    // This copies the CurrentCommand buffer into the Commands[0] buffer.
    strcpy_s(GameState.CommandHistory.Commands, GameState.CommandHistory.BufferSize,
             GameState.CommandHistory.CurrentCommand);
    
    memset(GameState.CommandHistory.CurrentCommand, 0, GameState.CommandHistory.CurrentSize);
    
    GameState.CommandHistory.CurrentPosition = -1;
}