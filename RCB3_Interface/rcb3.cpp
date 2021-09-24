/*
 * File:         rcb3.c
 * Date:         January, 2007
 * Description:  Handles the basic communication with the RCB3
 * Author:       Laurent Lessieux
 *
 * Copyright (c) 2006 Laurent Lessieux
 *               www.lessieux.com
 */

#include "stdafx.h"

#ifndef UBYTE
#define UBYTE unsigned char
#endif

#ifndef UINT
#define UINT unsigned int
#endif


#include "rcb3.h"
#include "com.h"
#include "blueSmirf.h"
#include "ezurio.h"

#define MAX_RCB_RETRY 500

extern UBYTE GenerateCheckSum(UBYTE *commands,UINT size,bool sevenbitMask = false);
bool RCB3Interface::m_possibleCollision[256];

RCB3Interface::RCB3Interface(int comPort,int speed)
{
    static bool init = true;
    if ( init )
    {
        init = false;
        for(int i=0;i<256;i++)
            m_possibleCollision[i] = false;

        m_possibleCollision[0x0F5] = true;
        m_possibleCollision[0x0FC] = true;
        m_possibleCollision[0x0F9] = true;
        m_possibleCollision[0x0F0] = true;
        m_possibleCollision[0x0E9] = true;
        m_possibleCollision[0x0FF] = true;
        m_possibleCollision[0x0E5] = true;
        m_possibleCollision[0x0ED] = true;
        m_possibleCollision[0x0DC] = true;
        m_possibleCollision[0x0DA] = true;
        m_possibleCollision[0x0D8] = true;
        m_possibleCollision[0x0D6] = true;
        m_possibleCollision[0x0D4] = true;
        m_possibleCollision[0x0D2] = true;
        m_possibleCollision[0x0D0] = true;
        m_possibleCollision[0x0CE] = true;
    }

    m_lastCommand = 0;
    m_comPort = new com(comPort,speed);
    m_receivingNotification = false;
    m_lastPower = -1.0f;
    m_lastAD1 = -1.0f;
    m_lastAD2 = -1.0f;
    m_lastAD3 = -1.0f;
    m_lastPortsDone = 0xFFFF;
    for(int i=0;i<24;i++)
        m_lastPortsMotion[i] = 0xFF;
    for(int i=0;i<7;i++)
        m_lastRemoteControlData[i] = 0xFF;
}

RCB3Interface::RCB3Interface(int comPort,int speed,bool autodetectSpeed ,bool writeConfigToFlash )
{
    m_comPort = BlueSmirf::CreateComPortViaBlueSmirf(comPort,speed,autodetectSpeed,writeConfigToFlash);

    m_receivingNotification = false;
    m_lastPower = -1.0f;
    m_lastAD1 = -1.0f;
    m_lastAD2 = -1.0f;
    m_lastAD3 = -1.0f;
    m_lastPortsDone = 0xFFFF;
    for(int i=0;i<24;i++)
        m_lastPortsMotion[i] = 0xFF;
    for(int i=0;i<7;i++)
        m_lastRemoteControlData[i] = 0xFF;
}

RCB3Interface::RCB3Interface(char *address,int port)
{
    m_comPort = new ezurio(address,port);

    m_receivingNotification = false;
    m_lastPower = -1.0f;
    m_lastAD1 = -1.0f;
    m_lastAD2 = -1.0f;
    m_lastAD3 = -1.0f;
    m_lastPortsDone = 0xFFFF;
    for(int i=0;i<24;i++)
        m_lastPortsMotion[i] = 0xFF;
    for(int i=0;i<7;i++)
        m_lastRemoteControlData[i] = 0xFF;
}

RCB3Interface::~RCB3Interface()
{
    if ( m_comPort != NULL )
    {
        m_comPort->CloseCOM();
        delete m_comPort;
    }
}

void RCB3Interface::CheckUpdate()
{
    if ( m_receivingNotification )
    {
        if ( m_comPort->IsDataPresent() != FALSE )
        {
            char answer;
            int read = 0;
            m_comPort->ReadCOM(&answer,1,&read);
            UBYTE first_byte = answer;
            switch(first_byte)
            {
            case 0xFD:
                ReadRemoteControlData();
                break;
            case 0xFE:
                ReadAnalogData();
                break;
            case 0xFF:
                ReadPortsData();
                break;
            }
        }
    }
}

bool RCB3Interface::ReadRemoteControlData(UBYTE *buffer,int *read0)
{
    UBYTE answer[9];
    if ( buffer == NULL )
        buffer = answer;

    buffer[0] = 0xFD;
    int read = 0;
    m_comPort->ReadCOM(&buffer[1],8,&read);
    
    if ( read0 )
        *read0 = read + 1;

    UBYTE chck = GenerateCheckSum(buffer,8,true);
    if ( chck != buffer[8] )
        return false;

    memcpy(m_lastRemoteControlData,buffer+1,7);

    return true;
}

