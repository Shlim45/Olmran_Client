/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jonathan Hawranko $
   $Notice: (C) Copyright 2020 by Jonathan Hawranko. All Rights Reserved. $
   ======================================================================== */
#ifndef WIN32_OLMRAN_H
#define WIN32_OLMRAN_H

struct win32_window_dimension
{
	int Width;
	int Height;
};

struct win32_socket
{
    SOCKET sock;
    SOCKADDR_IN addr;
    int status;
};

struct win32_state
{
	uint64 TotalSize;
	void *GameMemoryBlock;
};

struct game_state
{
    HWND Window;
    HWND GameOutput;
    HWND GameInput;
    bool32 isInitialized;
    char *GameOutputBuffer;
    uint16 GameOutputBufferLength;
    char *GameInputBuffer;
    uint16 GameInputBufferLength;
};

global_variable bool GlobalRunning;
global_variable win32_socket Socket;
global_variable game_state GameState;

internal void
win32_AppendText(const HWND GameOutput, const char *newText);

#endif //WIN32_OLMRAN_H
