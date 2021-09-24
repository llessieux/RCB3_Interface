/*
 * File:         com.cpp
 * Date:         January, 2007
 * Description:  Handles the basic communication via COM PORT

 * Author:  http://www.cppfrance.com/code.aspx?ID=33724
 * Modified Laurent Lessieux http:www.lessieux.com
*/
#include "stdafx.h"

#ifdef WIN32
#include <string.h>
#include "com.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#endif

void errCom::setMsg(char* message)
{
	if (msg!=NULL) delete (msg); //destruction de l'ancienne variable
    size_t msize = strlen(message)+1;
	msg=new char[msize];//creation de la variable msg
	strcpy_s(msg,msize,message);
};

errCom::errCom()
{
	msg=NULL;
	setMsg("erreur non définie");
};

errCom::errCom(char* message)
{
	msg=NULL;
	setMsg(message);
};

errCom::~errCom()
{
	if (msg!=NULL) delete (msg); 
};

const char* errCom::quoi(){
	return msg;
};

errCom::errCom(const errCom& cpy)
{
	msg=NULL;
	setMsg(cpy.msg);
};





com::com(int numPort,int vitesse)
{
#ifdef WIN32
	g_hCOM=NULL;

	g_cto.ReadIntervalTimeout = MAX_WAIT_READ;
	g_cto.ReadTotalTimeoutMultiplier=2;
	g_cto.ReadTotalTimeoutConstant=MAX_WAIT_READ;
	g_cto.WriteTotalTimeoutMultiplier=0;
	g_cto.WriteTotalTimeoutConstant=0;
	
	g_dcb.BaudRate=vitesse;//9600
	g_dcb.ByteSize=8;
	g_dcb.DCBlength=sizeof(DCB);
	g_dcb.EofChar=0x1A;
	g_dcb.ErrorChar='?';
	g_dcb.EvtChar=0x10;
	g_dcb.fAbortOnError=FALSE;
	g_dcb.fBinary=TRUE;
	g_dcb.fDsrSensitivity=FALSE;
	g_dcb.fDtrControl=DTR_CONTROL_ENABLE;
	g_dcb.fDummy2=0;
	g_dcb.fErrorChar=FALSE;
	g_dcb.fInX=FALSE;
	g_dcb.fNull=FALSE;
	g_dcb.fOutX=FALSE;
	g_dcb.fOutxCtsFlow=FALSE;
	g_dcb.fOutxDsrFlow=FALSE;
	g_dcb.fParity=FALSE;
	g_dcb.fRtsControl=RTS_CONTROL_ENABLE;
	g_dcb.fTXContinueOnXoff=FALSE;
	g_dcb.Parity=NOPARITY;
	g_dcb.StopBits=ONESTOPBIT;
	g_dcb.wReserved=0;
	g_dcb.wReserved1=0;//n'était pas définit ???
	g_dcb.XoffChar=0x13;
	g_dcb.XoffLim=0x100;
	g_dcb.XonChar=0x11;
	g_dcb.XonLim=0x100;
#else
    m_vitesse = vitesse;
#endif
	if(!OpenCOM(numPort)) 
    {
        m_opened = false;
    }
    else
        m_opened = true;
}

com::~com()
{
	CloseCOM();
}

int com::ByteToRead()
{
#ifdef WIN32
	COMSTAT cs = {0};
	DWORD dwErrors;
	ClearCommError(g_hCOM, &dwErrors, &cs);
	return cs.cbInQue;
#else
    return 0;
#endif
}

BOOL com::OpenCOM    (int nId)
{
	/* variables locales */
    char szCOM[16];

    /* construction du nom du port, tentative d'ouverture */
#ifdef WIN32
    sprintf_s(szCOM,16, "\\\\.\\COM%d", nId);//modifie pour prendre les port com>9
    g_hCOM = CreateFile(szCOM, GENERIC_READ|GENERIC_WRITE, 0, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, NULL);
    if(g_hCOM == INVALID_HANDLE_VALUE)
    {
        //throw errCom("Erreur lors de l'ouverture du port com");
        return FALSE;
    }

    /* affectation taille des tampons d'émission et de réception */
    SetupComm(g_hCOM, RX_SIZE, TX_SIZE);

    /* configuration du port COM */
    if(!SetCommTimeouts(g_hCOM, &g_cto) || !SetCommState(g_hCOM, &g_dcb))
    {
        //throw errCom("Erreur lors de la configuration du port com");
        CloseHandle(g_hCOM);
        return FALSE;
    }

    /* on vide les tampons d'émission et de réception, mise à 1 DTR */
    PurgeComm(g_hCOM, PURGE_TXCLEAR|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_RXABORT);
    EscapeCommFunction(g_hCOM, SETDTR);
    EscapeCommFunction(g_hCOM, CLRDTR);

#else
    sprintf_s(szCOM,16, "/dev/ttyAM%d", nId);//modifie pour prendre les port com>9
    g_hCOM = open(szCOM,O_RDWR|O_NONBLOCK);

    if ( g_hCOM < 0 )
        return FALSE;

    struct termios  tio;
    memset(&tio,0,sizeof(tio));
    tio.c_cflag = CS8 | CLOCAL | CREAD;
    tio.c_cc[VTIME] = 100;
    cfsetispeed(&tio,m_vitesse);
    cfsetospeed(&tio,m_vitesse);
    tcsetattr(g_hCOM,TCSANOW,&tio);

#endif
    
    return TRUE;
}

BOOL com::CloseCOM   ()
{
	/* fermeture du port COM */
    if (g_hCOM)
        CloseHandle(g_hCOM);
    g_hCOM = NULL;
    return TRUE;
}

BOOL com::ReadCOM    (void* buffer, int nBytesToRead, int* pBytesRead)
{
#ifdef WIN32
	return ReadFile(g_hCOM, buffer, nBytesToRead, (unsigned long *)pBytesRead, NULL);
#else

    int len = read(g_hCOM,buffer,nBytesToRead);
    if ( len > 0 )
    {
        
        *pBytesRead = len;
        return TRUE;
    }

    return FALSE;
#endif
}

BOOL com::WriteCOM   (void* buffer, int nBytesToWrite, int* pBytesWritten,int /*ext*/)
{
	/* écriture sur le port */
#ifdef WIN32
    BOOL ok =  WriteFile(g_hCOM, buffer, nBytesToWrite, (unsigned long *)pBytesWritten, NULL);
    FlushFileBuffers(g_hCOM);
    return ok;
#else

    int len = write(g_hCOM,buffer,nBytesToWrite);
    if ( len > 0 )
    {
        if ( pBytesWritten )
            *pBytesWritten = len;
        return TRUE;
    }
    flush(g_hCOM);

    return FALSE;

#endif    
}

BOOL com::IsDataPresent()
{
#ifdef WIN32
    DWORD mask;
    GetCommMask(g_hCOM,&mask);

    return (mask & EV_RXCHAR) ? TRUE : FALSE; 
#else
    return FALSE;
#endif
}