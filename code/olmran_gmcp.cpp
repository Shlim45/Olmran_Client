/* In every .c file that uses jsmn include only declarations: */
//#define JSMN_HEADER
#include "jsmn.h"

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
    memset(GameState.GMCP.BufferOut, 0, GameState.GMCP.BufferSize);
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
    
    //memset(GameState.GMCP.BufferOut, 0, GameState.GMCP.BufferSize);
    strncpy_s(GameState.GMCP.BufferOut, GameState.GMCP.BufferSize,
              GMCPBuffer, Pos);
    sendGMCP();
}

internal int 
jsoneq(const char *json, jsmntok_t *tok, const char *s) 
{
    if (tok->type == JSMN_STRING
        && (int)strlen(s) == tok->end - tok->start
        && strncmp(json + tok->start, s, tok->end - tok->start) == 0)
    {
        return 0;
    }
    return -1;
}

internal void
handleGMCP()
{
    if (GameState.GMCP.BufferIn[0] == 0)
        return;
    
    char *jsonObject = {};
    const char *command = strtok_s(GameState.GMCP.BufferIn, " ", &jsonObject);
    
    OutputDebugStringA("GMCP Command: ");
    OutputDebugStringA(command);
    OutputDebugStringA("\n");
    // TODO(jon):  iterate over SupportedGMCPMessages to find matching command
    
    jsmn_parser parser = {};
    jsmntok_t tokens[128] = {};
    
    if (strcmp("charinfo", command) == 0)
    {
        strncpy_s(GameState.GMCP.BufferOut, GameState.GMCP.BufferSize,
                  "char_status", 11);
        sendGMCP();
        
        strncpy_s(GameState.GMCP.BufferOut, GameState.GMCP.BufferSize,
                  "char_base", 9);
        sendGMCP();
        
        strncpy_s(GameState.GMCP.BufferOut, GameState.GMCP.BufferSize,
                  "char_vitals", 11);
        sendGMCP();
        
        strncpy_s(GameState.GMCP.BufferOut, GameState.GMCP.BufferSize,
                  "room_info", 9);
        sendGMCP();
    }
    else if (strcmp("loggedin", command) == 0)
    {
        GameState.User.Account.LoggedIn = true;
    }
    else if (strcmp("logout", command) == 0)
    {
        memset(&GameState.User.Player, 0, sizeof(GameState.User.Player));
        GameState.User.Account.LoggedIn = false;
        win32_UpdateClientTitle();
        win32_StopMIDIPlayback(GameState.MIDIDevice);
    }
    else if (strcmp("accountdata", command) == 0)
    {
        jsmn_init(&parser);
        int Result = jsmn_parse(&parser, jsonObject, strlen(jsonObject), tokens, 128);
        
        if (Result > 0)
        {
            // handle tokens
            if (tokens[0].type == JSMN_OBJECT)
            {
                char valueBuff[256] = {};
                char arrayBuff[1024] = {};
                uint32 valueInt = 0;
                
                for (int Index = 1, Size = Result; Index < Size; Index++)
                {
                    if (tokens[Index].type != JSMN_STRING)
                    {
                        OutputDebugStringA("JSON Error:  Key not a string\n");
                        continue;
                    }
                    
                    int start = tokens[Index+1].start;
                    int size = tokens[Index+1].end - start;
                    
                    if (jsoneq(jsonObject, &tokens[Index], "characters") == 0) 
                    {
                        memcpy( arrayBuff, &jsonObject[start], size );
                        
                        // use the value
                        OutputDebugStringA("Characters: ");
                        OutputDebugStringA(arrayBuff);
                        Index++;
                        
                        // TODO(jon):  Loop over the array here
                        // the array is of characterCount number of jsonObjects
                    }
                    else if (jsoneq(jsonObject, &tokens[Index], "characterCount") == 0) 
                    {
                        memcpy( valueBuff, &jsonObject[start], size );
                        
                        valueInt = atoi(valueBuff);
                        GameState.User.Account.CharCount = (uint8) valueInt;
                        Index++;
                    }
                    else if (jsoneq(jsonObject, &tokens[Index], "lastCharacter") == 0) 
                    {
                        memcpy( valueBuff, &jsonObject[start], size );
                        memcpy( GameState.User.Account.LastPlayed, valueBuff, size );
                        
                        // use the value
                        OutputDebugStringA("LastPlayed: ");
                        Index++;
                    }
                    else
                    {
                        OutputDebugStringA("Unknown key.\n");
                    }
                    
                    // print the value for now
                    OutputDebugStringA(valueBuff);
                    OutputDebugStringA("\n");
                    
                    // clear the value
                    memset(valueBuff, 0, 256);
                    valueInt = 0;
                }
            }
        }
    }
    else if (strcmp("char.base", command) == 0)
    {
        jsmn_init(&parser);
        
        // NOTE(jon):  Returns the total number of tokens in the jsonObject.
        // The entire JSON object itself counts as 1 token, with tokens 1-n
        // being the inner parts.
        int Result = jsmn_parse(&parser, jsonObject, strlen(jsonObject), tokens, 128);
        
        if (Result > 0)
        {
            // handle tokens
            if (tokens[0].type == JSMN_OBJECT)
            {
                char valueBuff[256] = {};
                uint32 valueInt = 0;
                
                for (int Index = 1, Size = Result; Index < Size; Index+=2)
                {
                    if (tokens[Index].type != JSMN_STRING)
                    {
                        OutputDebugStringA("JSON Error:  Key not a string\n");
                        continue;
                    }
                    
                    int start = tokens[Index+1].start;
                    int size = tokens[Index+1].end - start;
                    
                    if (jsoneq(jsonObject, &tokens[Index], "name") == 0) 
                    {
                        memcpy( valueBuff, &jsonObject[start], size );
                        memcpy( GameState.User.Player.Name, valueBuff, size );
                        
                        GameState.User.Account.LoggedIn = true;
                        win32_UpdateClientTitle();
                        //win32_UpdateClient();
                        
                        // use the value
                        OutputDebugStringA("Name:      ");
                    }
                    else if (jsoneq(jsonObject, &tokens[Index], "gender") == 0) 
                    {
                        memcpy( valueBuff, &jsonObject[start], size );
                        memcpy( GameState.User.Player.Gender, valueBuff, size );
                        
                        // use the value
                        OutputDebugStringA("Gender:    ");
                    }
                    else if (jsoneq(jsonObject, &tokens[Index], "class") == 0) 
                    {
                        continue;
#if 0
                        memcpy( valueBuff, &jsonObject[start], size );
                        // NOTE(jon):  This is the base class, not really used.
                        
                        // use the value
                        OutputDebugStringA("Class:     ");
#endif
                    }
                    else if (jsoneq(jsonObject, &tokens[Index], "subclass") == 0) 
                    {
                        memcpy( valueBuff, &jsonObject[start], size );
                        memcpy( GameState.User.Player.Class, valueBuff, size );
                        
                        // use the value
                        OutputDebugStringA("Subclass:  ");
                    }
                    else if (jsoneq(jsonObject, &tokens[Index], "race") == 0) 
                    {
                        memcpy( valueBuff, &jsonObject[start], size );
                        memcpy( GameState.User.Player.Race, valueBuff, size );
                        
                        // use the value
                        OutputDebugStringA("Race:      ");
                    }
                    else if (jsoneq(jsonObject, &tokens[Index], "realm") == 0) 
                    {
                        memcpy( valueBuff, &jsonObject[start], size );
                        
                        valueInt = atoi(valueBuff);
                        GameState.User.Player.Realm = (uint8) valueInt;
                        // use the value
                        OutputDebugStringA("Realm:     ");
                    }
                    else if (jsoneq(jsonObject, &tokens[Index], "perlevel") == 0) 
                    {
                        continue;
#if 0
                        memcpy( valueBuff, &jsonObject[start], size );
                        // NOTE(jon):  this value is the total exp the player
                        // needs to lvl, which isnt used at the moment
                        valueInt = atoi(valueBuff);
                        GameState.User.Player.ExpTotalTNL = valueInt;
                        // use the value
                        OutputDebugStringA("Perlevel:  ");
#endif
                    }
                    else if (jsoneq(jsonObject, &tokens[Index], "pretitle") == 0) 
                    {
                        // add 3 to skip the "*, "
                        memcpy( valueBuff, &jsonObject[start+3], size-3 );
                        memcpy( GameState.User.Player.Title, valueBuff, size );
                        
                        // use the value
                        OutputDebugStringA("Pretitle:  ");
                    }
                    else if (jsoneq(jsonObject, &tokens[Index], "clan") == 0) 
                    {
                        memcpy( valueBuff, &jsonObject[start], size );
                        memcpy( GameState.User.Player.Guild, valueBuff, size );
                        
                        // use the value
                        OutputDebugStringA("Clan:      ");
                        
                    }
                    else
                    {
                        OutputDebugStringA("Unknown key.\n");
                    }
                    
                    // print the value for now
                    OutputDebugStringA(valueBuff);
                    OutputDebugStringA("\n");
                    
                    // clear the value
                    memset(valueBuff, 0, 256);
                    valueInt = 0;
                }
            }
        }
        else if (Result == JSMN_ERROR_INVAL)
        {
            OutputDebugStringA("bad token, JSON string is corrupted\n");
        }
        else if (Result == JSMN_ERROR_NOMEM)
        {
            // If you get JSMN_ERROR_NOMEM, you can re-allocate more tokens and call jsmn_parse once more.
            OutputDebugStringA("not enough tokens, JSON string is too large\n");
        }
        else if (Result == JSMN_ERROR_PART)
        {
            /*
            If you read json data from the stream, you can periodically call jsmn_parse and check if 
                return value is JSMN_ERROR_PART. You will get this error until you reach the end of JSON data.
                */
            OutputDebugStringA("JSON string is too short, expecting more JSON data\n");
        }
        else
        {
            OutputDebugStringA("Failed to parse JSON\n");
        }
        
        // TODO(jon):  Start this elsewhere?  Need on player logon
        if (win32_PlayMIDIFile(GameState.MIDIDevice, GameState.Window, "audio/dark2.mid"))
        {
            OutputDebugStringA("Error starting MIDI file.");
        }
    }
    else if (strcmp("char.status", command) == 0)
    {
        
        jsmn_init(&parser);
        
        int Result = jsmn_parse(&parser, jsonObject, strlen(jsonObject), tokens, 128);
        
        if (Result > 0)
        {
            if (tokens[0].type == JSMN_OBJECT)
            {
                char valueBuff[256] = {};
                uint32 valueInt = 0;
                
                for (int Index = 1, Size = Result; Index < Size; Index+=2)
                {
                    if (tokens[Index].type != JSMN_STRING)
                    {
                        OutputDebugStringA("JSON Error:  Key not a string\n");
                        continue;
                    }
                    
                    int start = tokens[Index+1].start;
                    int size = tokens[Index+1].end - start;
                    if (jsoneq(jsonObject, &tokens[Index], "level") == 0) 
                    {
                        memcpy( valueBuff, &jsonObject[start], size );
                        
                        valueInt = atoi(valueBuff);
                        GameState.User.Player.Level = (uint16) valueInt;
                        // use the value
                        OutputDebugStringA("Level:     ");
                    }
                    else if (jsoneq(jsonObject, &tokens[Index], "tnl") == 0) 
                    {
                        memcpy( valueBuff, &jsonObject[start], size );
                        
                        valueInt = atoi(valueBuff);
                        GameState.User.Player.ExpTNL = valueInt;
                        // use the value
                        OutputDebugStringA("ExpTNL:    ");
                    }
                    else
                    {
                        OutputDebugStringA("Unhandled key.\n");
                    }
                    
                    // print the value for now
                    OutputDebugStringA(valueBuff);
                    OutputDebugStringA("\n");
                    
                    // clear the value
                    memset(valueBuff, 0, 256);
                    valueInt = 0;
                }
            }
        }
        else if (Result == JSMN_ERROR_INVAL)
        {
            OutputDebugStringA("bad token, JSON string is corrupted\n");
        }
        else if (Result == JSMN_ERROR_NOMEM)
        {
            // If you get JSMN_ERROR_NOMEM, you can re-allocate more tokens and call jsmn_parse once more.
            OutputDebugStringA("not enough tokens, JSON string is too large\n");
        }
        else if (Result == JSMN_ERROR_PART)
        {
            /*
            If you read json data from the stream, you can periodically call jsmn_parse and check if 
                return value is JSMN_ERROR_PART. You will get this error until you reach the end of JSON data.
                */
            OutputDebugStringA("JSON string is too short, expecting more JSON data\n");
        }
        else
        {
            OutputDebugStringA("Failed to parse JSON\n");
        }
    }
    else if (strcmp("xpupdate", command) == 0)
    {
        
        jsmn_init(&parser);
        
        int Result = jsmn_parse(&parser, jsonObject, strlen(jsonObject), tokens, 128);
        
        if (Result > 0)
        {
            if (tokens[0].type == JSMN_OBJECT)
            {
                char valueBuff[256] = {};
                uint32 valueInt = 0;
                
                for (int Index = 1, Size = Result; Index < Size; Index+=2)
                {
                    if (tokens[Index].type != JSMN_STRING)
                    {
                        OutputDebugStringA("JSON Error:  Key not a string\n");
                        continue;
                    }
                    
                    int start = tokens[Index+1].start;
                    int size = tokens[Index+1].end - start;
                    if (jsoneq(jsonObject, &tokens[Index], "exp") == 0) 
                    {
                        memcpy( valueBuff, &jsonObject[start], size );
                        
                        valueInt = atoi(valueBuff);
                        GameState.User.Player.ExpTNL = valueInt;
                        // use the value
                        OutputDebugStringA("ExpTNL:    ");
                    }
                    else
                    {
                        OutputDebugStringA("Unknown key.\n");
                    }
                    
                    // print the value for now
                    OutputDebugStringA(valueBuff);
                    OutputDebugStringA("\n");
                    
                    // clear the value
                    memset(valueBuff, 0, 256);
                    valueInt = 0;
                }
            }
        }
        else if (Result == JSMN_ERROR_INVAL)
        {
            OutputDebugStringA("bad token, JSON string is corrupted\n");
        }
        else if (Result == JSMN_ERROR_NOMEM)
        {
            // If you get JSMN_ERROR_NOMEM, you can re-allocate more tokens and call jsmn_parse once more.
            OutputDebugStringA("not enough tokens, JSON string is too large\n");
        }
        else if (Result == JSMN_ERROR_PART)
        {
            /*
            If you read json data from the stream, you can periodically call jsmn_parse and check if 
                return value is JSMN_ERROR_PART. You will get this error until you reach the end of JSON data.
                */
            OutputDebugStringA("JSON string is too short, expecting more JSON data\n");
        }
        else
        {
            OutputDebugStringA("Failed to parse JSON\n");
        }
    }
    else if (strcmp("roominfo", command) == 0)
    {
        const int SOUTHWEST = 1;
        const int SOUTH = 2;
        const int SOUTHEAST = 4;
        const int WEST = 8;
        const int EAST = 16;
        const int NORTHWEST = 32;
        const int NORTH = 64;
        const int NORTHEAST = 128;
        const int UP = 256;
        const int DOWN = 512;
        
        /* TODO(jon):

GMCP In: room.info 
{
  "num":-788242509,
"id":"Stonefist Temple#2",
"name":"Stonefist Temple",
"zone":"Stonefist Temple",
"desc":"You`re in a large circular room at the center of the several clan headquarters. Each archway is adorned with symbols attributed to each clan, marks made to honor particular members, murals to remember great battles, and the occasional lewd message taunting rival clans, most likely left by overly enthusiastic initiates.",
"terrain":"plains",
"details":"",
"exits":{"N":1334286014,"S":-788242505,"E":-788242507,"W":-788242503,"U":1334286020,"NE":-788242508,"NW":-788242502,"SE":-788242506,"SW":-788242504},
"coord":{"id":0,"x":-1,"y":-1,"cont":0}
}

*/
        
        jsmn_init(&parser);
        int Result = jsmn_parse(&parser, jsonObject, strlen(jsonObject), tokens, 128);
        
        if (Result > 0)
        {
            // handle tokens
            if (tokens[0].type == JSMN_OBJECT)
            {
                bool32 parsingExits = false;
                
                for (int Index = 1, Size = Result; Index < Size; Index++)
                {
                    if (tokens[Index].type != JSMN_STRING)
                    {
                        OutputDebugStringA("JSON Error:  Key not a string\n");
                        continue;
                    }
                    
                    if (jsoneq(jsonObject, &tokens[Index], "exits") == 0) 
                    {
                        OutputDebugStringA("Exits: ");
                        ++Index; // skip the object token
                        
                        GameState.Room.Exits = 0;
                        parsingExits = true;
                    }
                    else if (parsingExits)
                    {
                        if (jsoneq(jsonObject, &tokens[Index], "E") == 0) 
                        {
                            OutputDebugStringA("East ");
                            GameState.Room.Exits |= EAST;
                            Index++; // skip the useless value
                        }
                        else if (jsoneq(jsonObject, &tokens[Index], "W") == 0) 
                        {
                            OutputDebugStringA("West ");
                            GameState.Room.Exits |= WEST;
                            Index++; // skip the useless value
                        }
                        else if (jsoneq(jsonObject, &tokens[Index], "N") == 0) 
                        {
                            OutputDebugStringA("North ");
                            GameState.Room.Exits |= NORTH;
                            Index++; // skip the useless value
                        }
                        else if (jsoneq(jsonObject, &tokens[Index], "S") == 0) 
                        {
                            OutputDebugStringA("South ");
                            GameState.Room.Exits |= SOUTH;
                            Index++; // skip the useless value
                        }
                        else if (jsoneq(jsonObject, &tokens[Index], "NW") == 0) 
                        {
                            OutputDebugStringA("Northwest ");
                            GameState.Room.Exits |= NORTHWEST;
                            Index++; // skip the useless value
                        }
                        else if (jsoneq(jsonObject, &tokens[Index], "NE") == 0) 
                        {
                            OutputDebugStringA("Northeast ");
                            GameState.Room.Exits |= NORTHEAST;
                            Index++; // skip the useless value
                        }
                        else if (jsoneq(jsonObject, &tokens[Index], "SW") == 0) 
                        {
                            OutputDebugStringA("Southwest ");
                            GameState.Room.Exits |= SOUTHWEST;
                            Index++; // skip the useless value
                        }
                        else if (jsoneq(jsonObject, &tokens[Index], "SE") == 0) 
                        {
                            OutputDebugStringA("Southeast ");
                            GameState.Room.Exits |= SOUTHEAST;
                            Index++; // skip the useless value
                        }
                        else if (jsoneq(jsonObject, &tokens[Index], "U") == 0) 
                        {
                            OutputDebugStringA("Up ");
                            GameState.Room.Exits |= UP;
                            Index++; // skip the useless value
                        }
                        else if (jsoneq(jsonObject, &tokens[Index], "D") == 0) 
                        {
                            OutputDebugStringA("Down ");
                            GameState.Room.Exits |= DOWN;
                            Index++; // skip the useless value
                        }
                    }
                    else
                    {
                        OutputDebugStringA("Skipping unused key.\n");
                        Index++;
                    }
                }
                OutputDebugStringA("\n");
            }
        }
    }
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
                  data, dataSize);
        data = {};
        
        OutputDebugStringA("GMCP In: ");
        OutputDebugStringA(GameState.GMCP.BufferIn);
        OutputDebugStringA("\n");
        
        handleGMCP();
    }
    // zero out Telnet.negBuffers.subNegotiation
    memset(Telnet.negBuffers.subNegotiation, 0, Telnet.negBuffers.subNegSize);
    
    // call GMCPHandlers.handleGMCP(data) - this will update the client etc.
}
