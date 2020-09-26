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
    uint16 BufferLength;
    char *Buffer;
};

struct user_input_history
{
    uint8 NumberOfCommands;
    int16 CurrentPosition;
    uint16 BufferSize;
    char *Commands;
    uint16 CurrentSize;
    char *CurrentCommand;
};

struct user_macros
{
    char *MacroBuffer;
};

struct gmcp_buffer
{
    char *BufferIn;
    char *BufferOut;
    uint32 BufferSize;
};

// {"name": "Zuid","class": "Wizard","level": "44","gender": "M","realm": "3"}
struct thin_player
{
    char Name[256];
    char Class[256];
    uint16 Level;
    uint8 Realm;
    char Gender;
};

// TODO(jon):  Cap account characters
struct user_account
{
    thin_player Characters[10];
    uint8 CharCount;
    char LastPlayed[256];
    uint16 Flags;
};

struct portrait_coords
{
    int X;
    int Y;
};

struct player_vitals
{
    uint32 Health;
    uint32 Fat;
    uint32 Power;
    uint32 MaxHealth;
    uint32 MaxFat;
    uint32 MaxPower;
};

struct user_player
{
    char Name[256];
    char Gender[256];
    char Class[256];
    char Race[256];
    char Title[256];
    char Guild[256];
    uint32 ExpTNL;
    uint16 Level;
    uint8 Realm;
    portrait_coords PortraitCoords;
    player_vitals Vitals;
    uint32 ActionTimer;
    bool32 LoggedIn;
};

struct room_info
{
    uint16 Exits;
    // something for shop/bank popups
};

struct user
{
    user_account Account;
    user_player Player;
};

struct midi_device
{
    uint32 DeviceID;
    bool32 IsPlaying;
};

struct game_display_controls
{
    HWND Control;
    HWND Vitals;
    HWND Portrait;
    HWND PlayerInfo;
    HWND Compass;
    HWND ActionTimer;
    HBITMAP Bitmap;
    HBITMAP PortraitBitmap;
    HBITMAP ControlSpritesBitmap;
};

struct sub_windows
{
    HWND Macros;
};

struct macro_group
{
    uint8 NumberOfMacros;
    uint16 BufferSize;
    uint16 MacroSize;
    user_macros Global;
    user_macros Player;
};

struct game_chat
{
    HWND Window;
};

struct game_state
{
    HWND Window;
    bool32 isInitialized;
    game_buffer GameOutput;
    game_buffer GameInput;
    game_chat GameChat;
    user_input_history CommandHistory;
    macro_group Macros;
    gmcp_buffer GMCP;
    COLORREF CurrentColor;
    bool32 AutoSneak;
    user User;
    midi_device *MIDIDevice;
    game_display_controls Display;
    sub_windows SubWindows;
    room_info Room;
};

struct telnet_negotiation_buffer
{
    char *sbbuf;          // current SB negotiation buffer
    char *subNegotiation;
    int32 subnegOffset;
    uint16 sbbufSize;
    uint16 subNegSize;
};

struct telnet_x_buffers // lol naming
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
    telnet_x_buffers xBuffers;
};

global_variable bool GlobalRunning;
global_variable win32_socket Socket;
global_variable game_state GameState;
global_variable telnet_state Telnet;

internal void Win32AppendText(const HWND GameOutput, const char *newText);
internal uint32 Win32WriteToSocket(SOCKET s, char *buf, int bufLen, int flags);
internal void Win32UpdateVitals(HWND Window);
internal void Win32UpdatePlayerInfo(HWND Window);
internal void Win32UpdateCompass(HWND Window);
internal void Win32StartActionTimer();
internal void Win32HandlePlayerLogin();
internal void Win32HandlePlayerLogoff();
internal void Win32EchoCommand(const HWND GameOutput, const char *Command);
internal void LoadPlayerMacros(char *PlayerName);
internal void SavePlayerSettings(char *PlayerName);

#endif //WIN32_OLMRAN_H
