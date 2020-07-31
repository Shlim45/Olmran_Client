
internal void
CycleThroughUserInputHistory(int8 Direction)
{
    if (Direction > 0)
    {
        // cycle up
        GameState.CommandHistory.CurrentPosition++;
    }
    else if (Direction < 0)
    {
        // cycle down
        GameState.CommandHistory.CurrentPosition--;
    }
}