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

#define ID_EDITCHILD 100
#define HOST_ADDRESS "192.168.1.208"
#define HOST_PORT 4000

#define global_variable static
#define local_persist static
#define internal static

struct win32_socket
{
    SOCKET sock;
    SOCKADDR_IN addr;
    int status;
};

bool win32_InitAndConnectSocket(HWND GameOutput);
void win32_CloseSocket();

global_variable bool running;
global_variable win32_socket Socket;

#endif //WIN32_OLMRAN_H
