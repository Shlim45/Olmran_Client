/* date = July 22nd 2020 7:15 pm */

#ifndef OLMRAN_TELNET_H
#define OLMRAN_TELNET_H


// Telnet

const char TN_BELL = static_cast<char>(7);

const char TN_EOR = static_cast<char>(239);
const char TN_SE = static_cast<char>(240);
const char TN_NOP = static_cast<char>(241);
const char TN_DM = static_cast<char>(242);
const char TN_B = static_cast<char>(243);
const char TN_IP = static_cast<char>(244);
const char TN_AO = static_cast<char>(245);
const char TN_AYT = static_cast<char>(246);
const char TN_EC = static_cast<char>(247);
const char TN_EL = static_cast<char>(248);
const char TN_GA = static_cast<char>(249);
const char TN_SB = static_cast<char>(250);
const char TN_WILL = static_cast<char>(251);
const char TN_WONT = static_cast<char>(252);
const char TN_DO = static_cast<char>(253);
const char TN_DONT = static_cast<char>(254);
const char TN_IAC = static_cast<char>(255);

const char TNSB_IS = 0;
const char TNSB_SEND = 1;


const char OPT_ECHO = 1;
const char OPT_STATUS = 5;
const char OPT_TIMING_MARK = 6;
const char OPT_TERMINAL_TYPE = 24;
const char OPT_EOR = 25;
const char OPT_NAWS = 31;
const char OPT_MSDP = 69; // http://tintin.sourceforge.net/msdp/
const char OPT_MSSP = static_cast<char>(70); // https://tintin.sourceforge.io/protocols/mssp/
const char OPT_COMPRESS = 85;
const char OPT_COMPRESS2 = 86;
const char OPT_MSP = 90;
const char OPT_MXP = 91;
const char OPT_102 = 102;
const char OPT_ATCP = static_cast<char>(200);
const char OPT_GMCP = static_cast<char>(201);

const char MSSP_VAR = 1;
const char MSSP_VAL = 2;

const char MSDP_VAR = 1;
const char MSDP_VAL = 2;
const char MSDP_TABLE_OPEN = 3;
const char MSDP_TABLE_CLOSE = 4;
const char MSDP_ARRAY_OPEN = 5;
const char MSDP_ARRAY_CLOSE = 6;


void processTelnetCommand(const char *command)
{
    char ch = command[1];
    
    char *_type;
    switch ((uint8)ch) {
        case 239:
        _type = "TN_EOR";
        break;
        case 249:
        _type = "TN_GA";
        break;
        case 250:
        _type = "SB";
        break;
        case 251:
        _type = "WILL";
        break;
        case 252:
        _type = "WONT";
        break;
        case 253:
        _type = "DO";
        break;
        case 254:
        _type = "DONT";
        break;
        case 255:
        _type = "IAC";
        break;
        default:
        _type="";
        //_type = QString::number((quint8)ch);
        break;
    }
    if (strlen(command) > 2) 
    {
        OutputDebugStringA("SERVER sent telnet:"); 
        OutputDebugStringA(_type);
        OutputDebugStringA(" + ");
        const char tmpBuf[2] = {command[2], '\0'};
        OutputDebugStringA(tmpBuf);
        OutputDebugStringA("\r\n");
    } else 
    {
        OutputDebugStringA("SERVER sent telnet:");
        OutputDebugStringA(_type);
        OutputDebugStringA("\r\n");
    }
}


#endif //OLMRAN_TELNET_H
