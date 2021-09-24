#include "StdAfx.h"
#include "ezurio.h"

extern int GetAnswerSizeForCommand(char command);

ezurio::ezurio(char *address,int port)
{
    m_opened = false;
    m_sock = connect(address,port);
    if ( m_sock.valid )
        m_opened = true;
}

ezurio::~ezurio(void)
{
    if ( m_opened) 
        CloseCOM();
}

BOOL ezurio::OpenCOM(int nId)
{
    return m_opened ? TRUE : FALSE;
}

BOOL ezurio::CloseCOM()
{
    if ( !m_opened )
        return FALSE;

    disconnect(m_sock);
    m_opened = false;
    return TRUE;
}

BOOL ezurio::ReadCOM(void* buffer, int nBytesToRead, int* pBytesRead)
{
    if ( !m_opened )
        return FALSE;

    int r = recv(m_sock,(char *)buffer,nBytesToRead);
    if ( pBytesRead  )
        *pBytesRead = r;

    return TRUE;
}

BOOL ezurio::WriteCOM(void* buffer, int nBytesToWrite, int* pBytesWritten,int which_controler)
{
    if ( !m_opened )
        return FALSE;
    char temp_buffer[256];
    int answer_size = GetAnswerSizeForCommand(*((char *)buffer)); //FIXME

    int size = 0;
    int start = 0;
    temp_buffer[0] = which_controler;
    switch(which_controler)
    {
    case 0:
        start = 3;
        temp_buffer[1] = nBytesToWrite;
        temp_buffer[2] = answer_size;
        size = min(nBytesToWrite,256-start);
        memcpy(temp_buffer+start,buffer,size);
        size += start;
        break;
    default:
        start = 1;
        size = min(nBytesToWrite,256-start);
        memcpy(temp_buffer+start,buffer,size);
        size += start;
        break;
    }
    int s = send(m_sock, temp_buffer, size);
    if ( pBytesWritten )
        *pBytesWritten = s-start;
    return TRUE;
}

BOOL ezurio::IsDataPresent()
{
    return FALSE;
}

int ezurio::ByteToRead()
{
    return 0;
}
