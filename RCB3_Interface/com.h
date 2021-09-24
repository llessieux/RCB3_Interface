/*
 * File:         com.h
 * Date:         January, 2007
 * Description:  Basic COM port communication
 * Author:       Laurent Lessieux
 *
 * Copyright (c) 2006 Laurent Lessieux
 *               www.lessieux.com
 */

#pragma once

#ifndef class_com
#define class_com

#ifdef WIN32

#include <windows.h>
#define COM_HANDLE HANDLE

#else

#define COM_HANDLE int
#define INVALID_HANDLE_VALUE -1

#endif

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#define RX_SIZE         4096    /* taille tampon d'entrée  */
#define TX_SIZE         4096    /* taille tampon de sortie */
#define MAX_WAIT_READ   100    /* temps max d'attente pour lecture (en ms) */

class errCom
{
private:
	char* msg;
	void setMsg(char*);
public:
	errCom();
	errCom(char*);
	errCom(const errCom&);//constructeur de copie pour les exceptions!
	~errCom();
	const char* quoi();
};

class com
{
private:
	COM_HANDLE g_hCOM;
    int m_vitesse;

#ifdef WIN32
	COMMTIMEOUTS g_cto;
	DCB g_dcb;
#endif
    
protected:
    bool m_opened;

public:
	com(int numPort=0,int vitesse=9600);
	virtual ~com();

    virtual BOOL IsOK() { return m_opened; }

	virtual BOOL OpenCOM    (int nId);
	virtual BOOL CloseCOM   ();
	virtual BOOL ReadCOM    (void* buffer, int nBytesToRead, int* pBytesRead);
	virtual BOOL WriteCOM   (void* buffer, int nBytesToWrite, int* pBytesWritten,int ext = 0);
    virtual BOOL IsDataPresent();
	virtual int ByteToRead();

    virtual bool needSignal() { return true; }
};




#endif