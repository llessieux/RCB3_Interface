#pragma once

#include "com.h"
#include "socket.h"

class ezurio : public com
{
public:
    ezurio(char *address,int port);
    virtual ~ezurio(void);

    virtual BOOL IsOK() { return m_opened; }

	virtual BOOL OpenCOM    (int nId);
	virtual BOOL CloseCOM   ();
	virtual BOOL ReadCOM    (void* buffer, int nBytesToRead, int* pBytesRead);

    //The which_controler extension allows communication with other controler attached to the Wism module.
    //By default 0 is the RCB3
    //1 for an extra controler
    //2 for local commands
    virtual BOOL WriteCOM   (void* buffer, int nBytesToWrite, int* pBytesWritten,int which_controler = 0); //
    virtual BOOL IsDataPresent();
	virtual int ByteToRead();

    virtual bool needSignal() { return false; }

private:
    Sock m_sock;
};
