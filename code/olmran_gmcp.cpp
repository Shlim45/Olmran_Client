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
        
        OutputDebugStringA(subData + '\0');
    }
    // zero out Telnet.negBuffers.subNegotiation
    memset(Telnet.negBuffers.subNegotiation, 0, Telnet.negBuffers.subNegSize);
    
    // call GMCPHandlers.handleGMCP(data) - this will update the client etc.
}
