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

struct game_buffer
{
    HWND Window;
    char *Buffer;
    uint16 BufferLength;
};

struct game_text_color
{
    COLORREF DefaultColor;
    COLORREF CurrentColor;
};

struct game_state
{
    HWND Window;
    bool32 isInitialized;
    game_buffer GameOutput;
    game_buffer GameInput;
    game_text_color Colors;
};

global_variable bool GlobalRunning;
global_variable win32_socket Socket;
global_variable game_state GameState;

internal void
win32_AppendText(const HWND GameOutput, const char *newText);

#endif //WIN32_OLMRAN_H
