/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jonathan Hawranko $
   $Notice: (C) Copyright 2020 by Jonathan Hawranko. All Rights Reserved. $
   ======================================================================== */

internal DWORD
Win32StopMIDIPlayback(midi_device *MIDIDevice)
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
        
        if (!Return)
        {
            MIDIDevice->IsPlaying = false;
            MIDIDevice->DeviceID = 0;
        }
    }
    
    return Return;
}

// Plays a specified MIDI file by using MCI_OPEN and MCI_PLAY. Returns 
// as soon as playback begins. The window procedure function for the 
// specified window will be notified when playback is complete. 
// Returns 0L on success; otherwise, it returns an MCI error code.
internal DWORD 
Win32PlayMIDIFile(midi_device *MIDIDevice, HWND Window, LPSTR MIDIFileName)
{
    if (MIDIDevice->IsPlaying || MIDIDevice->DeviceID)
        Win32StopMIDIPlayback(MIDIDevice);
    
    DWORD dwReturn;
    MCI_PLAY_PARMS mciPlayParms = {};
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
        return dwReturn;
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
        return dwReturn;
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
        return dwReturn;
    }
    
    MIDIDevice->IsPlaying = true;
    return 0L;
}
