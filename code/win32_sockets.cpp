
internal uint32
win32_WriteToSocket(SOCKET s, char *buf, int bufLen, int flags)
{
    uint32 iResult;
    iResult = send( s, buf, bufLen, flags );
    if (iResult == SOCKET_ERROR) {
        OutputDebugStringA("send failed with error\n");
        closesocket(s);
        WSACleanup();
        return 1;
    }
    return iResult;
}

internal uint32
win32_SendInputThroughSocket(SOCKET s, game_state gState)
{
    char inputBuf[512] = {0};
    int inputLength = GetWindowTextA(                           // returns length (not including \0)
                                     gState.GameInput,       // A handle to the edit control.
                                     (LPSTR) inputBuf,          // A pointer to the buffer that will receive the text.
                                     gState.GameInputBufferLength// The maximum number of characters to copy to the buffer, including the NULL terminator.
                                     );
    SetWindowTextA(gState.GameInput, "");
    inputBuf[inputLength] = '\n';
    inputLength = (int) strlen(inputBuf);
    
    uint32 iResult;
    iResult = win32_WriteToSocket( s, inputBuf, (int)strlen(inputBuf), 0 );
    return iResult;
}


internal void 
win32_CloseSocket()
{
    closesocket(Socket.sock);
    WSACleanup();
    Socket.status = 0;
}

internal int64 
win32_InitAndConnectSocket()
{
    // Initialize WinSock
    WSAData data;
    WORD ver = MAKEWORD(2,2);
    int WSResult = WSAStartup(ver, &data);
    
    if (WSResult != 0)
    {
        OutputDebugStringA("Can't start WinSock, Err #");
        return WSResult;
    }
    
    // Create Socket
    Socket.sock = socket(AF_INET, SOCK_STREAM, 0);
    if (Socket.sock == INVALID_SOCKET)
    {
        OutputDebugStringA("Can't create socket, Err #");
        return INVALID_SOCKET;
    }
    
    // Fill in hint structure
    Socket.addr.sin_family = AF_INET;
    Socket.addr.sin_port = htons(HOST_PORT);
    inet_pton(AF_INET, HOST_ADDRESS, &Socket.addr.sin_addr);
    
    // Connect to server
    if (connect(Socket.sock, (sockaddr*) &Socket.addr, sizeof(Socket.addr)) == SOCKET_ERROR)
    {
        OutputDebugStringA("Can't connect to server, Err #");
        
        win32_CloseSocket();
        return SOCKET_ERROR;
    }
    
    Socket.status = 1;
    return 0;
}


internal void
ProcessInputFromSocket(char *recvBuf)
{
    local_persist TCHAR tmpBuf[4096];
    memset(tmpBuf,0,4096);
    uint16 recvIndex = 0;
    uint16 tmpIndex = 0;
    if (strlen(recvBuf) == 0)
        return;
    while (recvBuf[recvIndex] != 0)
    {
        // NOTE(jon):  Filtering telnet commands
        
        if (recvBuf[recvIndex] >= ' ' && recvBuf[recvIndex] <= '~')
        {
            // valid char
            tmpBuf[tmpIndex] = recvBuf[recvIndex];
            tmpIndex++;
        }
        else if (recvBuf[recvIndex] >= 9 && recvBuf[recvIndex] <= 13)
        {
            // valid formatting
            if (recvBuf[recvIndex] != 13) // prevent double spacing (convert from CRLF to LF)
            {
                tmpBuf[tmpIndex] = recvBuf[recvIndex];
                tmpIndex++;
            }
        }
        else if (recvBuf[recvIndex] == '\u001B')
        {
            // valid Escape sequence
            tmpBuf[tmpIndex] = recvBuf[recvIndex];
            tmpIndex++;
        }
        else
        {
            switch (recvBuf[recvIndex])
            {
                case TN_GA:
                { OutputDebugStringA("GA\n\r"); } break;
                case TN_NOP:
                { OutputDebugStringA("NOP\n\r"); } break;
                case TN_SE:
                { OutputDebugStringA("SE\n\r"); } break;
                case TN_EOR:
                { OutputDebugStringA("EOR\n\r"); } break;
                case TN_IAC:
                { OutputDebugStringA("IAC "); } break;
                case TN_WILL:
                { OutputDebugStringA("WILL "); } break;
                case TN_WONT:
                { OutputDebugStringA("WONT "); } break;
                case TN_DO:
                { OutputDebugStringA("DO "); } break;
                case TN_DONT:
                { OutputDebugStringA("DONT "); } break;
                case OPT_ECHO:
                { OutputDebugStringA("ECHO\n\r"); } break;
                case OPT_STATUS:
                { OutputDebugStringA("STATUS\n\r"); } break;
                case OPT_TIMING_MARK:
                { OutputDebugStringA("TIMING_MARK\n\r"); } break;
                case OPT_TERMINAL_TYPE:
                { OutputDebugStringA("TERMINAL_TYPE\n\r"); } break;
                case OPT_EOR:
                { OutputDebugStringA("EOR\n\r"); } break;
                case OPT_NAWS:
                { OutputDebugStringA("NAWS\n\r"); } break;
                case OPT_ATCP:
                { OutputDebugStringA("ATCP\n\r"); } break;
                case OPT_GMCP:
                { OutputDebugStringA("GMCP\n\r"); } break;
                default:
                OutputDebugStringA("\nDEFAULT: " + (char)recvBuf[recvIndex]);
            }
        }
        recvIndex++;
    }
    // clear recvBuf and fill it with 
    memset(recvBuf, 0, 4096);
    
    // TODO(jon):  Remove this when streamin is fixed.
    if( strlen(tmpBuf)>0 )
    {
        ANSITest(tmpBuf);
        //win32_AppendText( GameState.GameOutput, recvBuf );
    }
}

DWORD WINAPI 
SocketListenThreadProc(LPVOID lpParameter)
{
    if (GameState.isInitialized)
    {
        int iResult;
        local_persist char recvbuf[4096];
        memset(recvbuf, 0, 4096);
        
        // Receive until the peer closes the connection
        do {
            iResult = recv(Socket.sock, recvbuf, 4096, 0);
            if ( iResult > 0 )
            {
                OutputDebugStringA("Bytes received: >0\n");//, iResult);
                if (strlen(recvbuf) > 0)
                {
                    ProcessInputFromSocket(recvbuf);
#if 0
                    win32_StreamToGameOutput(recvbuf, 4096);
#endif
                }
            }
            else if ( iResult == 0 )
                OutputDebugStringA("Connection closed\n");
            else
            {
                int error = WSAGetLastError();
                OutputDebugStringA("recv failed: ?\n");//, WSAGetLastError());
            }
            
        } while( iResult > 0 );
        
        // cleanup
        closesocket(Socket.sock);
        WSACleanup();
    }
    return 0;
}