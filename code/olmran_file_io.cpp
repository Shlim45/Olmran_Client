
internal void
WriteToFile(const char *FileName, char *DataBuffer)
{
    HANDLE hFile; 
    //char DataBuffer[] = "This is some test data to write to the file.\r\nWindows NewLine\nLinux Newline";
    DWORD dwBytesToWrite = (DWORD)strlen(DataBuffer);
    DWORD dwBytesWritten = 0;
    BOOL bErrorFlag = FALSE;
    
    hFile = CreateFileA(FileName,               // name of the write
                        GENERIC_WRITE,          // open for writing
                        0,                      // do not share
                        NULL,                   // default security
                        OPEN_ALWAYS,            // open if exist, create otherwise
                        FILE_ATTRIBUTE_NORMAL,  // normal file
                        NULL);                  // no attr. template
    
    if (hFile == INVALID_HANDLE_VALUE) 
    { 
        OutputDebugStringA("Terminal failure: Unable to open file for write.\n");
        return;
    }
    
    bErrorFlag = WriteFile( 
                           hFile,           // open file handle
                           DataBuffer,      // start of data to write
                           dwBytesToWrite,  // number of bytes to write
                           &dwBytesWritten, // number of bytes that were written
                           NULL);            // no overlapped structure
    
    if (bErrorFlag == FALSE)
    {
        OutputDebugStringA("Terminal failure: Unable to write to file.\n");
    }
    else
    {
        if (dwBytesWritten != dwBytesToWrite)
        {
            // This is an error because a synchronous write that results in
            // success (WriteFile returns TRUE) should write all data as
            // requested. This would not necessarily be the case for
            // asynchronous writes.
            OutputDebugStringA("Error: dwBytesWritten != dwBytesToWrite\n");
        }
        else
        {
            OutputDebugStringA("Wrote successfully.\n");
        }
    }
    
    CloseHandle(hFile);
}

global_variable DWORD g_BytesTransferred = 0;

internal VOID CALLBACK 
FileIOCompletionRoutine(DWORD dwErrorCode,
                        DWORD dwNumberOfBytesTransfered,
                        LPOVERLAPPED lpOverlapped )
{
    OutputDebugStringA("Error code:\n");
    OutputDebugStringA("Number of bytes:\n");
    g_BytesTransferred = dwNumberOfBytesTransfered;
}

internal void
ReadFromFile(const char *FileName)
{
    HANDLE hFile; 
    DWORD  dwBytesRead = 0;
    char   ReadBuffer[Kilobytes(64)] = {0};
    OVERLAPPED ol = {0};
    
    OutputDebugStringA("\n");
    
    hFile = CreateFile(FileName,              // file to open
                       GENERIC_READ,          // open for reading
                       FILE_SHARE_READ,       // share for reading
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // normal file
                       NULL);                 // no attr. template
    
    if (hFile == INVALID_HANDLE_VALUE) 
    { 
        OutputDebugStringA("Terminal failure: unable to open file for read.\n");
        return; 
    }
    
    // Read one character less than the buffer size to save room for
    // the terminating NULL character. 
    
    if( FALSE == ReadFileEx(hFile, ReadBuffer, Kilobytes(64)-1, &ol, FileIOCompletionRoutine) )
    {
        OutputDebugStringA("Terminal failure: Unable to read from file.\n");
        CloseHandle(hFile);
        return;
    }
    SleepEx(5000, TRUE);
    dwBytesRead = g_BytesTransferred;
    // This is the section of code that assumes the file is ANSI text. 
    // Modify this block for other data types if needed.
    
    if (dwBytesRead > 0 && dwBytesRead <= Kilobytes(64)-1)
    {
        ReadBuffer[dwBytesRead]='\0'; // NULL character
        
        OutputDebugStringA(ReadBuffer);
        OutputDebugStringA("\n");
    }
    else if (dwBytesRead == 0)
    {
        OutputDebugStringA("No data read from file %s\n");
    }
    else
    {
        OutputDebugStringA("\n ** Unexpected value for dwBytesRead ** \n");
    }
    
    // It is always good practice to close the open file handles even though
    // the app will exit here and clean up open handles anyway.
    
    CloseHandle(hFile);
}