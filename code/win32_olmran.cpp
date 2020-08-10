/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jonathan Hawranko $
   $Notice: (C) Copyright 2020 by Jonathan Hawranko. All Rights Reserved. $
   ======================================================================== */

#include "olmran_platform.h"

#include <ws2tcpip.h>
#include <windows.h>
#include <richedit.h>

#include "win32_olmran.h"

#include "olmran_gmcp.cpp"
#include "olmran_telnet.cpp"
#include "ansi.cpp"
#include "olmran_state.cpp"
#include "win32_sockets.cpp"
#include "win32_controls.cpp"

internal DWORD
win32_StopMIDIPlayback(midi_device *MIDIDevice)
{
    DWORD Return = 0L;
    if (MIDIDevice->DeviceID)
    {
        MCI_GENERIC_PARMS mciGenericParms = {};
        
        Return = mciSendCommand(MIDIDevice->DeviceID, MCI_STOP, MCI_NOTIFY, (DWORD_PTR) &mciGenericParms);
        if (Return)
            OutputDebugStringA("MIDI:  Error STOPPING MIDI playback.\n");
        
        Return = mciSendCommandA(MIDIDevice->DeviceID, MCI_CLOSE, 0, (DWORD_PTR) &mciGenericParms);
        if (Return)
            OutputDebugStringA("MIDI:  Error closing MIDI device.\n");
        
        MIDIDevice->DeviceID = 0;
    }
    MIDIDevice->IsPlaying = false;
    
    return Return;
}

// Plays a specified MIDI file by using MCI_OPEN and MCI_PLAY. Returns 
// as soon as playback begins. The window procedure function for the 
// specified window will be notified when playback is complete. 
// Returns 0L on success; otherwise, it returns an MCI error code.
internal DWORD 
win32_PlayMIDIFile(midi_device *MIDIDevice, HWND Window, LPSTR MIDIFileName)
{
    if (MIDIDevice->IsPlaying)
        win32_StopMIDIPlayback(MIDIDevice);
    
    DWORD dwReturn;
    MCI_PLAY_PARMS mciPlayParms = {};
    
    if (MIDIDevice->DeviceID)
        win32_StopMIDIPlayback(MIDIDevice);
    
    MCI_OPEN_PARMS mciOpenParms = {};
    MCI_STATUS_PARMS mciStatusParms = {};
    
    // Open the device by specifying the device and filename.
    // MCI will attempt to choose the MIDI mapper as the output port.
    mciOpenParms.lpstrDeviceType = "sequencer";
    mciOpenParms.lpstrElementName = MIDIFileName;
    
    dwReturn = mciSendCommandA(0, MCI_OPEN,
                               MCI_OPEN_TYPE | MCI_OPEN_ELEMENT,
                               (DWORD_PTR) &mciOpenParms);
    if (dwReturn)
    {
        // Failed to open device. Don't close it; just return error.
        return (dwReturn);
    }
    
    // The device opened successfully; get the device ID.
    MIDIDevice->DeviceID = mciOpenParms.wDeviceID;
    
    // Check if the output port is the MIDI mapper.
    mciStatusParms.dwItem = MCI_SEQ_STATUS_PORT;
    
    dwReturn = mciSendCommandA(MIDIDevice->DeviceID, MCI_STATUS, 
                               MCI_STATUS_ITEM, (DWORD_PTR) &mciStatusParms);
    if (dwReturn)
    {
        mciSendCommandA(MIDIDevice->DeviceID, MCI_CLOSE, 0, NULL);
        return (dwReturn);
    }
    // Begin playback. The window procedure function for the parent 
    // window will be notified with an MM_MCINOTIFY message when 
    // playback is complete. At this time, the window procedure closes 
    // the device.
    mciPlayParms.dwCallback = (DWORD_PTR) Window;
    dwReturn = mciSendCommandA(MIDIDevice->DeviceID, MCI_PLAY, MCI_NOTIFY, 
                               (DWORD_PTR) &mciPlayParms);
    if (dwReturn)
    {
        mciSendCommandA(MIDIDevice->DeviceID, MCI_CLOSE, 0, NULL);
        return (dwReturn);
    }
    
    MIDIDevice->IsPlaying = true;
    return (0L);
}

