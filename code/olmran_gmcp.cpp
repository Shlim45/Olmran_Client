#define TOTAL_SUPPORTED_GMCP_MESSAGES 29

const char SupportedGMCPMessages[TOTAL_SUPPORTED_GMCP_MESSAGES][25] =
{
    "core_ping",
    "comm.channel",
    "charTimer",
    "charInfo",
    "charVitals",
    "char_status",
    "char_base",
    "switch.base",
    "char_vitals",
    "char_worth",
    "room_info",
    "roomInfo",
    "xpUpdate",
    "allIdols",
    "idolUpdate",
    "char_statusvars",
    "char_status",
    "accountprompt",
    "passwordprompt",
    "loggedin",
    "logout",
    "accountnotfound",
    "maximumplayers",
    "banned",
    "invalidpassword",
    "pendinglogin",
    "loginspam",
    "accountnotrecognized",
    "accountdata"
};

internal void
handleGMCP()
{
    if (GameState.GMCP.BufferIn[0] == 0)
        return;
    
    char *command = strtok_s(GameState.GMCP.BufferIn, " ", &command);
    
    OutputDebugStringA("GMCP Command: ");
    OutputDebugStringA(command);
    OutputDebugStringA("\n");
    // TODO(jon):  iterate over SupportedGMCPMessages to find matching command
}

internal void
handle_gmcp(char *subData, int dataSize)
{
    // check that dataSize > 1 (for handling custom client byte?)
    if (dataSize > 1)
    {
        // create local copy of subdata
        char *data = subData;
        
        // check that string has more than one _
        int count = 0;
        int first = 0;
        for (int i = 0; i < dataSize; i++)
        {
            if (data[i] == 0)
                break;
            if (data[i] == '_')
            {
                if(++count==1)
                    first = i;
            }
        }
        
        // if so, replace first _ with a .
        if (count > 1 && first <= dataSize)
            data[first] = '.';
        
        memset(GameState.GMCP.BufferIn, 0, GameState.GMCP.BufferSize);
        strncpy_s(GameState.GMCP.BufferIn, GameState.GMCP.BufferSize,
                  data, strlen(data));
        
        
        OutputDebugStringA("GMCP In: ");
        OutputDebugStringA(subData);
        OutputDebugStringA("\n");
        
        handleGMCP();
    }
    // zero out Telnet.negBuffers.subNegotiation
    memset(Telnet.negBuffers.subNegotiation, 0, Telnet.negBuffers.subNegSize);
    
    // call GMCPHandlers.handleGMCP(data) - this will update the client etc.
}

internal void 
sendGMCP()
{
    //byte[] input = data.getBytes();
    //byte[] msg = new byte[3 + input.length + 2];
    
    char msg[1024] = {};
    uint32 Pos = 0;
    // TODO(jon):  These values are in telnet.h, need here.
    msg[Pos++]=static_cast<char>(255);
    msg[Pos++]=static_cast<char>(250);
    msg[Pos++]=static_cast<char>(201);
    for (uint32 Index = 0, Size = (uint32) strlen(GameState.GMCP.BufferOut); Index < Size; Index++)
    {
        msg[Pos++] = GameState.GMCP.BufferOut[Index];
    }
    
    msg[Pos++]=static_cast<char>(255);
    msg[Pos++]=static_cast<char>(240);
    
    win32_WriteToSocket(Socket.sock, msg, Pos, 0);
    //msg = addBytes(msg, input);
    //msg = addBytes(msg, new byte[]{IAC, SE});
    //write(msg);
}

internal void
addGMCPSupport()
{
    
    // This method will add support for each GMCP message used
    
    char GMCPBuffer[1024] = { "core_supports_set [" };
    uint32 Pos = 19;
    
    for (uint32 Index = 0; Index < TOTAL_SUPPORTED_GMCP_MESSAGES; Index++)
    {
        //GMCPBuffer[Pos++] = '\\';
        GMCPBuffer[Pos++] = '\"';
        // copy command to buffer, char by char
        for (uint16 i = 0; i < strlen(SupportedGMCPMessages[Index]); i++)
        {
            GMCPBuffer[Pos] = SupportedGMCPMessages[Index][i];
            Pos++;
        }
        //GMCPBuffer[Pos++] = '\\';
        GMCPBuffer[Pos++] = '\"';
        
        if (Index != (TOTAL_SUPPORTED_GMCP_MESSAGES - 1))
            GMCPBuffer[Pos++] = ',';
	}
    
    GMCPBuffer[Pos++] = ']';
    GMCPBuffer[Pos++] = 0;
    
    memset(GameState.GMCP.BufferOut, 0, GameState.GMCP.BufferSize);
    strncpy_s(GameState.GMCP.BufferOut, GameState.GMCP.BufferSize,
              GMCPBuffer, Pos);
    sendGMCP();
}
