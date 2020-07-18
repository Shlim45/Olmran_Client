/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jonathan Hawranko $
   $Notice: (C) Copyright 2020 by Jonathan Hawranko. All Rights Reserved. $
   ======================================================================== */

#ifndef SOCKET_CONNECTION_H
#define SOCKET_CONNECTION_H

#include <ws2tcpip.h>

#define HOST_ADDRESS "192.168.1.208"
#define HOST_PORT 4000

class SocketConnection
{
    public:
    SocketConnection(),
    m_socket(INVALID_SOCKET),
    m_hint(SOCKET_ERROR)
    {
        WSAData data;
        WORD ver = MAKEWORD(2,2);
        int wsResult = WSAStartup(ver, &data);
        
        if (wsResult != 0)
        {
            // TODO(jon): Error starting WinSock, Err# = wsResult
        }
        else
        {
            Initialize();
        }
    }
    ~SocketConnection();
    
    void Initialize()
    {
        CreateSocket();
        
        // Fill in hint structure
        m_hint.sin_family = AF_INET;
        m_hint.sin_port = htons(HOST_PORT);
        inet_pton(AF_INET, HOST_ADDRESS, &m_hint.sin_addr);
        
        ConnectToServer();
    }
    
    private:
    SOCKET m_socket;
    sockaddr_in m_hint;
    
    bool CreateSocket()
    {
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        return (m_socket != INVALID_SOCKET);
    }
    
    bool ConnectToServer()
    {
        int connResult = connect(m_socket, (sock_addr*)&m_hint);
        if (connResult == SOCKET_ERROR)
        {
            closesocket(m_socket);
            WSACleanup();
            return false;
        }
    }
    
#endif