internal uint32
Win32WriteToSocket(SOCKET s, char *buf, int bufLen, int flags)
{
    uint32 iResult;
    iResult = send( s, buf, bufLen, flags );
    if (iResult == SOCKET_ERROR) {
        OutputDebugStringA("send failed with error\n");
        closesocket(s);
        WSACleanup();
        GlobalRunning = false;
        return 1;
    }
    return iResult;
}

internal uint32
Win32SendInputThroughSocket(SOCKET s, game_state gState)
{
    int inputLength = GetWindowTextA(                                   // returns length (not including \0)
                                     gState.GameInput.Window,                  // A handle to the edit control.
                                     (LPSTR) gState.GameInput.Buffer, // buffer to receive text.
                                     gState.GameInput.BufferLength       // max number of chars to copy, including the NULL terminator.
                                     );
    
    if (GameState.User.Account.Flags & (FLAG_LOGGEDIN) &&
        GameState.User.Account.Flags & (FLAG_ECHO))
        Win32EchoCommand(GameState.GameOutput.Window, gState.GameInput.Buffer);
    
    if (!(GameState.User.Account.Flags & FLAG_PERSIST))
        SetWindowTextA(gState.GameInput.Window, "");
    else
        SendMessageA(gState.GameInput.Window,EM_SETSEL,0,-1);
    
    if (inputLength < gState.GameInput.BufferLength - 1)
    {
        gState.GameInput.Buffer[inputLength] = '\n';
        ++inputLength;
    }
    
    uint32 iResult;
    iResult = Win32WriteToSocket( s, gState.GameInput.Buffer, inputLength, 0 );
    
    memset(gState.GameInput.Buffer, 0, gState.GameInput.BufferLength);
    
    return iResult;
}

internal uint32
Win32WriteStringToSocket(SOCKET s, game_buffer GameInput, char *str)
{
    uint32 iResult;
    uint16 inputLength = (uint16) strlen(str);
    
    strcpy_s(GameInput.Buffer, GameInput.BufferLength, str);
    
    if (inputLength < GameInput.BufferLength - 1)
    {
        GameInput.Buffer[inputLength] = '\n';
        ++inputLength;
    }
    
    iResult = Win32WriteToSocket( s, GameInput.Buffer, inputLength, 0 );
    
    memset(GameInput.Buffer, 0, GameInput.BufferLength);
    
    return iResult;
}

internal void 
Win32CloseSocket()
{
    closesocket(Socket.sock);
    WSACleanup();
    Socket.status = 0;
}

internal int64 
Win32InitAndConnectSocket()
{
    // Initialize WinSock
    WSAData data;
    WORD ver = MAKEWORD(2,2);
    int WSResult = WSAStartup(ver, &data);
    
    if (WSResult != 0)
    {
        OutputDebugStringA("Can't start WinSock, Err #\n");
        return WSResult;
    }
    
    // Create Socket
    Socket.sock = socket(AF_INET, SOCK_STREAM, 0);
    if (Socket.sock == INVALID_SOCKET)
    {
        OutputDebugStringA("Can't create socket, Err #\n");
        return INVALID_SOCKET;
    }
    
    // Fill in hint structure
    Socket.addr.sin_family = AF_INET;
    Socket.addr.sin_port = htons(HOST_PORT);
    inet_pton(AF_INET, HOST_ADDRESS, &Socket.addr.sin_addr);
    
    // Connect to server
    if (connect(Socket.sock, (sockaddr*) &Socket.addr, sizeof(Socket.addr)) == SOCKET_ERROR)
    {
        OutputDebugStringA("Can't connect to server, Err #\n");
        
        Win32CloseSocket();
        return SOCKET_ERROR;
    }
    
    Socket.status = 1;
    return 0;
}

internal void
ProcessInputFromSocket(char *recvBuf)
{
    // TODO(jon):  Too many strlen calls.
    if (strlen(recvBuf) == 0)
        return;
    
    // TODO(jon):  Tie this into GameMemory
    const int tmpBufSize = Kilobytes(4);
    local_persist TCHAR tmpBuf[tmpBufSize];
    memset(tmpBuf,0,tmpBufSize);
    
    TelnetNegotiation(Telnet, tmpBuf, recvBuf, (uint32) strlen(recvBuf));
    memset(recvBuf, 0, tmpBufSize);
    
    if( strlen(tmpBuf)>0 )
    {
        ParseBufferForANSI(tmpBuf);
    }
}

DWORD WINAPI 
SocketListenThreadProc(LPVOID lpParameter)
{
    if (GameState.isInitialized)
    {
        int iResult;
        
        local_persist char recvbuf[4096];
        game_buffer OutputBuffer = {};
        OutputBuffer.Window = GameState.GameOutput.Window;
        OutputBuffer.BufferLength = 4096;
        OutputBuffer.Buffer = recvbuf;
        
        GameState.GameOutput = OutputBuffer;
        
        // Receive until the peer closes the connection
        do {
            iResult = recv(Socket.sock, GameState.GameOutput.Buffer, GameState.GameOutput.BufferLength, 0);
            if ( iResult > 0 )
            {
                if (strlen(GameState.GameOutput.Buffer) > 0)
                {
                    ProcessInputFromSocket(GameState.GameOutput.Buffer);
                }
            }
            else if ( iResult == 0 )
                OutputDebugStringA("Connection closed\n");
            else
            {
                int error = WSAGetLastError();
                OutputDebugStringA("recv failed: Closed by user?\n");//, WSAGetLastError());
            }
            
        } while( iResult > 0 );
        
        // cleanup
        closesocket(Socket.sock);
        WSACleanup();
        
        // NOTE(jon):  This will close the window when socket disconnects
        GlobalRunning = false;
    }
    
    return 0;
}

internal bool32
InitializeSocketConnection(game_state *State, HANDLE *SocketListenThreadHandle, LPDWORD ThreadID)
{
    bool32 Result = false;
    
    if (Win32InitAndConnectSocket()==0)
    {
        OutputDebugStringA("Socket Connected\n");
        TelnetInit(Telnet);
        
        char *Param = "Socket listening.\n";
        
        *SocketListenThreadHandle = 
            CreateThread(0, 0, SocketListenThreadProc, Param, 0, ThreadID);
        
        Result = true;
    }
    else
    {
        Win32AppendText(State->GameOutput.Window, "Could not connect to server.\n");
        OutputDebugStringA("Error in Win32InitAndConnectSocket()\n");
        SocketListenThreadHandle = 0;
    }
    
    return Result;
}
