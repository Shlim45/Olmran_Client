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

struct game_state
{
    HWND Window;
    bool32 isInitialized;
    game_buffer GameOutput;
    game_buffer GameInput;
    COLORREF CurrentColor;
    bool32 AutoSneak;
};

struct telnet_negotiation_buffer
{
    char *sbbuf;          // current SB negotiation buffer
    char *subNegotiation;
    int32 subnegOffset;
    uint16 sbbufSize;
    uint16 subNegSize;
};

struct telnet_X_buffers // lol naming
{
    char *receivedDX; // What IAC DO(NT) request do we have already received?
    char *receivedWX; // What IAC WILL/WONT ...
    char *sentDX;     // What IAC DO(NOT) request do we have sent already?
    char *sentWX;     // What IAC WILL/WONT ...
    uint16 bufferSize;// Size for ALL buffers
};

struct telnet_state
{
    char currentSb;           // What IAC SB <xx> are we handling right now?
    char negState;            // Current negotiation state
    int32 outOffset;
    telnet_negotiation_buffer negBuffers; // name?
    telnet_X_buffers xBuffers;
};

global_variable bool GlobalRunning;
global_variable win32_socket Socket;
global_variable game_state GameState;
global_variable telnet_state Telnet;

internal void
win32_AppendText(const HWND GameOutput, const char *newText);

internal uint32
win32_WriteToSocket(SOCKET s, char *buf, int bufLen, int flags);
#endif //WIN32_OLMRAN_H