LRESULT CALLBACK
win32_MainWindowCallback(HWND   Window,
                         UINT   Message,
                         WPARAM WParam,
                         LPARAM LParam)
{
    LRESULT Result = 0;
    HWND GameOutput;
    HWND GameInput;
    
    switch(Message)
    {
        case WM_CREATE:
        {
            // Create Edit Control
            GameOutput = CreateGameOutput(Window, 0, 0, 0, 0, (HMENU) ID_EDITCHILD,
                                          (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE));
            GameState.GameOutput = {};
            GameState.GameOutput.Window = GameOutput;
            
            // Create Input Control
            GameInput = CreateGameInput(Window, (HMENU) ID_INPUTCHILD, (HINSTANCE) GetWindowLongPtr(Window, GWLP_HINSTANCE));
            GameState.GameInput = {};
            GameState.GameInput.Window = GameInput;
            
            // Set starting Text Color
            GameState.CurrentColor = C_RESET;
            
            // Set autosneak
            GameState.AutoSneak = false;
            
        } break; 
        
        case WM_SETFOCUS:
        {
            SetFocus(GameState.GameInput.Window); 
        } break; 
        
        case WM_SIZE:
        {
            // Make the control the size of the window's client 
            MoveWindow(GameState.GameOutput.Window, 
                       5, 5,                  // starting x- and y-coordinates 
                       LOWORD(LParam)-10,     // width of client area 
                       HIWORD(LParam)-40,     // height of client area 
                       TRUE);                 // repaint window 
            
            MoveWindow(GameState.GameInput.Window, 
                       5, HIWORD(LParam)-30,  // starting x- and y-coordinates 
                       LOWORD(LParam)-10,     // width of client area 
                       20,                    // height of client area 
                       TRUE);                 // repaint window 
            
            SetFocus(GameState.GameInput.Window);
        } break;
        
        case WM_DESTROY:
        {
            // TODO(jon):  Handle this as an error, recreate window?
            GlobalRunning = false;
        } break;
        
        case WM_CLOSE:
        {
            // TODO(jon):  Handle this with a message to the user?
            GlobalRunning = false;
        } break;
        
        case WM_ACTIVATEAPP:
        {
            SetFocus(GameState.GameInput.Window);
        } break;
        
        case MM_MCINOTIFY:
        {
            OutputDebugStringA("MM_MCINOTIFY:  MIDI Playback stopped.");
            GameState.MIDIDevice->IsPlaying = false;
        } break;
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        }
    }
    
    return Result;
}

int CALLBACK
WinMain(
        HINSTANCE   Instance,
        HINSTANCE   PrevInstance,
        LPSTR       CommandLine,
        int         ShowCode)
{
    WNDCLASSA WindowClass = {};
    // TODO(jon):  Check if any of this is needed.
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = win32_MainWindowCallback;
    WindowClass.hInstance = Instance;
    //    WindowClass.hIcon = ;
    //    WindowClass.hCursor = ;
    WindowClass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    //    WindowClass.lpszMenuName = ;
    WindowClass.lpszClassName = "OlmranWindowClass";
    
    if(RegisterClass(&WindowClass))
    {
        HWND WindowHandle =
            CreateWindowEx(
                           0,
                           WindowClass.lpszClassName,
                           "Olmran Client",
                           WS_OVERLAPPEDWINDOW|WS_SIZEBOX|WS_VISIBLE,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           0,
                           0,
                           Instance,
                           0);
        
        if(WindowHandle)
        {
            GameState.Window = WindowHandle;
            GameState.isInitialized = true;
            
            // TODO(jon):  Pool these VirtualAllocs together.
            GameState.GameInput.BufferLength = 512;
            GameState.GameInput.Buffer = (char *)VirtualAlloc(0, GameState.GameInput.BufferLength, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            GameState.CommandHistory.BufferSize = 512;
            GameState.CommandHistory.NumberOfCommands = 10;
            GameState.CommandHistory.CurrentPosition = -1;
            GameState.CommandHistory.Commands = (char *)VirtualAlloc(0, GameState.CommandHistory.BufferSize * GameState.CommandHistory.NumberOfCommands, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            GameState.CommandHistory.CurrentSize = 512;
            GameState.CommandHistory.CurrentCommand = (char *)VirtualAlloc(0, GameState.CommandHistory.CurrentSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            GameState.GMCP.BufferSize = 1024;
            GameState.GMCP.BufferOut = (char *)VirtualAlloc(0, GameState.GMCP.BufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            GameState.GMCP.BufferIn = (char *)VirtualAlloc(0, GameState.GMCP.BufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            GameState.User.Account = {};
            GameState.User.Player = {};
            
            GameState.MIDIDevice = (midi_device *)VirtualAlloc(0, sizeof(midi_device), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            DWORD ThreadID;
            HANDLE SocketListenThreadHandle;
            if (win32_InitAndConnectSocket()==0)
            {
                OutputDebugStringA("Socket Connected\r\n");
                
                TelnetInit(Telnet);
                
                char *Param = "Socket listening.\r\n";
                
                SocketListenThreadHandle = CreateThread(0, 0, SocketListenThreadProc, Param, 0, &ThreadID);
            }
            else
            {
                win32_AppendText(GameState.GameOutput.Window, TEXT("Could not connect to server.\r\n"));
                OutputDebugStringA("Error in win32_InitAndConnectSocket()");
                SocketListenThreadHandle = 0;
            }
            
            GlobalRunning = true;
            while (GlobalRunning)
            {
                MSG Message;
                BOOL MessageResult = GetMessageA(&Message, 0, 0, 0);
                if(MessageResult > 0)
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                else
                    break;
            }
            // NOTE(jon):  Is this necessary?  Windows might clean it up itself.
            win32_CloseSocket();
            if (SocketListenThreadHandle) { CloseHandle(SocketListenThreadHandle); }
        }
        else
        {
            // TODO(jon): Logging (failed)
        }
    }
    else
    {
        // TODO(jon):  Logging (failed)
    }
    
    return 0;
}
