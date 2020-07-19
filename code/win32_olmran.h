/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jonathan Hawranko $
   $Notice: (C) Copyright 2020 by Jonathan Hawranko. All Rights Reserved. $
   ======================================================================== */
#ifndef WIN32_OLMRAN_H
#define WIN32_OLMRAN_H

#include <ws2tcpip.h>
#include <windows.h>
#include <stdint.h>

#define ID_EDITCHILD 100
#define HOST_ADDRESS "192.168.1.208"
#define HOST_PORT 4000

#define global_variable static
#define local_persist static
#define internal static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

struct win32_socket
{
    SOCKET sock;
    SOCKADDR_IN addr;
    int status;
};

//bool win32_InitAndConnectSocket(HWND GameOutput);
//void win32_CloseSocket();

global_variable bool GlobalRunning;
global_variable win32_socket Socket;

#endif //WIN32_OLMRAN_H
