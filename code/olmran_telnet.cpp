#include "olmran_telnet.h"

internal void
TelnetSendResponse(char byte1, char byte2, char byte3)
{
    char outResponse[3];
    memset(outResponse, 0, 3);
    
    outResponse[0] = byte1;
    outResponse[1] = byte2;
    outResponse[2] = byte3;
    
    Win32WriteToSocket(Socket.sock, outResponse, 3, 0);
}

internal void
TelnetInit(telnet_state State)
{
    State.negState = 0;
    State.negBuffers.subNegotiation = {};
    State.negBuffers.subNegSize = 0;
    State.negBuffers.subnegOffset = 0;
    State.outOffset = 0;
    State.xBuffers.receivedDX = {};
    State.xBuffers.receivedWX = {};
    State.xBuffers.sentDX = {};
    State.xBuffers.sentWX = {};
}

internal int64
TelnetNegotiation(telnet_state Tel, char *outBuffer, char *inData, uint32 inDataSize)
{
    // TODO(jon):  Tie this into GameMemory
    local_persist char outResponse[3];
    local_persist char subNegBuf[Kilobytes(4)];
    memset(outResponse, 0, 3);
    memset(subNegBuf, 0, Kilobytes(4));
    
    Tel.negBuffers.subNegotiation = subNegBuf;
    Tel.negBuffers.subNegSize = Kilobytes(4);
    
    char reply = {};
    
    memset(Tel.negBuffers.subNegotiation, 0, Tel.negBuffers.subNegSize);
    
    bool32 exit = false;
    for (uint16 i = 0, len = (uint16) strlen(inData);
         i < len; i++)
    {
        char b = inData[i];
        if (b == 0)
            exit = true;
        if (exit)
            break;
        
        switch (Tel.negState)
        {
            case STATE_DATA:
            {
                if (b == TN_IAC)
                    Tel.negState = STATE_IAC;
                // treat CR and CRLF as LF
                // CRLF case, ignore CR
                else if (b == 13 
                         && i < (len-1)
                         && inData[i+1] == 10)
                {
                    // skip
                }
                // LFCR case, ignore CR
                else if (b == 13 
                         && inData[i-1] == 10)
                {
                    // skip
                }
                // CR case, convert to LF
                else if (b == 13 
                         && i < (len-1)
                         && inData[i+1] != 10)
                {
                    outBuffer[Tel.outOffset] = 10;
                    Tel.outOffset++;
                }
                // CR case at end of string, convert to LF
                else if (b == 13 
                         && i == (len-1)
                         && inData[i-1] != 10)
                {
                    outBuffer[Tel.outOffset] = 10;
                    Tel.outOffset++;
                }
                else
                {
                    // TODO(jon):  seg fault?
                    outBuffer[Tel.outOffset] = b;
                    Tel.outOffset++;
                }
            } break;
            
            case STATE_IAC:
            {
                switch (b)
                {
                    case TN_IAC:
                    {
                        Tel.negState = STATE_DATA;
                        outBuffer[i] = TN_IAC;
                    } break;
                    
                    case TN_WILL:
                    {
                        Tel.negState = STATE_IACWILL;
                    } break;
                    
                    case TN_WONT:
                    {
                        Tel.negState = STATE_IACWONT;
                    } break;
                    
                    case TN_DO:
                    {
                        Tel.negState = STATE_IACDO;
                    } break;
                    
                    case TN_DONT:
                    {
                        Tel.negState = STATE_IACDONT;
                    } break;
                    
                    case TN_EOR:
                    {
                        // end of record
                        OutputDebugStringA("TELNET NEGOTIATION: TN_EOR (unhandled)\n");
                    } break;
                    
                    case TN_SB:
                    {
                        Tel.negState = STATE_IACSB;
                    } break;
                    
                    case TN_SE:
                    {
                        exit = true;
                    } break;
                    
                    default:
                    {
                        Tel.negState = STATE_DATA;
                    } break;
                }
            } break;
            
            case STATE_IACWILL:
            {
                switch (b)
                {
                    case OPT_ECHO:
                    case OPT_EOR:
                    {
                        reply = TN_DONT;
                    } break;
                    
                    case OPT_BINARY:
                    case OPT_GMCP:
                    case CUSTOM_CLIENT_BYTE:
                    {
                        reply = TN_DO;
                    } break;
                    
                    case OPT_COMPRESS:
                    case OPT_COMPRESS2: // this is sent from client
                    case OPT_MSP:       // this is sent from client
                    case OPT_MSDP:      // this is sent from client
                    case OPT_NAWS:      // this is sent from client
                    default:
                    {
                        reply = TN_DONT;
                    } break;
                }
                TelnetSendResponse(TN_IAC, reply, b);
                if (b == OPT_GMCP)
                {
                    addGMCPSupport();
                }
                Tel.negState = STATE_DATA;
            } break;
            
            case STATE_IACWONT:
            {
                if (b == CUSTOM_CLIENT_BYTE)
                    reply = TN_DO;
                else
                    reply = TN_DONT;
                
                TelnetSendResponse(TN_IAC, reply, b);
                Tel.negState = STATE_DATA;
            } break;
            
            case STATE_IACDO:
            case STATE_IACDONT:
            {
                reply = TN_WONT;
                
                TelnetSendResponse(TN_IAC, reply, b);
                Tel.negState = STATE_DATA;
            } break;
            
            case STATE_IACSB:
            {
                Tel.negState = STATE_IACSBDATA;
            } break;
            
            case STATE_IACSBDATA:
            {
                if (b == TN_IAC)
                    Tel.negState = STATE_IACSBDATAIAC;
                else
                {
                    Tel.negBuffers.subNegotiation[Tel.negBuffers.subnegOffset] = b;
                    Tel.negBuffers.subnegOffset++;
                }
            } break;
            
            case STATE_IACSBDATAIAC:
            {
                if (b == TN_SE)
                {
                    // TODO(jon): handle_sb( subneg );
                    handle_gmcp( Tel.negBuffers.subNegotiation, Tel.negBuffers.subnegOffset );
                    Tel.negBuffers.subnegOffset = 0;
                }
                Tel.negState = STATE_DATA;
            } break;
            
            default:
            Tel.negState = STATE_DATA;
        }
    }
    
    if (Tel.outOffset == 0)
        return -1;
    
    // NOTE(jon):  Return offset + 1 to indicate 'length' of outBuffer;
    return Tel.outOffset;
}