bool RCB3Interface::ReadAnalogData(UBYTE *buffer,int *read0)
{
    UBYTE answer[10];
    if ( buffer == NULL )
        buffer = answer;

    buffer[0] = 0xFE;
    int read = 0;
    m_comPort->ReadCOM(&buffer[1],9,&read);

    if ( read0 )
        *read0 = read + 1;

    UBYTE chck = GenerateCheckSum(buffer,9,true);
    if ( chck != buffer[8] )
        return false;

    UINT power = (((UINT)buffer[1])<<8) + buffer[2];
    UINT ad1 = (((UINT)buffer[3])<<8) + buffer[4];
    UINT ad2 = (((UINT)buffer[5])<<8) + buffer[6];
    UINT ad3 = (((UINT)buffer[7])<<8) + buffer[8];

    m_lastPower = (float)power * 0.01539f;
    m_lastAD1 = (float)ad1 * 0.00532f;
    m_lastAD2 = (float)ad2 * 0.00532f;
    m_lastAD3 = (float)ad3 * 0.00532f;
    return true;
}

bool RCB3Interface::ReadPortsData(UBYTE *buffer,int *read0)
{
    UBYTE answer[32];
     if ( buffer == NULL )
        buffer = answer;
   buffer[0] = 0xFF;
    int read = 0;
    m_comPort->ReadCOM(&buffer[1],31,&read);

    if ( read0 )
        *read0 = read + 1;

    UBYTE chck = GenerateCheckSum(buffer,31,true);
    if ( chck != buffer[31] )
        return false;

    m_lastPortsDone = buffer[6];
    m_lastPortsDone <<= 4;
    m_lastPortsDone += buffer[5];
    m_lastPortsDone <<= 4;
    m_lastPortsDone += buffer[4];
    m_lastPortsDone <<= 4;
    m_lastPortsDone += buffer[3];
    m_lastPortsDone <<= 4;
    m_lastPortsDone += buffer[2];
    m_lastPortsDone <<= 4;
    m_lastPortsDone += buffer[1];

    for(int i=0;i<24;i++)
        m_lastPortsMotion[i] = buffer[7+i];

    return true;
}


bool RCB3Interface::WaitForSignal()
{
    if ( m_comPort->needSignal() )
    {
        int  retry = MAX_RCB_RETRY;
        while( retry != 0 )
        {
            int written = 0;
            char signal = 0xD;
            m_comPort->WriteCOM(&signal,1,&written);
            if ( written == 1 )
            {
                char answer;
                int read = 0;
                m_comPort->ReadCOM(&answer,1,&read);

                if ( m_receivingNotification )
                {
                    UBYTE first_byte = answer;
                    switch(first_byte)
                    {
                    case 0xFD:
                        ReadRemoteControlData();
                        retry++; //Allow the retry because we just got a notification
                        break;
                    case 0xFE:
                        ReadAnalogData();
                        retry++; //Allow the retry because we just got a notification
                        break;
                    case 0xFF:
                        ReadPortsData();
                        retry++;  //Allow the retry because we just got a notification
                        break;
                    }
                }

                if (( read == 1 ) && (answer == signal))
                {
                    return true;
                }
            }
            retry--;
        }
        return false;
    }

    return true;
}

bool RCB3Interface::SendCommand(UBYTE *command,UINT size)
{
    if ( !WaitForSignal() )
        return false;

    m_lastCommand = command[0];
    int written = 0;
    m_comPort->WriteCOM(command,size,&written);
    if ( written != size )
        return false;
    return true;
}

bool RCB3Interface::ReadAnswer(UBYTE *command,UINT size)
{
    int read = 0;
    if ( m_receivingNotification )
    {
        m_comPort->ReadCOM(command,1,&read);
        if ( read != 1 )
            return false;

        if ( m_possibleCollision[m_lastCommand] )
        {
            UBYTE buffer[512];
            buffer[0] = command[0];
            bool valid = false;
            if ( command[0] >= 0xFD )
            {
                switch(command[0])
                {
                case 0xFD:
                    valid = ReadRemoteControlData(buffer,&read);
                    break;
                case 0xFE:
                    valid = ReadAnalogData(buffer,&read);
                    break;
                case 0xFF:
                    valid = ReadPortsData(buffer,&read);
                    break;
                }

                if ( !valid )
                {
                    if ( (UINT)read <= size )
                    {
                        //we have not read enough 
                        memcpy(command,buffer,read);
                        int read2;
                        m_comPort->ReadCOM(command+read,size-read,&read2);
                        read += read2;
                        if ( read != size-1 )
                            return false;
                        return true;
                    }
                    else
                    {
                        //there is more than necessary.
                        memcpy(command,buffer,size);
                        if ( buffer[size] >= 0xFD )
                        {
                            //Check if we have a notification at the end.
                            switch(buffer[size])
                            {
                            case 0xFD:
                                valid = ReadRemoteControlData();
                                break;
                            case 0xFE:
                                valid = ReadAnalogData();
                                break;
                            case 0xFF:
                                valid = ReadPortsData();
                                break;
                            }
                        }

                        return true;
                    }
                }
                //else use the code at the end of the function to read the real message.
            }
            else
            {
                m_comPort->ReadCOM(command+1,size-1,&read);
                if ( read != size-1 )
                    return false;
                return true;
            }
        }
        else
        {
            if ( command[0] >= 0xFD )
            {
                switch(command[0])
                {
                case 0xFD:
                    ReadRemoteControlData();
                    break;
                case 0xFE:
                    ReadAnalogData();
                    break;
                case 0xFF:
                    ReadPortsData();
                    break;
                }
            }
            else
            {
                m_comPort->ReadCOM(command+1,size-1,&read);
                if ( read != size-1 )
                    return false;
                return true;
            }
        }
    }

    m_comPort->ReadCOM(command,size,&read);
    if ( read != size )
        return false;
    return true;
}

