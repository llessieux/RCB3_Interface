/*
 * File:         rcb3_interface.cpp
 * Date:         January, 2007
 * Description:  Handles the communication with the RCB3
 * Author:       Laurent Lessieux
 *
 * Copyright (c) 2006 Laurent Lessieux
 *               www.lessieux.com
 */

#include "stdafx.h"
#include "RCB3_interface.h"
#include <vector>
#include <memory>
#include "com.h"
#include "rcb3.h"
#include "Khr2_data.h"


#define RCB_FORCE_ACK 1
#define checkForNullPointer(variable) \
if ( variable == NULL )\
{\
    char message[512];\
    sprintf_s(message,#variable" : NULL POINTER\n");\
    SetError(RCB3_INVALID_PARAMETER,message);\
    return false;\
}

#define receiveAck() \
UBYTE ack;\
if ( !rcb->ReadAnswer(&ack,1) )\
{\
    SetError(RCB3_COM_READ_FAILED,NULL);\
    return false;\
}\
\
if ( ack != 0x6 )\
    return false;

#define receiveAnswerAndCheck(size) \
    UBYTE answer[size];\
    if ( !rcb->ReadAnswer(answer,size) )\
    {\
        SetError(RCB3_COM_READ_FAILED,NULL);\
        return false;\
    }\
\
    UBYTE chck = GenerateCheckSum(answer,size-1);\
    if ( chck != answer[size-1] )\
    {\
        SetError(RCB_INVALID_CHECKSUM_RECEIVED,NULL);\
        return false;\
    }

#define GenerateCHKSUMAndSend(size) \
    command[size-1] = GenerateCheckSum(command,size-1);\
    if ( !rcb->SendCommand(command,size) )\
    {\
        SetError(RCB3_COM_WRITE_FAILED,NULL);\
        return false;\
    }

#define ClearErrorAndReturn() \
    g_lastError = RCB3_NO_ERROR;\
    return true;


char *g_errorNames[] =
{
    "NO ERROR",
    "Cannot open COM Port",
    "Failed to write to COM Port",
    "Failed to read from COM Port",
    "Memory allocation failure",
    "Invalid RCB3 Interface token",
    "Invalid Port",
    "Invalid Checksum received",
    "Invalid Scenario Index",
    "Invalid Motion Index",
    "Invalid Slot Index",
    "Invalid Parameter",    
    "No Response from RCB3",
    "Function not supported on this Model",
    NULL
};

UINT g_maxErrorNameIndex = RCB3_LAST_ERROR;
char g_fullErrorMessage[512];
UINT g_lastError = RCB3_NO_ERROR;
int g_answer[256];

void SetError(UINT error,char *full_message )
{
    g_lastError = error;
    if ( full_message != NULL )
        strcpy_s(g_fullErrorMessage,512,full_message);
    else
    {
        memcpy(g_fullErrorMessage,g_errorNames[error],strlen(g_errorNames[error])+1);
    }
}

UBYTE GenerateCheckSum(UBYTE *commands,UINT size,bool sevenbitMask = false)
{
    UINT checksum = 0;
    for(UINT i=0;i<size;i++)
        checksum += commands[i];
    if ( sevenbitMask )
        checksum &= 0x7F;
    else
        checksum &= 0xFF;

    return checksum;
}

std::vector<RCB3Interface *> g_rcbs;

RCB3Interface *GetRCB(UINT index)
{
    static bool init = true;
    if ( init )
    {
        for(int i=0;i<256;i++)
            g_answer[i] = 0;

        g_answer[0x00E2] = 1;
        g_answer[0x00E1] = 19;
        g_answer[0x00F2] = 1;
        g_answer[0x00F1] = 3;
        g_answer[0x00F4] = 1;
        g_answer[0x00F3] = 1;
        g_answer[0x00FE] = 1;
        g_answer[0x00FD] = 1;
        g_answer[0x00FC] = 50;
        g_answer[0x00FB] = 1;
        g_answer[0x00FA] = 1;
        g_answer[0x00F9] = 49;
        g_answer[0x00F0] = 49;
        g_answer[0x00F8] = 1;
        g_answer[0x00F7] = 2;
        g_answer[0x00EF] = 1;
        g_answer[0x00EE] = 3;
        g_answer[0x00EC] = 1;
        g_answer[0x00EB] = 4;
        g_answer[0x00EA] = 1;
        g_answer[0x00E9] = 4;
        g_answer[0x00FF] = 65;
        g_answer[0x00E8] = 9;
        g_answer[0x00E7] = 1;
        g_answer[0x00E6] = 1;
        g_answer[0x00E5] = 7;
        g_answer[0x00E4] = 1;
        g_answer[0x00E1] = 25;
        g_answer[0x00ED] = 8;
        g_answer[0x00DE] = 1;
        g_answer[0x00E0] = 1;
        g_answer[0x00DF] = 5;
        g_answer[0x00DD] = 1;
        g_answer[0x00DC] = 4;
        g_answer[0x00DB] = 1;
        g_answer[0x00DA] = 18;
        g_answer[0x00D9] = 1;
        g_answer[0x00D8] = 30;
        g_answer[0x00CF] = 25;
        g_answer[0x00CE] = 25;
        g_answer[0x00D7] = 1;
        g_answer[0x00D6] = 13;
        g_answer[0x00D5] = 1;
        g_answer[0x00D4] = 4;
        g_answer[0x00D3] = 1;
        g_answer[0x00D2] = 49;
        g_answer[0x00D1] = 1;
        g_answer[0x00D0] = 33;
        g_answer[0x00F6] = 1;
        g_answer[0x00F5] = 43;
        g_answer[0x00FF] = 65;

        init = false;
    }

    if ( index >= g_rcbs.size() )
    {
        SetError(RCB3_INVALID_INTERFACE,NULL);
        return NULL;
    }

    if ( g_rcbs[index] )
    {
        return g_rcbs[index];
    }

    SetError(RCB3_INVALID_INTERFACE,NULL);
    return NULL;
}

RCB_EXPORT bool CreateRCB3Interface(int in_comPort,int in_comSpeed,int in_model,UINT &out_rcb3Interface)
{
    RCB3Interface *rcb3 = new RCB3Interface(in_comPort,in_comSpeed);
    if ( rcb3 == NULL )
    {
        SetError(RCB3_NOT_ENOUGH_MEMORY,NULL);
        return false;
    }
    rcb3->m_model = in_model;
    if ( rcb3->m_comPort == NULL )
    {
        SetError(RCB3_NOT_ENOUGH_MEMORY,NULL);
        return false;
    }

    if ( rcb3->m_comPort->IsOK() )
    {
        unsigned int i=0;
        bool added  = false;
        while(i<g_rcbs.size())
        {
            if ( g_rcbs[i] == NULL )
            {
                g_rcbs[i] = rcb3;
                out_rcb3Interface = i;

                char version[65];
                if (!GetRCBVersion(i,version) )
                {
                    delete rcb3;
                    g_rcbs[i] = NULL;
                    char message[512];
                    sprintf_s(message,"No Response from the RCB3 on port %d : \n",in_comPort);
                    SetError(RCB3_NO_RESPONSE,message);
                    return false;
                }

                ClearErrorAndReturn();
            }
        }
        g_rcbs.push_back(rcb3);
        out_rcb3Interface = (UINT)(g_rcbs.size()-1);

        char version[65];
        if (!GetRCBVersion(out_rcb3Interface,version) )
        {
            delete rcb3;
            g_rcbs[i] = NULL;
            char message[512];
            sprintf_s(message,"No Response from the RCB3 on port %d : \n",in_comPort);
            SetError(RCB3_NO_RESPONSE,message);
            return false;
        }
        ClearErrorAndReturn();
    }

    char message[512];
    sprintf_s(message,"Failed to open port %d : \n",in_comPort);
    SetError(RCB3_CANNOT_OPEN_COM,message);
    delete rcb3;
    return false;
}

RCB_EXPORT bool CreateRCB3InterfaceSkipCheck(int in_comPort,int in_comSpeed,int in_model,UINT &out_rcb3Interface)
{
    RCB3Interface *rcb3 = new RCB3Interface(in_comPort,in_comSpeed);
    if ( rcb3 == NULL )
    {
        SetError(RCB3_NOT_ENOUGH_MEMORY,NULL);
        return false;
    }
    rcb3->m_model = in_model;
    if ( rcb3->m_comPort == NULL )
    {
        SetError(RCB3_NOT_ENOUGH_MEMORY,NULL);
        return false;
    }

    if ( rcb3->m_comPort->IsOK() )
    {
        unsigned int i=0;
        bool added  = false;
        while(i<g_rcbs.size())
        {
            if ( g_rcbs[i] == NULL )
            {
                g_rcbs[i] = rcb3;
                out_rcb3Interface = i;
                ClearErrorAndReturn();
            }
        }
        g_rcbs.push_back(rcb3);
        out_rcb3Interface = (UINT)(g_rcbs.size()-1);
        ClearErrorAndReturn();
    }

    char message[512];
    sprintf_s(message,"Failed to open port %d : \n",in_comPort);
    SetError(RCB3_CANNOT_OPEN_COM,message);
    delete rcb3;
    return false;
}

RCB_EXPORT bool CreateRCB3InterfaceViaBlueSmirf(
    int in_comPort,int in_comSpeed,int in_rcbmodel,
    int in_autodetectSpeed,int in_writeGoodConfigToFlash,int in_skipCheck,UINT &out_rcb3Interface)
{
    RCB3Interface *rcb3 = new RCB3Interface(in_comPort,in_comSpeed,(in_autodetectSpeed != 0),(in_writeGoodConfigToFlash!=0));
    if ( rcb3 == NULL )
    {
        SetError(RCB3_NOT_ENOUGH_MEMORY,NULL);
        return false;
    }
    rcb3->m_model = in_rcbmodel;
    if ( rcb3->m_comPort == NULL )
    {
        SetError(RCB3_CANNOT_OPEN_COM,NULL);
        return false;
    }

    if ( rcb3->m_comPort->IsOK() )
    {
        unsigned int i=0;
        bool added  = false;
        while(i<g_rcbs.size())
        {
            if ( g_rcbs[i] == NULL )
            {
                g_rcbs[i] = rcb3;
                out_rcb3Interface = i;

                if ( in_skipCheck != 0 )
                {
                    char version[65];
                    if (!GetRCBVersion(i,version) )
                    {
                        delete rcb3;
                        g_rcbs[i] = NULL;
                        char message[512];
                        sprintf_s(message,"No Response from the RCB3 on port %d : \n",in_comPort);
                        SetError(RCB3_NO_RESPONSE,message);
                        return false;
                    }
                }
                ClearErrorAndReturn();
            }
        }
        g_rcbs.push_back(rcb3);
        out_rcb3Interface = (UINT)(g_rcbs.size()-1);

        if ( in_skipCheck == 0 )
        {
            char version[65];
            if (!GetRCBVersion(out_rcb3Interface,version) )
            {
                delete rcb3;
                g_rcbs[i] = NULL;
                char message[512];
                sprintf_s(message,"No Response from the RCB3 on port %d : \n",in_comPort);
                SetError(RCB3_NO_RESPONSE,message);
                return false;
            }
        }
        ClearErrorAndReturn();
    }

    char message[512];
    sprintf_s(message,"Failed to open port %d : \n",in_comPort);
    SetError(RCB3_CANNOT_OPEN_COM,message);
    delete rcb3;
    return false;

}

RCB_EXPORT bool CreateRCB3InterfaceViaEZWism(
    char *in_address,int in_port,int in_rcbmodel,int in_skipCheck,UINT &out_rcb3Interface)
{
    RCB3Interface *rcb3 = new RCB3Interface(in_address,in_port);
    if ( rcb3 == NULL )
    {
        SetError(RCB3_NOT_ENOUGH_MEMORY,NULL);
        return false;
    }
    rcb3->m_model = in_rcbmodel;
    if ( rcb3->m_comPort == NULL )
    {
        SetError(RCB3_CANNOT_OPEN_COM,NULL);
        return false;
    }

    if ( rcb3->m_comPort->IsOK() )
    {
        unsigned int i=0;
        bool added  = false;
        while(i<g_rcbs.size())
        {
            if ( g_rcbs[i] == NULL )
            {
                g_rcbs[i] = rcb3;
                out_rcb3Interface = i;

                if ( in_skipCheck != 0 )
                {
                    char version[65];
                    if (!GetRCBVersion(i,version) )
                    {
                        delete rcb3;
                        g_rcbs[i] = NULL;
                        char message[512];
                        sprintf_s(message,"No Response from the RCB3 on address(port) %s : %d : \n",in_address,in_port);
                        SetError(RCB3_NO_RESPONSE,message);
                        return false;
                    }
                }
                ClearErrorAndReturn();
            }
        }
        g_rcbs.push_back(rcb3);
        out_rcb3Interface = (UINT)(g_rcbs.size()-1);

        if ( in_skipCheck == 0 )
        {
            char version[65];
            if (!GetRCBVersion(out_rcb3Interface,version) )
            {
                delete rcb3;
                g_rcbs[i] = NULL;
                char message[512];
                sprintf_s(message,"No Response from the RCB3 on address(port) %s : %d : \n",in_address,in_port);
                SetError(RCB3_NO_RESPONSE,message);
                return false;
            }
        }
        ClearErrorAndReturn();
    }

    char message[512];
    sprintf_s(message,"Failed to open port on address %s : %d : \n",in_address,in_port);
    SetError(RCB3_CANNOT_OPEN_COM,message);
    delete rcb3;
    return false;

}

RCB_EXPORT bool DestroyRCB3Interface(UINT in_rcb3Interface)
{
    if ( in_rcb3Interface >= g_rcbs.size() )
    {
        SetError(RCB3_INVALID_INTERFACE,NULL);
        return false;
    }

    if ( g_rcbs[in_rcb3Interface] )
    {
        delete g_rcbs[in_rcb3Interface];
        g_rcbs[in_rcb3Interface] = NULL;
        ClearErrorAndReturn();
    }

    SetError(RCB3_INVALID_INTERFACE,NULL);
    return false;
}

RCB_EXPORT UINT GetLastRCB3Error()
{
    return g_lastError;
}

RCB_EXPORT char* GetLastRCB3ErrorName(UINT error)
{
    if ( error > g_maxErrorNameIndex)
        return NULL;

    return g_errorNames[error];
}

RCB_EXPORT char* GetLastRCB3FullErrorMessage()
{
    if ( g_lastError == RCB3_NO_ERROR )
        return NULL;

    return g_fullErrorMessage;
}

//value of 0 or 1 for each. Usage depends on command too.
RCB_EXPORT UINT RCB3MakeOption(UINT storeToEeprom,UINT forceMotion)
{
    UINT option = 0;
    if ( storeToEeprom)
        option |= RCB_STORE_TO_EEPROM;
    if ( forceMotion )
        option |= RCB_FORCE_MOTION;

    return option;
}


RCB_EXPORT bool SetMotionPlaybackforAD(UINT in_rcb3,const UINT *in_motionScenarioIndices/*6*/,
                                                    const UINT *in_levels/*6*/,
                                                    const UINT *in_reverse/*6*/,
                                                    const UINT *in_mixingSwitch/*6*/,
                                                    UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(in_levels);
    checkForNullPointer(in_reverse);
    checkForNullPointer(in_mixingSwitch);

    in_option &= RCB_STORE_TO_EEPROM; //Only ROM and ACK valid
    in_option |= RCB_FORCE_ACK; //force the ACK

    UBYTE command[21];
    command[0] = 0xE2;
    command[1] = in_option;
    try
    {
        for(int i=0;i<6;i++)
        {
            UINT motion = in_motionScenarioIndices[i];
            if ( motion > 84 )
            {
                char message[512];
                sprintf_s(message,"Invalid Motion Index (=%d) at position %d for SetMotionPlaybackforAD\n",motion,i);
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }
            command[2+i] = motion;
        }

        for(int i=0;i<6;i++)
        {
            UINT level = in_levels[i];
            if ( level > 1023 )
            {
                char message[512];
                sprintf_s(message,"Invalid Level (=%d) at position %d for SetMotionPlaybackforAD: Should be lower than 1024\n",level,i);
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }
            command[8+i*2] = (level>>8);
            command[9+i*2] = level&0xFF;
        }

        for(int i=0;i<6;i++)
        {
            UINT reverse = in_reverse[i];
            if ( reverse > 1 )
            {
                char message[512];
                sprintf_s(message,"Invalid Reverse (=%d) at position %d for SetMotionPlaybackforAD: Should be 0 or 1\n",reverse,i);
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }
            command[8+i*2] |= (reverse<<6);
        }

        for(int i=0;i<6;i++)
        {
            UINT mixing = in_mixingSwitch[i];
            if ( mixing > 1 )
            {
                char message[512];
                sprintf_s(message,"Invalid MixingSwitch (=%d) at position %d for SetMotionPlaybackforAD: Should be 0 or 1\n",mixing,i);
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }
            command[8+i*2] |= (mixing<<7);
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while reading some parameters\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    GenerateCHKSUMAndSend(21);
    receiveAck();

    ClearErrorAndReturn();
}

RCB_EXPORT bool GetMotionPlaybackforAD(UINT in_rcb3,UINT in_option,
                                       UINT *out_motionScenarioIndices/*6*/,
                                       UINT *out_levels/*6*/,
                                       UINT *out_reverse/*6*/,
                                       UINT *out_mixingSwitch/*6*/
                                                    )
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(out_motionScenarioIndices);
    checkForNullPointer(out_levels);
    checkForNullPointer(out_reverse);
    checkForNullPointer(out_mixingSwitch);

    in_option &= RCB_STORE_TO_EEPROM; //Only ROM

    UBYTE command[3];
    command[0] = 0xE1;
    command[1] = in_option;

    GenerateCHKSUMAndSend(3);
    receiveAnswerAndCheck(19);

    try
    {
        for(int i=0;i<6;i++)
        {
            out_motionScenarioIndices[i] = answer[i];
        }

        for(int i=0;i<6;i++)
        {
            UINT level = answer[6+i*2];
            level <<= 8;
            level += answer[6+i*2] & 0x3;
            out_levels[i] = level;
        }

        for(int i=0;i<6;i++)
        {
            out_reverse[i] = (answer[6+i*2]>>6) & 1;
        }

        for(int i=0;i<6;i++)
        {
            out_mixingSwitch[i] = (answer[6+i*2]>>7) & 1;
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing some output\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    ClearErrorAndReturn();
}


//Software Switches
RCB_EXPORT bool SetRCB3SoftwareSwitch(UINT in_rcb3,UINT in_SWITCH,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    in_option |= RCB_FORCE_ACK; //force the ACK

    //TODO: Check if the notification are active or not
    UBYTE command[5];
    command[0] = 0xF2;
    command[1] = in_option;
    command[2] = (in_SWITCH>>8);
    command[3] = (in_SWITCH&0xFF);
    
    GenerateCHKSUMAndSend(5);
    receiveAck();

    if ( in_SWITCH & 0x304 )
        rcb->m_receivingNotification = true;
    else
        rcb->m_receivingNotification = false;

    ClearErrorAndReturn();
}

RCB_EXPORT bool GetRCB3SoftwareSwitch(UINT in_rcb3,UINT &out_SWITCH,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;
    return false;

    in_option &= RCB_STORE_TO_EEPROM;

    UBYTE command[3];
    command[0] = 0xF1;
    command[1] = in_option;
    
    GenerateCHKSUMAndSend(3);
    receiveAnswerAndCheck(3);

    out_SWITCH = (((UINT)answer[0]) << 8) + (UINT)answer[1];

    ClearErrorAndReturn();

}

//Motion/Scenario Playback
RCB_EXPORT bool PlayMotion(UINT in_rcb3,UINT in_motionIndex,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    UINT motion = in_motionIndex;
    if ( motion > 79 )
    {
        char message[512];
        sprintf_s(message,"Invalid Motion Index (=%d) PlayMotion\n",motion);
        SetError(RCB3_INVALID_MOTION_INDEX,message);
        return false;
    }

    in_option |= RCB_FORCE_ACK; //force ACK

    UBYTE command[4];
    command[0] = 0xF4;
    command[1] = in_option;
    command[2] = in_motionIndex;
    
    GenerateCHKSUMAndSend(4);
    receiveAck();

    ClearErrorAndReturn();
}

RCB_EXPORT bool PlayScenario(UINT in_rcb3,UINT in_scenarioIndex,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;
    UINT scenario = in_scenarioIndex;
    if ( scenario > 4 )
    {
        char message[512];
        sprintf_s(message,"Invalid Scenario Index (=%d) PlayScenario\n",scenario);
        SetError(RCB3_INVALID_SCENARIO_INDEX,message);
        return false;
    }

    in_option |= RCB_FORCE_ACK; //force ACK

    UBYTE command[4];
    command[0] = 0xF4;
    command[1] = in_option;
    command[2] = 80+scenario;
    
    GenerateCHKSUMAndSend(4);
    receiveAck();
    ClearErrorAndReturn();
}

//Interruption of port motion (one bit set per port to stop)
RCB_EXPORT bool StopPorts(UINT in_rcb3,UINT in_portsToStop)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    UBYTE command[6];
    command[0] = 0xF3;
    command[1] = 1; //force ACK
    command[2] = in_portsToStop&0xFF;
    command[3] = (in_portsToStop>>8 ) & 0xFF;
    command[4] = (in_portsToStop>>16) & 0xFF;    
    
    GenerateCHKSUMAndSend(6);
    receiveAck();
    ClearErrorAndReturn();
}

//Set Port Speed and Position
RCB_EXPORT bool SetPortSpeedAndPosition(UINT in_rcb3,UINT in_portIndex,UINT in_speed,UINT in_position,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;
    in_option |= RCB_FORCE_ACK; //force ACK

    if ( in_portIndex > 23 )
    {
        char message[512];
        sprintf_s(message,512,"PortIndex(%d) outside valid range: Should be between [0..23]\n",in_portIndex);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_speed == 0 || in_speed > 255 )
    {
        char message[512];
        sprintf_s(message,512,"Speed (%d) outside valid range: Should be between [1..255]\n",in_speed);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    UBYTE command[7];
    command[0] = 0xFE;
    command[1] = in_option;
    command[2] = in_portIndex;
    command[3] = in_speed;
    command[4] = (in_position>>8) & 0xFF;    
    command[5] = in_position & 0xFF;
    
    GenerateCHKSUMAndSend(7);
    receiveAck();  //0xFE do not ack
    ClearErrorAndReturn();
}

RCB_EXPORT bool SetPortsSpeedAndPosition(UINT in_rcb3,UINT in_motionIndex,UINT in_slotIndex,UINT in_speed,const UINT *in_position/*24*/,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(in_position);

    in_option |= RCB_FORCE_ACK; //force ACK

    if ( in_motionIndex > 79 )
    {
        char message[512];
        sprintf_s(message,512,"MotionIndex (%d) outside valid range: Should be between [0..79]\n",in_motionIndex);
        SetError(RCB3_INVALID_MOTION_INDEX,message);
        return false;
    }

    if ( in_speed == 0 || in_speed > 255 )
    {
        char message[512];
        sprintf_s(message,512,"Speed (%d) outside valid range: Should be between [1..255]\n",in_speed);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }
    /*Commented because in fact, you can store more than 30 slots, it's just taking the space of the next scenarios.
    if ( in_option & RCB_STORE_TO_EEPROM )
    {
        if ( in_slotIndex > 29 )
        {
            char message[512];
            sprintf_s(message,512,"Slot Index (%d) outside valid range: Should be between [0..29]\n",in_slotIndex);
            SetError(RCB3_INVALID_SLOT_INDEX,message);
            return false;
        }
    }*/

    UBYTE command[54];
    command[0] = 0xFD;
    command[1] = in_option;
    command[2] = in_motionIndex;
    command[3] = in_slotIndex;
    command[4] = in_speed;
    try
    {
        for(UINT i=0;i<24;i++)
        {
            command[5+i*2] = (in_position[i]>>8) & 0xFF;    
            command[6+i*2] = in_position[i] & 0xFF;
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while reading in_position variable in SetPortsSpeedAndPosition\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    GenerateCHKSUMAndSend(54);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetPortsSpeedAndPosition(UINT in_rcb3,UINT in_option,UINT in_motionIndex,UINT in_slotIndex,UINT &out_speed,UINT *out_position/*24*/)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(out_position);
    
    if ( in_motionIndex > 79 )
    {
        char message[512];
        sprintf_s(message,512,"MotionIndex (%d) outside valid range: Should be between [0..79]\n",in_motionIndex);
        SetError(RCB3_INVALID_MOTION_INDEX,message);
        return false;
    }

    /*Commented because in fact, you can store more than 30 slots, it's just taking the space of the next scenarios.
    /*if ( in_option & RCB_STORE_TO_EEPROM )
    {
        if ( in_slotIndex > 29 )
        {
            char message[512];
            sprintf_s(message,512,"Slot Index (%d) outside valid range: Should be between [0..29]\n",in_slotIndex);
            SetError(RCB3_INVALID_SLOT_INDEX,message);
            return false;
        }
    }*/

    UBYTE command[5];
    command[0] = 0xFC;
    command[1] = in_option;
    command[2] = in_motionIndex;
    command[3] = in_slotIndex;
    
    GenerateCHKSUMAndSend(5);
    receiveAnswerAndCheck(50);

    out_speed = answer[0];
    try
    {
        for(UINT i=0;i<24;i++)
        {
            out_position[i] = (((UINT)answer[1+i*2]) << 8) + (UINT)answer[2+i*2];
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing out_position variable in GetPortsSpeedAndPosition\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    ClearErrorAndReturn();
}


RCB_EXPORT bool SetPortHomePosition(UINT in_rcb3,UINT in_portIndex,UINT in_homePosition,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;
    in_option |= RCB_FORCE_ACK; //force ACK

    if ( in_portIndex > 23 )
    {
        char message[512];
        sprintf_s(message,512,"PortIndex(%d) outside valid range: Should be between [0..23]\n",in_portIndex);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    UBYTE command[6];
    command[0] = 0xFB;
    command[1] = in_option;
    command[2] = in_portIndex;
    command[3] = (in_homePosition>>8) & 0xFF;    
    command[4] = in_homePosition & 0xFF;
    
    GenerateCHKSUMAndSend(6);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool SetPortsHomePosition(UINT in_rcb3,const UINT *in_homePositions/*24*/,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(in_homePositions);

    in_option |= RCB_FORCE_ACK; //force ACK

    UBYTE command[51];
    command[0] = 0xFA;
    command[1] = in_option;
    try
    {
        for(UINT i=0;i<24;i++)
        {
            command[2+i*2] = (in_homePositions[i]>>8) & 0xFF;    
            command[3+i*2] = in_homePositions[i] & 0xFF;
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while reading in_homePositions variable in SetPortsHomePosition\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    GenerateCHKSUMAndSend(51);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetPortsHomePosition(UINT in_rcb3,UINT *out_homePositions/*24*/,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;
    
    checkForNullPointer(out_homePositions);

    UBYTE command[3];
    command[0] = 0xF9;
    command[1] = in_option;
    
    GenerateCHKSUMAndSend(3);
    receiveAnswerAndCheck(49);

    try
    {
        for(UINT i=0;i<24;i++)
        {
            out_homePositions[i] = (((UINT)answer[i*2]) << 8) + (UINT)answer[1+i*2];
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing out_homePositions variable in GetPortsHomePosition\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    ClearErrorAndReturn();
}


//Teaching
RCB_EXPORT bool GetTeachedPosition(UINT in_rcb3,UINT *out_positions/*24*/)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(out_positions);

    UBYTE command[3];
    command[0] = 0xF0;
    
    GenerateCHKSUMAndSend(2);
    receiveAnswerAndCheck(49);

    try
    {
        for(UINT i=0;i<24;i++)
        {
            out_positions[i] = (((UINT)answer[i*2]) << 8) + (UINT)answer[1+i*2];
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing out_positions variable in GetTeachedPosition\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    ClearErrorAndReturn();
}


//Scenario Edition
RCB_EXPORT bool SetMotionInScenario(UINT in_rcb3,UINT in_scenarioIndex,UINT in_slotIndex,UINT in_motionIndex,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    if ( in_motionIndex > 79 )
    {
        char message[512];
        sprintf_s(message,512,"MotionIndex (%d) outside valid range: Should be between [0..79]\n",in_motionIndex);
        SetError(RCB3_INVALID_MOTION_INDEX,message);
        return false;
    }

    UINT scenario = in_scenarioIndex;
    if ( scenario > 4 )
    {
        char message[512];
        sprintf_s(message,"ScenarioIndex (%d) outside valid range: Should be between [0..4]\n",scenario);
        SetError(RCB3_INVALID_SCENARIO_INDEX,message);
        return false;
    }

    if ( in_slotIndex > 200 )
    {
        char message[512];
        sprintf_s(message,512,"Slot Index (%d) outside valid range: Should be between [0..199]\n",in_slotIndex);
        SetError(RCB3_INVALID_SLOT_INDEX,message);
        return false;
    }

    in_option |= RCB_FORCE_ACK; //force ACK

    UBYTE command[6];
    command[0] = 0xF8;
    command[1] = in_option;
    command[2] = scenario;   
    command[3] = in_slotIndex;
    command[4] = in_motionIndex;
    
    GenerateCHKSUMAndSend(6);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetMotionInScenario(UINT in_rcb3,UINT in_scenarioIndex,UINT in_slotIndex,UINT &out_motionIndex)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    UINT scenario = in_scenarioIndex;
    if ( scenario > 4 )
    {
        char message[512];
        sprintf_s(message,"ScenarioIndex (%d) outside valid range: Should be between [0..4]\n",scenario);
        SetError(RCB3_INVALID_SCENARIO_INDEX,message);
        return false;
    }

    if ( in_slotIndex > 200 )
    {
        char message[512];
        sprintf_s(message,512,"Slot Index (%d) outside valid range: Should be between [0..199]\n",in_slotIndex);
        SetError(RCB3_INVALID_SLOT_INDEX,message);
        return false;
    }

    UBYTE command[4];
    command[0] = 0xF7;
    command[1] = scenario;   
    command[2] = in_slotIndex;
    
    GenerateCHKSUMAndSend(4);
    receiveAnswerAndCheck(2);

    out_motionIndex = answer[0];

    ClearErrorAndReturn();
}


//Start switch/Power on Motion/Scenario
RCB_EXPORT bool SetStartupMotion(UINT in_rcb3,UINT in_motionScenarioIndexSwitchPressed,UINT in_motionScenarioIndexPowerOn,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    if ( in_motionScenarioIndexSwitchPressed > 84 )
    {
        char message[512];
        sprintf_s(message,512,"MotionScenarioIndex (%d) outside valid range: Should be between [0..84]\n",in_motionScenarioIndexSwitchPressed);
        SetError(RCB3_INVALID_MOTION_INDEX,message);
        return false;
    }
    if ( in_motionScenarioIndexPowerOn > 84 )
    {
        char message[512];
        sprintf_s(message,512,"MotionScenarioIndex (%d) outside valid range: Should be between [0..84]\n",in_motionScenarioIndexPowerOn);
        SetError(RCB3_INVALID_MOTION_INDEX,message);
        return false;
    }

    in_option |= RCB_FORCE_ACK; //force ACK

    UBYTE command[5];
    command[0] = 0xEF;
    command[1] = in_option;
    command[2] = in_motionScenarioIndexSwitchPressed;   
    command[3] = in_motionScenarioIndexPowerOn;
    
    GenerateCHKSUMAndSend(5);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetStartupMotion(UINT in_rcb3,UINT &out_motionScenarioIndexSwitchPressed,UINT &out_motionScenarioIndexPowerOn)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    UBYTE command[2];
    command[0] = 0xEE;
    
    GenerateCHKSUMAndSend(2);
    receiveAnswerAndCheck(3);

    out_motionScenarioIndexSwitchPressed = answer[0];
    out_motionScenarioIndexPowerOn = answer[1];

    ClearErrorAndReturn();
}


//Low Power Configuration (in V)
RCB_EXPORT bool SetLowPowerThresholdAndMotionOrScenario(UINT in_rcb3,float in_lowPower,UINT in_motionScenarioIndex,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    if ( in_motionScenarioIndex > 84 )
    {
        char message[512];
        sprintf_s(message,512,"MotionScenarioIndex (%d) outside valid range: Should be between [0..84]\n",in_motionScenarioIndex);
        SetError(RCB3_INVALID_MOTION_INDEX,message);
        return false;
    }

    int power_threshold = (int)(in_lowPower/0.01539f);
    if ( in_lowPower < 0 || power_threshold > 1023 )
    {
        char message[512];
        sprintf_s(message,512,"in_lowPower (%f) outside valid range: Should be between [0..15.7V]\n",in_lowPower);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    in_option |= RCB_FORCE_ACK; //force ACK

    UBYTE command[6];
    command[0] = 0xEC;
    command[1] = in_option;
    command[2] = in_motionScenarioIndex;   
    command[3] = power_threshold>>8;
    command[4] = power_threshold&0xFF;
    
    GenerateCHKSUMAndSend(6);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetLowPowerThresholdAndMotionOrScenario(UINT in_rcb3,float &out_lowPower,UINT &out_motionScenarioIndex)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;
    UBYTE command[2];
    command[0] = 0xEB;
    
    GenerateCHKSUMAndSend(2);
    receiveAnswerAndCheck(4);

    out_motionScenarioIndex = answer[0];
    int power = answer[1];
    power <<= 8;
    power += answer[2];

    out_lowPower = 0.01539f * (float)power;

    ClearErrorAndReturn();
}


RCB_EXPORT bool SetSerialServoPortUsage(UINT in_rcb3,UINT in_portsUsed,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    in_option |= RCB_FORCE_ACK; //force ACK
    if ( rcb->m_model == RCB3_MODEL_J )
    {
        SetError(RCB3_FUNCTION_NOT_SUPPORTED_ON_THIS_MODEL,NULL);
        return false;
    }

    UBYTE command[6];
    command[0] = 0xEA;
    command[1] = in_option;
    command[2] = in_portsUsed&0xFF;   
    command[3] = (in_portsUsed>>8)&0xFF;
    command[4] = (in_portsUsed>>16)&0xFF;
    
    GenerateCHKSUMAndSend(6);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetSerialServoPortUsage(UINT in_rcb3,UINT &out_portsUsed)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    if ( rcb->m_model == RCB3_MODEL_J )
    {
        SetError(RCB3_FUNCTION_NOT_SUPPORTED_ON_THIS_MODEL,NULL);
        return false;
    }

    UBYTE command[2];
    command[0] = 0xE9;
    
    GenerateCHKSUMAndSend(2);
    receiveAnswerAndCheck(4);

    out_portsUsed = answer[2];
    out_portsUsed <<= 8;
    out_portsUsed += answer[1];
    out_portsUsed <<= 8;
    out_portsUsed += answer[0];

    ClearErrorAndReturn();
}


//RCB3 version (last byte will always be zero
RCB_EXPORT bool GetRCBVersion(UINT in_rcb3,char *out_version/*65*/)
{
    checkForNullPointer(out_version);

    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    UBYTE command[2]; 
    command[0] = 0xFF;
    
    GenerateCHKSUMAndSend(2);
    //receiveAnswerAndCheck(65);
    const int size = 65;
    UBYTE answer[size];
    if ( !rcb->ReadAnswer(answer,size) )
    {
        SetError(RCB3_COM_READ_FAILED,NULL);
        return false;
    }

    UBYTE chck = GenerateCheckSum(answer,size-1);
    if ( chck != answer[size-1] )
    {
        SetError(RCB_INVALID_CHECKSUM_RECEIVED,NULL);
        return false;
    }

    try
    {
        memcpy(out_version,answer,64);
        answer[64] = 0;
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing to out_version buffer\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }
    
    ClearErrorAndReturn();
}


//Analog Input 
//outputs/inputs (In V)

RCB_EXPORT bool GetAnalogInputs(UINT in_rcb3,float &out_power,float &out_ad1,float &out_ad2,float &out_ad3)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    UBYTE command[2]; 
    command[0] = 0xE8;
    
    GenerateCHKSUMAndSend(2);
    receiveAnswerAndCheck(9);

    UINT power = (((UINT)answer[0])<<8) + answer[1];
    UINT ad1 = (((UINT)answer[2])<<8) + answer[3];
    UINT ad2 = (((UINT)answer[4])<<8) + answer[5];
    UINT ad3 = (((UINT)answer[6])<<8) + answer[7];

    out_power = (float)power * 0.01539f;
    out_ad1 = (float)ad1 * 0.00532f;
    out_ad2 = (float)ad2 * 0.00532f;
    out_ad3 = (float)ad3 * 0.00532f;
    
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetAnalogInputsRAW(UINT in_rcb3,UINT &out_power,UINT &out_ad1,UINT &out_ad2,UINT &out_ad3)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    UBYTE command[2]; 
    command[0] = 0xE8;
    
    GenerateCHKSUMAndSend(2);
    receiveAnswerAndCheck(9);

    UINT power = (((UINT)answer[0])<<8) + answer[1];
    UINT ad1 = (((UINT)answer[2])<<8) + answer[3];
    UINT ad2 = (((UINT)answer[4])<<8) + answer[5];
    UINT ad3 = (((UINT)answer[6])<<8) + answer[7];

    out_power = power;
    out_ad1 = ad1;
    out_ad2 = ad2;
    out_ad3 = ad3;
    
    ClearErrorAndReturn();
}

//Channel = 0,1 or 2
//Set to -1V for automatic detection but should not be done with ROM option
RCB_EXPORT bool SetAnalogBaseValue(UINT in_rcb3,UINT in_channel,float in_baseValue,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    if ( in_channel > 2 )
    {
        char message[512];
        sprintf_s(message,512,"in_channel (%f) outside valid range: Should be between [0..2]\n",in_channel);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

     
    if ((( in_baseValue < 0 || in_baseValue > 5.44 )&&(in_baseValue != -1.0f))||
        ( in_option & RCB_STORE_TO_EEPROM ) &&(in_baseValue == -1.0f))
    {
        char message[512];
        sprintf_s(message,512,"in_baseValue (%f) outside valid range: Should be between [0..5.44V] or -1.0 when storing in RAM for an automatic detection\n",in_baseValue);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    in_option |= RCB_FORCE_ACK;

    int value = (int)(in_baseValue / 0.00532f);
    if ( in_baseValue == -1.0f )
        value = 0xFFFF;

    UBYTE command[6]; 
    command[0] = 0xE7;
    command[1] = in_option;
    command[2] = in_channel;
    command[3] = value>>8;
    command[4] = value&0xFF;
    
    GenerateCHKSUMAndSend(6);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool SetAnalogBaseValues(UINT in_rcb3,const float *in_baseValue/*3*/,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(in_baseValue);

    try
    {
        for(int i=0;i<3;i++)
            if ((( in_baseValue[i] < 0 || in_baseValue[i] > 5.44 )&&(in_baseValue[i] != -1.0f))||
                ( in_option & RCB_STORE_TO_EEPROM ) &&(in_baseValue[i] == -1.0f))

            {
                char message[512];
                sprintf_s(message,512,"in_baseValue[i] (%d,%f) outside valid range: Should be between [0..5.44V]\n",i,in_baseValue[i]);
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while reading in_baseValue\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    in_option |= RCB_FORCE_ACK;

    int v0 = (int)(in_baseValue[0] / 0.00532f);
    int v1 = (int)(in_baseValue[1] / 0.00532f);
    int v2 = (int)(in_baseValue[2] / 0.00532f);
    if ( in_baseValue[0] == -1.0f )
        v0 = 0xFFFF;
    if ( in_baseValue[1] == -1.0f )
        v1 = 0xFFFF;
    if ( in_baseValue[2] == -1.0f )
        v2 = 0xFFFF;

    UBYTE command[9]; 
    command[0] = 0xE6;
    command[1] = in_option;
    
    command[2] = v0>>8;
    command[3] = v0&0xFF;

    command[4] = v1>>8;
    command[5] = v1&0xFF;

    command[6] = v2>>8;
    command[7] = v2&0xFF;

    GenerateCHKSUMAndSend(9);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetAnalogBaseValues(UINT in_rcb3,float *out_baseValue/*3*/,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(out_baseValue);

    in_option &= RCB_STORE_TO_EEPROM;

    UBYTE command[3]; 
    command[0] = 0xE5;
    command[1] = in_option;
    
    GenerateCHKSUMAndSend(3);
    receiveAnswerAndCheck(7);

    UINT ad1 = (((UINT)answer[0])<<8) + answer[1];
    UINT ad2 = (((UINT)answer[2])<<8) + answer[3];
    UINT ad3 = (((UINT)answer[4])<<8) + answer[5];

    try
    {
        out_baseValue[0] = (float)ad1 * 0.00532f;
        out_baseValue[1] = (float)ad2 * 0.00532f;
        out_baseValue[2] = (float)ad3 * 0.00532f;
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing to out_baseValue\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    ClearErrorAndReturn();
}

//Channel = 0,1 or 2
//Set to -1V for automatic detection but should not be done with ROM option
RCB_EXPORT bool SetAnalogBaseValueRAW(UINT in_rcb3,UINT in_channel,UINT in_baseValue,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    if ( in_channel > 2 )
    {
        char message[512];
        sprintf_s(message,512,"in_channel (%f) outside valid range: Should be between [0..2]\n",in_channel);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }
    
    in_option |= RCB_FORCE_ACK;

    int value = in_baseValue;

    UBYTE command[6]; 
    command[0] = 0xE7;
    command[1] = in_option;
    command[2] = in_channel;
    command[3] = value>>8;
    command[4] = value&0xFF;
    
    GenerateCHKSUMAndSend(6);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool SetAnalogBaseValuesRAW(UINT in_rcb3,const UINT *in_baseValue/*3*/,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(in_baseValue);

    try
    {
        int check = 0;
        for(int i=0;i<3;i++)
        {
            check += in_baseValue[i];
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while reading in_baseValue\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    in_option |= RCB_FORCE_ACK;

    int v0 = in_baseValue[0];
    int v1 = in_baseValue[1];
    int v2 = in_baseValue[2];

    UBYTE command[9]; 
    command[0] = 0xE6;
    command[1] = in_option;
    
    command[2] = v0>>8;
    command[3] = v0&0xFF;

    command[4] = v1>>8;
    command[5] = v1&0xFF;

    command[6] = v2>>8;
    command[7] = v2&0xFF;

    GenerateCHKSUMAndSend(9);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetAnalogBaseValuesRAW(UINT in_rcb3,UINT *out_baseValue/*3*/,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(out_baseValue);

    in_option &= RCB_STORE_TO_EEPROM;

    UBYTE command[3]; 
    command[0] = 0xE5;
    command[1] = in_option;
    
    GenerateCHKSUMAndSend(3);
    receiveAnswerAndCheck(7);

    UINT ad1 = (((UINT)answer[0])<<8) + answer[1];
    UINT ad2 = (((UINT)answer[2])<<8) + answer[3];
    UINT ad3 = (((UINT)answer[4])<<8) + answer[5];

    try
    {
        out_baseValue[0] = ad1;
        out_baseValue[1] = ad2;
        out_baseValue[2] = ad3;
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing to out_baseValue\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    ClearErrorAndReturn();
}


//RealTime Mixing
RCB_EXPORT bool SetRealTimeMixing(UINT in_rcb3,UINT in_channel,const UINT *in_mixingFactors/*24*/,const UINT *in_reverseFactor/*24*/,const UINT *in_set/*24*/,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(in_mixingFactors);
    checkForNullPointer(in_reverseFactor);
    checkForNullPointer(in_set);

    if ( in_channel > 2 )
    {
        char message[512];
        sprintf_s(message,512,"in_channel (%f) outside valid range: Should be between [0..2]\n",in_channel);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    in_option |= RCB_FORCE_ACK; //force the ACK

    UBYTE command[28];
    command[0] = 0xE4;
    command[1] = in_option;
    command[2] = in_channel;

    try
    {
        for(int i=0;i<24;i++)
        {
            UINT factor = in_mixingFactors[i];
            if ( factor > 63)
            {
                char message[512];
                sprintf_s(message,"Invalid mixing factor (=%d) at position %d for SetRealTimeMixing: Should be lower than 64\n",factor,i);
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }
            command[3+i] = factor;
        }

        for(int i=0;i<24;i++)
        {
            UINT reverse = in_reverseFactor[i];
            if ( reverse > 1 )
            {
                char message[512];
                sprintf_s(message,"Invalid Reverse (=%d) at position %d for SetRealTimeMixing: Should be 0 or 1\n",reverse,i);
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }
            command[3+i] |= (reverse<<6);
        }

        for(int i=0;i<24;i++)
        {
            UINT mixing = in_set[i];
            if ( mixing > 1 )
            {
                char message[512];
                sprintf_s(message,"Invalid MixingSwitch (=%d) at position %d for SetRealTimeMixing: Should be 0 or 1\n",mixing,i);
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }
            command[3+i] |= (mixing<<7);
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while reading some parameters\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    GenerateCHKSUMAndSend(28);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetRealTimeMixing(UINT in_rcb3,UINT in_channel,UINT *out_mixingFactors/*24*/,UINT *out_reverseFactor/*24*/,UINT *out_set/*24*/,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(out_mixingFactors);
    checkForNullPointer(out_reverseFactor);
    checkForNullPointer(out_set);

    if ( in_channel > 2 )
    {
        char message[512];
        sprintf_s(message,512,"in_channel (%f) outside valid range: Should be between [0..2]\n",in_channel);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    in_option &= RCB_STORE_TO_EEPROM; //Only ROM

    UBYTE command[4];
    command[0] = 0xE1;
    command[1] = in_option;
    command[2] = in_channel;
    
    GenerateCHKSUMAndSend(4);
    receiveAnswerAndCheck(25);

    try
    {
        for(int i=0;i<24;i++)
        {
            UINT factor = answer[i];
            out_mixingFactors[i] = factor & 0x3F;
            out_reverseFactor[i] = (answer[i]>>6) & 1;
            out_set[i] = (answer[i]>>7) & 1;
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing some output\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    ClearErrorAndReturn();
}


//Configuration of Motion playback on AD Deltas.
//0 or 1 to enable or disable motion playback
//RCB_EXPORT bool SetMotionPlaybackforADDeltas(UINT in_rcb3,UINT in_sc1AD1,UINT in_sc2AD1,UINT in_sc3AD2,UINT in_sc4AD2,UINT in_sc5AD3,UINT in_sc6AD3,

//Remote Control
RCB_EXPORT bool GetDataFromRemoteControl(UINT in_rcb3,char *out_remoteControl/*7*/)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(out_remoteControl);

    UBYTE command[2];
    command[0] = 0xED;
    
    GenerateCHKSUMAndSend(2);
    receiveAnswerAndCheck(8);

    try
    {
        memcpy(out_remoteControl,answer,7);
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing some output\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }


    ClearErrorAndReturn();
}

RCB_EXPORT bool SetDataFromRemoveControlOverwrite(UINT in_rcb3,UINT in_option,UINT in_button,const UINT *in_analogChannel/*4*/)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(in_analogChannel);

    in_option |= RCB_FORCE_ACK;

    UBYTE command[10];
    command[0] = 0xDE;
    command[1] = in_option;
    command[2] = 0x80;
    if ( in_button == 0xFFFF )
    {
        command[3] = 0xFF;
        command[4] = 0xFF;
    }
    else
    {
        command[3] = (in_button>>7) & 0x7F;
        command[4] = in_button&0x7F;
    }

    try
    {
        for(int i=0;i<4;i++)
        {
            if ( in_analogChannel[i] > 255 && in_analogChannel[i] != 255 )
            {
                char message[512];
                sprintf_s(message,"Invalid in_analogChannel (=%d) at position %d for SetDataFromRemoveControlOverwrite: Should be [0..127] or 255\n",in_analogChannel[i],i);
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }
            command[5+i] = in_analogChannel[i]&0xFF;
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while reading in_analogChannel\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    GenerateCHKSUMAndSend(10);
    receiveAck();
    ClearErrorAndReturn();
}


//Serial Extension
RCB_EXPORT bool SetSerialExtensionAnalogBaseValues(UINT in_rcb3,const UBYTE *in_baseValue/*4*/,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;
    in_option |= RCB_FORCE_ACK;

    checkForNullPointer(in_baseValue);

    UBYTE command[7];
    command[0] = 0xE0;
    command[1] = in_option;
    try
    {
        for(int i=0;i<4;i++)
        {
            if (( in_baseValue[i] > 127 && (in_option & RCB_STORE_TO_EEPROM)) ||
                ( in_baseValue[i] > 127 && in_baseValue[i] != 255 ))
            {
                char message[512];
                sprintf_s(message,"Invalid in_baseValue (=%d) at position %d for SetSerialExtensionAnalogBaseValues: Should be [0..127] or 255 if not Storing to ROM\n",in_baseValue[i],i);
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }

            command[2+i] = in_baseValue[i]&0xFF;
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while reading in_analogChannel\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    GenerateCHKSUMAndSend(7);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetSerialExtensionAnalogBaseValues(UINT in_rcb3,UINT in_option,UBYTE *out_baseValue/*4*/)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(out_baseValue);

    in_option &= RCB_STORE_TO_EEPROM; //Only ROM

    UBYTE command[3];
    command[0] = 0xDF;
    command[1] = in_option;
    
    GenerateCHKSUMAndSend(3);
    receiveAnswerAndCheck(5);

    try
    {
        memcpy(out_baseValue,answer,4);
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing out_baseValue\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    ClearErrorAndReturn();
}


//ICS Mode
//1 bit per port
RCB_EXPORT bool SetPortsInICSMode(UINT in_rcb3,UINT in_portsInICS,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    in_option |= RCB_FORCE_ACK; //force ACK

    UBYTE command[6];
    command[0] = 0xDD;
    command[1] = in_option;
    command[2] = in_portsInICS&0xFF;   
    command[3] = (in_portsInICS>>8)&0xFF;
    command[4] = (in_portsInICS>>16)&0xFF;
    
    GenerateCHKSUMAndSend(6);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetPortsInICSMode(UINT in_rcb3,UINT &out_portsInICS)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    UBYTE command[2];
    command[0] = 0xDC;
    
    GenerateCHKSUMAndSend(2);
    receiveAnswerAndCheck(4);

    out_portsInICS = answer[2];
    out_portsInICS <<= 8;
    out_portsInICS += answer[1];
    out_portsInICS <<= 8;
    out_portsInICS += answer[0];

    ClearErrorAndReturn();
}


//RedVersion Port configuration
RCB_EXPORT bool SetRedVersionConfiguration(UINT in_rcb3,UINT in_portNumber,UBYTE in_backup,const UBYTE *in_stretch/*3*/,
                                           const UBYTE *in_speed/*3*/,UINT in_pulsePunch,UINT in_deadBand,UINT in_damping,
                                           UINT in_protectionTime, UINT in_reverse, 
                                           UINT in_lowerPositionLimit,UINT in_upperPositionLimit,
                                           UINT in_freeTime)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;
    checkForNullPointer(in_stretch);
    checkForNullPointer(in_speed);

    if ( rcb->m_model == RCB3_MODEL_J )
    {
        SetError(RCB3_FUNCTION_NOT_SUPPORTED_ON_THIS_MODEL,NULL);
        return false;
    }

    if ( in_portNumber > 23 )
    {
        char message[512];
        sprintf_s(message,512,"in_portNumber(%d) outside valid range: Should be between [0..23]\n",in_portNumber);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    UBYTE command[20];
    command[0] = 0xDB;
    command[1] = in_portNumber;
    command[2] = in_backup;

    try
    {
        for(int i=0;i<3;i++)
        {
            UBYTE st = in_stretch[i];
            UBYTE sp = in_speed[i];
            if ( st > 5 || st == 0 )
            {
                char message[512];
                sprintf_s(message,512,"Stretch[%d] = %d outside valid range: Should be between [1..5]\n",i,st);
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }
            if ( sp > 3 || sp == 0 )
            {
                char message[512];
                sprintf_s(message,512,"Speed[%d] = %d outside valid range: Should be between [1..3]\n",i,sp);
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }

            if ( i == 0 )
            {
                command[3] = st;
                command[4] = sp;
            }
            else
            {
                command[13+i*2] = st;
                command[14+i*2] = sp;
            }
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while reading some parameters\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_pulsePunch > 3 || in_pulsePunch == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_pulsePunch = %d outside valid range: Should be between [1..3]\n",in_pulsePunch);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_deadBand > 5 || in_deadBand == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_deadBand = %d outside valid range: Should be between [1..5]\n",in_deadBand);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_damping > 2 || in_damping == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_damping = %d outside valid range: Should be 1 or 2\n",in_damping);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_protectionTime > 10 || in_protectionTime == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_protectionTime = %d outside valid range: Should be between [1..10]\n",in_protectionTime);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_reverse > 1 )
    {
        char message[512];
        sprintf_s(message,512,"in_reverse = %d outside valid range: Should be  0 or 1 \n",in_reverse);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_upperPositionLimit > 2300 || in_upperPositionLimit < 1700 )
    {
        char message[512];
        sprintf_s(message,512,"in_upperPositionLimit = %d outside valid range: Should be between [1700..2300]\n",in_upperPositionLimit);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_lowerPositionLimit > 1300 || in_lowerPositionLimit < 700 )
    {
        char message[512];
        sprintf_s(message,512,"in_lowerPositionLimit = %d outside valid range: Should be between [700..1300]\n",in_lowerPositionLimit);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    command[5] = in_pulsePunch;
    command[6] = in_deadBand;
    command[7] = in_damping;
    command[8] = in_protectionTime;
    command[9] = in_reverse;
    command[10] = in_upperPositionLimit>>8;
    command[11] = in_upperPositionLimit&0xFF;
    command[12] = in_lowerPositionLimit>>8;
    command[13] = in_lowerPositionLimit&0xFF;
    command[14] = in_freeTime;
    
    GenerateCHKSUMAndSend(20);
    receiveAck();
    ClearErrorAndReturn();
}


RCB_EXPORT bool GetRedVersionConfiguration(UINT in_rcb3,UINT in_portNumber,UBYTE &out_backup,UBYTE *out_stretch/*3*/,
                                           UBYTE *out_speed/*3*/,UINT &out_pulsePunch,UINT &out_deadBand,UINT &out_damping,
                                           UINT &out_protectionTime, UINT &out_reverse, 
                                           UINT &out_lowerPositionLimit,UINT &out_upperPositionLimit,
                                           UINT &out_freeTime)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;
    checkForNullPointer(out_stretch);
    checkForNullPointer(out_speed);

    if ( in_portNumber > 23 )
    {
        char message[512];
        sprintf_s(message,512,"in_portNumber(%d) outside valid range: Should be between [0..23]\n",in_portNumber);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    UBYTE command[3];
    command[0] = 0xDA;
    command[1] = in_portNumber;
    
    GenerateCHKSUMAndSend(3);
    receiveAnswerAndCheck(18);

    try
    {
        out_backup = answer[0];
        out_stretch[0] = answer[1];
        out_stretch[1] = answer[13];
        out_stretch[2] = answer[15];
        out_speed[0] = answer[2];
        out_speed[1] = answer[14];
        out_speed[2] = answer[16];

        out_pulsePunch = answer[3];
        out_deadBand = answer[4];
        out_damping = answer[5];
        out_protectionTime = answer[6];
        out_reverse = answer[7];
        out_lowerPositionLimit = (UINT)answer[11] + (((UINT)answer[10])<<8);
        out_upperPositionLimit = (UINT)answer[9] + (((UINT)answer[8])<<8);
        out_freeTime = answer[12];
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing some parameters\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    ClearErrorAndReturn();
}


//Serial Servo Port configuration

RCB_EXPORT bool SetSerialServoConfiguration(UINT in_rcb3,UINT in_portNumber,UBYTE in_backup,const UBYTE *in_stretch/*3*/,
                                           const UBYTE *in_speed/*3*/,UINT in_pulsePunch,UINT in_deadBand,
                                           UINT in_damping,UINT in_protectionTime, UINT in_reverse, 
                                           UINT in_lowerPositionLimit,UINT in_upperPositionLimit,
                                           UINT in_freeTime,UINT in_stretchSIO,UINT in_speedSIO,
                                           UINT in_offset,UINT in_ID, UINT in_p1,UINT in_p2, UINT in_n1, UINT in_n2)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;
    checkForNullPointer(in_stretch);
    checkForNullPointer(in_speed);

    if ( in_portNumber > 23 )
    {
        char message[512];
        sprintf_s(message,512,"in_portNumber(%d) outside valid range: Should be between [0..23]\n",in_portNumber);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( rcb->m_model == RCB3_MODEL_J )
    {
        SetError(RCB3_FUNCTION_NOT_SUPPORTED_ON_THIS_MODEL,NULL);
        return false;
    }

    UBYTE command[32];
    command[0] = 0xD9;
    command[1] = in_portNumber;
    command[2] = in_backup;

    try
    {
        for(int i=0;i<3;i++)
        {
            UBYTE st = in_stretch[i];
            UBYTE sp = in_speed[i];
            if ( st > 127 || st == 0 )
            {
                char message[512];
                sprintf_s(message,512,"Stretch[%d] = %d outside valid range: Should be between [1..127]\n",i,st);
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }
            if ( sp > 127 || sp == 0 )
            {
                char message[512];
                sprintf_s(message,512,"Speed[%d] = %d outside valid range: Should be between [1..127]\n",i,sp);
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }

            if ( i == 0 )
            {
                command[3] = st;
                command[4] = sp;
            }
            else
            {
                command[13+i*2] = st;
                command[14+i*2] = sp;
            }
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while reading some parameters\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_pulsePunch > 3 || in_pulsePunch == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_pulsePunch = %d outside valid range: Should be between [1..3]\n",in_pulsePunch);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_deadBand > 5 || in_deadBand == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_deadBand = %d outside valid range: Should be between [1..5]\n",in_deadBand);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_damping > 2 || in_damping == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_damping = %d outside valid range: Should be 1 or 2\n",in_damping);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_protectionTime > 10 || in_protectionTime == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_protectionTime = %d outside valid range: Should be between [1..10]\n",in_protectionTime);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_reverse > 1 )
    {
        char message[512];
        sprintf_s(message,512,"in_reverse = %d outside valid range: Should be  0 or 1 \n",in_reverse);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_upperPositionLimit > 11500 || in_upperPositionLimit < 8000 )
    {
        char message[512];
        sprintf_s(message,512,"in_upperPositionLimit = %d outside valid range: Should be between [8000..11500]\n",in_upperPositionLimit);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_lowerPositionLimit > 6000 || in_lowerPositionLimit < 3500 )
    {
        char message[512];
        sprintf_s(message,512,"in_lowerPositionLimit = %d outside valid range: Should be between [3500..6000]\n",in_lowerPositionLimit);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_stretchSIO > 127 || in_stretchSIO == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_stretchSIO = %d outside valid range: Should be between [1..127]\n",in_stretchSIO);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_speedSIO > 127 || in_speedSIO == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_speedSIO = %d outside valid range: Should be between [1..127]\n",in_speedSIO);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_offset > 250 || in_offset < 190 )
    {
        char message[512];
        sprintf_s(message,512,"in_offset = %d outside valid range: Should be between [190..250]\n",in_offset);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_freeTime > 255 || in_freeTime < 112 )
    {
        char message[512];
        sprintf_s(message,512,"in_freeTime = %d outside valid range: Should be between [112..255]\n",in_freeTime);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    command[5] = in_pulsePunch;
    command[6] = in_deadBand;
    command[7] = in_damping;
    command[8] = in_protectionTime;
    command[9] = in_reverse;
    command[10] = in_upperPositionLimit>>8;
    command[11] = in_upperPositionLimit&0xFF;
    command[12] = in_lowerPositionLimit>>8;
    command[13] = in_lowerPositionLimit&0xFF;
    command[14] = in_freeTime;
    command[19] = in_speedSIO;
    command[20] = in_stretchSIO;
    command[21] = in_p1>>8;
    command[22] = in_p1&0xFF;
    command[23] = in_p2>>8;
    command[24] = in_p2&0xFF;
    command[25] = in_n1>>8;
    command[26] = in_n1&0xFF;
    command[27] = in_n2>>8;
    command[28] = in_n2&0xFF;
    command[29] = in_offset;
    command[30] = in_ID;
    
    GenerateCHKSUMAndSend(32);
    receiveAck();
    ClearErrorAndReturn();
}


RCB_EXPORT bool GetSerialServoConfiguration(UINT in_rcb3,UINT in_portNumber,UBYTE &out_backup,UBYTE *out_stretch/*3*/,
                                           UBYTE *out_speed/*3*/,UINT &out_pulsePunch,UINT &out_deadBand,
                                           UINT &out_damping,UINT &out_protectionTime, UINT &out_reverse, 
                                           UINT &out_lowerPositionLimit,UINT &out_upperPositionLimit,
                                           UINT &out_freeTime,UINT &out_stretchSIO,UINT &out_speedSIO,
                                           UINT &out_offset,UINT &out_ID, UINT &out_p1,UINT &out_p2, UINT &out_n1, UINT &out_n2)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;
    checkForNullPointer(out_stretch);
    checkForNullPointer(out_speed);

    if ( in_portNumber > 23 )
    {
        char message[512];
        sprintf_s(message,512,"in_portNumber(%d) outside valid range: Should be between [0..23]\n",in_portNumber);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( rcb->m_model == RCB3_MODEL_J )
    {
        SetError(RCB3_FUNCTION_NOT_SUPPORTED_ON_THIS_MODEL,NULL);
        return false;
    }

    UBYTE command[3];
    command[0] = 0xD8;
    command[1] = in_portNumber;
    
    GenerateCHKSUMAndSend(3);
    receiveAnswerAndCheck(30);

    try
    {
        out_backup = answer[0];
        out_stretch[0] = answer[1];
        out_stretch[1] = answer[13];
        out_stretch[2] = answer[15];
        out_speed[0] = answer[2];
        out_speed[1] = answer[14];
        out_speed[2] = answer[16];

        out_pulsePunch = answer[3];
        out_deadBand = answer[4];
        out_damping = answer[5];
        out_protectionTime = answer[6];
        out_reverse = answer[7];
        out_lowerPositionLimit = (UINT)answer[11] + (((UINT)answer[10])<<8);
        out_upperPositionLimit = (UINT)answer[9] + (((UINT)answer[8])<<8);
        out_freeTime = answer[12];
        out_stretchSIO = answer[18];
        out_speedSIO = answer[17];
        out_offset = answer[27];
        out_ID = answer[28];
        out_p1 = (UINT)answer[20] + ((UINT)answer[19] << 8);
        out_p2 = (UINT)answer[22] + ((UINT)answer[21] << 8);
        out_n1 = (UINT)answer[24] + ((UINT)answer[23] << 8);
        out_n2 = (UINT)answer[26] + ((UINT)answer[25] << 8);
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing some parameters\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    ClearErrorAndReturn();
}


RCB_EXPORT bool GetSerialServoStretch(UINT in_rcb3,UINT *out_stretch/*24*/)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(out_stretch);

    if ( rcb->m_model == RCB3_MODEL_J )
    {
        SetError(RCB3_FUNCTION_NOT_SUPPORTED_ON_THIS_MODEL,NULL);
        return false;
    }

    UBYTE command[2];
    command[0] = 0xCF;
    
    GenerateCHKSUMAndSend(2);
    receiveAnswerAndCheck(25);

    try
    {
        for(int i=0;i<24;i++)
            out_stretch[i] = answer[i];
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing out_stretch variable\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    ClearErrorAndReturn();
}

RCB_EXPORT bool GetSerialServoSpeed(UINT in_rcb3,UINT *out_speed/*24*/)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(out_speed);
    if ( rcb->m_model == RCB3_MODEL_J )
    {
        SetError(RCB3_FUNCTION_NOT_SUPPORTED_ON_THIS_MODEL,NULL);
        return false;
    }

    UBYTE command[2];
    command[0] = 0xCE;
    
    GenerateCHKSUMAndSend(2);
    receiveAnswerAndCheck(25);

    try
    {
        for(int i=0;i<24;i++)
            out_speed[i] = answer[i];
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing out_speed variable\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    ClearErrorAndReturn();
}



//PDS Servo Port configuration
RCB_EXPORT bool SetPDSConfiguration(UINT in_rcb3,UINT in_portNumber,UBYTE in_backup,UINT in_stretch,
                                   UINT in_speed,UINT in_pulsePunch,UINT in_deadBand,UINT in_damping,
                                   UINT in_failSafeTime,
                                   UINT in_lowerPositionLimit,UINT in_upperPositionLimit
                                   )
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;
    if ( in_portNumber > 23 )
    {
        char message[512];
        sprintf_s(message,512,"in_portNumber(%d) outside valid range: Should be between [0..23]\n",in_portNumber);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_stretch > 5 || in_stretch == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_stretch = %d outside valid range: Should be between [1..5]\n",in_stretch);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }
    if ( in_speed > 3 || in_speed == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_speed = %d outside valid range: Should be between [1..3]\n",in_speed);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( rcb->m_model == RCB3_MODEL_J )
    {
        SetError(RCB3_FUNCTION_NOT_SUPPORTED_ON_THIS_MODEL,NULL);
        return false;
    }

    UBYTE command[15];
    command[0] = 0xD7;
    command[1] = in_portNumber;
    command[2] = in_backup;

    command[3] = in_stretch;
    command[4] = in_speed;

    if ( in_pulsePunch > 3 || in_pulsePunch == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_pulsePunch = %d outside valid range: Should be between [1..3]\n",in_pulsePunch);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_deadBand > 5 || in_deadBand == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_deadBand = %d outside valid range: Should be between [1..5]\n",in_deadBand);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_damping > 2 || in_damping == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_damping = %d outside valid range: Should be 1 or 2\n",in_damping);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_failSafeTime > 10 || in_failSafeTime == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_failSafeTime = %d outside valid range: Should be between [1..10]\n",in_failSafeTime);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_upperPositionLimit > 2200 || in_upperPositionLimit < 1700 )
    {
        char message[512];
        sprintf_s(message,512,"in_upperPositionLimit = %d outside valid range: Should be between [1700..2200]\n",in_upperPositionLimit);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_lowerPositionLimit > 1300 || in_lowerPositionLimit < 800 )
    {
        char message[512];
        sprintf_s(message,512,"in_lowerPositionLimit = %d outside valid range: Should be between [800..1300]\n",in_lowerPositionLimit);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    command[5] = in_pulsePunch;
    command[6] = in_deadBand;
    command[7] = in_damping;
    command[8] = in_failSafeTime;
    command[9] = 0;
    command[10] = in_upperPositionLimit>>8;
    command[11] = in_upperPositionLimit&0xFF;
    command[12] = in_lowerPositionLimit>>8;
    command[13] = in_lowerPositionLimit&0xFF;
    
    GenerateCHKSUMAndSend(15);
    receiveAck();
    ClearErrorAndReturn();
}


RCB_EXPORT bool GetPDSConfiguration(UINT in_rcb3,UINT in_portNumber,UBYTE &out_backup,UINT &out_stretch,
                                   UINT &out_speed,UINT &out_pulsePunch,UINT &out_deadBand,UINT &out_damping,
                                   UINT &out_failSafeTime,
                                   UINT &out_lowerPositionLimit,UINT &out_upperPositionLimit
                                   )
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;
    if ( in_portNumber > 23 )
    {
        char message[512];
        sprintf_s(message,512,"in_portNumber(%d) outside valid range: Should be between [0..23]\n",in_portNumber);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }
    if ( rcb->m_model == RCB3_MODEL_J )
    {
        SetError(RCB3_FUNCTION_NOT_SUPPORTED_ON_THIS_MODEL,NULL);
        return false;
    }

    UBYTE command[3];
    command[0] = 0xD6;
    command[1] = in_portNumber;
    
    GenerateCHKSUMAndSend(3);
    receiveAnswerAndCheck(13);

    out_backup = answer[0];
    out_stretch = answer[1];
    out_speed   = answer[2];
    out_pulsePunch = answer[3];
    out_deadBand = answer[4];
    out_damping = answer[5];
    out_failSafeTime = answer[6];
    out_lowerPositionLimit = (UINT)answer[9] + (((UINT)answer[8])<<8);
    out_upperPositionLimit = (UINT)answer[11] + (((UINT)answer[10])<<8);

    ClearErrorAndReturn();
}


RCB_EXPORT bool SetPDSPortsInUse(UINT in_rcb3,UINT in_pdsPortsInUse,UINT in_option)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;
    in_option |= RCB_FORCE_ACK; //force ACK

    if ( rcb->m_model == RCB3_MODEL_J )
    {
        SetError(RCB3_FUNCTION_NOT_SUPPORTED_ON_THIS_MODEL,NULL);
        return false;
    }

    UBYTE command[6];
    command[0] = 0xD5;
    command[1] = in_option;
    command[2] = in_pdsPortsInUse&0xFF;   
    command[3] = (in_pdsPortsInUse>>8)&0xFF;
    command[4] = (in_pdsPortsInUse>>16)&0xFF;
    
    GenerateCHKSUMAndSend(6);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetPDSPortsInUse(UINT in_rcb3,UINT &out_pdsPortsInUse)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    if ( rcb->m_model == RCB3_MODEL_J )
    {
        SetError(RCB3_FUNCTION_NOT_SUPPORTED_ON_THIS_MODEL,NULL);
        return false;
    }

    UBYTE command[2];
    command[0] = 0xD4;
    
    GenerateCHKSUMAndSend(2);
    receiveAnswerAndCheck(4);

    out_pdsPortsInUse = answer[2];
    out_pdsPortsInUse <<= 8;
    out_pdsPortsInUse += answer[1];
    out_pdsPortsInUse <<= 8;
    out_pdsPortsInUse += answer[0];

    ClearErrorAndReturn();
}



//Default Port Position
RCB_EXPORT bool SetPortsDefaultPosition(UINT in_rcb3,const UINT *in_defaultPositions/*24*/)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(in_defaultPositions);

    UBYTE command[51];
    command[0] = 0xD3;
    command[1] = RCB_FORCE_ACK;
    try
    {
        for(UINT i=0;i<24;i++)
        {
            command[2+i*2] = (in_defaultPositions[i]>>8) & 0xFF;    
            command[3+i*2] = in_defaultPositions[i] & 0xFF;
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while reading in_defaultPositions variable in SetPortsDefaultPosition\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    GenerateCHKSUMAndSend(51);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetPortsDefaultPosition(UINT in_rcb3,UINT *out_defaultPositions/*24*/)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(out_defaultPositions);

    UBYTE command[2];
    command[0] = 0xD2;
    
    GenerateCHKSUMAndSend(2);
    receiveAnswerAndCheck(49);

    try
    {
        for(UINT i=0;i<24;i++)
        {
            out_defaultPositions[i] = (((UINT)answer[i*2]) << 8) + (UINT)answer[1+i*2];
        }
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing out_defaultPositions variable in GetPortsDefaultPosition\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    ClearErrorAndReturn();
}


//Port Names
RCB_EXPORT bool SetPortName(UINT in_rcb3,UINT in_option,UINT in_portIndex,const char *in_name)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    if ( in_portIndex > 23 )
    {
        char message[512];
        sprintf_s(message,512,"PortIndex(%d) outside valid range: Should be between [0..23]\n",in_portIndex);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    in_option |= RCB_FORCE_ACK;

    UBYTE command[36];
    command[0] = 0xD1;
    command[1] = in_option;
    command[2] = in_portIndex;
    memset(command+3,32,20);

    if ( in_name != NULL )
    {
        try
        {
            if ( strlen(in_name) > 32 )
            {
                char message[512];
                sprintf_s(message,512,"in_name string too long : Should be max 32 characters long\n");
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }            
            memcpy(command+3,in_name,strlen(in_name));
        }
        catch(...)
        {
            char message[512];
            sprintf_s(message,512,"Exception caught while reading in_name variable\n");
            SetError(RCB3_INVALID_PARAMETER,message);
            return false;
        }
    }

    GenerateCHKSUMAndSend(36);
    receiveAck();
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetPortName(UINT in_rcb3,UINT in_portIndex,char *out_name/*32*/)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(out_name);

    if ( in_portIndex > 23 )
    {
        char message[512];
        sprintf_s(message,512,"PortIndex(%d) outside valid range: Should be between [0..23]\n",in_portIndex);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    UBYTE command[3];
    command[0] = 0xD0;
    command[1] = in_portIndex;
    
    GenerateCHKSUMAndSend(3);
    receiveAnswerAndCheck(33);

    try
    {
        memcpy(out_name,answer,32);
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing out_name variable\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    ClearErrorAndReturn();
}


//Motion/Scenario Edition
RCB_EXPORT bool SetMotionScenarioData(UINT in_rcb3,UINT in_option,UINT in_motionScenarioIndex,UINT in_moveCount,UINT in_portsUsed, 
                                      UINT in_minute,UINT in_hour,UINT in_day,UINT in_month,
                                      UINT in_button,const char *in_name /*32 max*/)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    if ( in_motionScenarioIndex > 23 )
    {
        char message[512];
        sprintf_s(message,512,"in_motionScenarioIndex(%d) outside valid range: Should be between [0..84]\n",in_motionScenarioIndex);
        SetError(RCB3_INVALID_MOTION_INDEX,message);
        return false;
    }

    if ( in_minute > 59 )
    {
        char message[512];
        sprintf_s(message,512,"in_minute (%d) outside valid range: Should be between [0..59]\n",in_minute);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_hour > 23 )
    {
        char message[512];
        sprintf_s(message,512,"in_hour (%d) outside valid range: Should be between [0..23]\n",in_hour);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_day > 31 || in_day == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_day (%d) outside valid range: Should be between [1..31]\n",in_day);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    if ( in_month > 12 || in_month == 0 )
    {
        char message[512];
        sprintf_s(message,512,"in_month (%d) outside valid range: Should be between [1..12]\n",in_month);
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    in_option |= RCB_FORCE_ACK;

    UBYTE command[46];
    command[0] = 0xF6;
    command[1] = in_option;
    command[2] = in_motionScenarioIndex;
    command[3] = in_moveCount;
    command[4] = in_portsUsed&0xFF;
    command[5] = (in_portsUsed>>8 ) & 0xFF;
    command[6] = (in_portsUsed>>16) & 0xFF;    
    command[7] = in_minute;
    command[8] = in_hour;
    command[9] = in_day;
    command[10] = in_month;
    command[11] = (in_button>>8)&0xFF;
    command[12] = in_button&0xFF;

    memset(command+13,32,20);

    if ( in_name != NULL )
    {
        try
        {
            if ( strlen(in_name) > 32 )
            {
                char message[512];
                sprintf_s(message,512,"in_name string too long : Should be max 32 characters long\n");
                SetError(RCB3_INVALID_PARAMETER,message);
                return false;
            }            
            memcpy(command+13,in_name,strlen(in_name));
        }
        catch(...)
        {
            char message[512];
            sprintf_s(message,512,"Exception caught while reading in_name variable\n");
            SetError(RCB3_INVALID_PARAMETER,message);
            return false;
        }
    }

    GenerateCHKSUMAndSend(46);
    receiveAck();
    ClearErrorAndReturn();

}

RCB_EXPORT bool GetMotionScenarioData(UINT in_rcb3,UINT in_motionScenarioIndex,
                                      UINT &out_moveCount,
                                      UINT &out_portsUsed, 
                                      UINT &out_minute,UINT &out_hour,UINT &out_day,UINT &out_month,
                                      UINT &out_button,char *out_name /*32 max*/)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(out_name);

    if ( in_motionScenarioIndex > 84 )
    {
        char message[512];
        sprintf_s(message,512,"in_motionScenarioIndex(%d) outside valid range: Should be between [0..84]\n",in_motionScenarioIndex);
        SetError(RCB3_INVALID_MOTION_INDEX,message);
        return false;
    }

    UBYTE command[3];
    command[0] = 0xF5;
    command[1] = in_motionScenarioIndex;
    
    GenerateCHKSUMAndSend(3);
    receiveAnswerAndCheck(43);

    try
    {
        memcpy(out_name,answer+10,32);
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing out_name variable\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }

    out_moveCount = answer[0];
    out_portsUsed = answer[3];
    out_portsUsed <<= 8;
    out_portsUsed += answer[2];
    out_portsUsed <<= 8;
    out_portsUsed += answer[1];

    out_minute = answer[4];
    out_hour = answer[5];
    out_day = answer[6];
    out_month = answer[7];
    out_button = answer[8];
    out_button<<=8;
    out_button += answer[9];

    ClearErrorAndReturn();
}

RCB_EXPORT bool GetLastDataFromRemoteControl(UINT in_rcb3,char *out_remoteData /*7*/)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(out_remoteData);

    rcb->CheckUpdate();
    try
    {
        memcpy(out_remoteData,rcb->m_lastRemoteControlData,7);
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing out_remoteData variable\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }
    ClearErrorAndReturn();
}

RCB_EXPORT bool GetLastPortsStatus(UINT in_rcb3,UINT &out_portsDone,UINT *out_portMotionDone/*24*/)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    checkForNullPointer(out_portMotionDone);

    rcb->CheckUpdate();
    out_portsDone = rcb->m_lastPortsDone;
    try
    {
        memcpy(out_portMotionDone,rcb->m_lastPortsMotion,7);
    }
    catch(...)
    {
        char message[512];
        sprintf_s(message,512,"Exception caught while writing out_portMotionDone variable\n");
        SetError(RCB3_INVALID_PARAMETER,message);
        return false;
    }
    ClearErrorAndReturn();

}

RCB_EXPORT bool GetLastPowerAndADValues(UINT in_rcb3,float &out_power,float &out_ad1,float &out_ad2,float &out_ad3)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    rcb->CheckUpdate();
    out_power = rcb->m_lastPower;
    out_ad1 = rcb->m_lastAD1;
    out_ad2 = rcb->m_lastAD2;
    out_ad3 = rcb->m_lastAD3;
    ClearErrorAndReturn();
}

RCB_EXPORT bool SendBytesToCOMPort(UINT in_rcb3,const char *buffer,UINT buffer_size, int which_controler)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    int written;
    rcb->m_comPort->WriteCOM((void *)buffer,buffer_size,&written,which_controler);

    if ( written == buffer_size )
        return true;

    return false;
}

RCB_EXPORT bool ReceiveBytesFromCOMPort(UINT in_rcb3,char *buffer,UINT buffer_size)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    int read;
    rcb->m_comPort->ReadCOM(buffer,buffer_size,&read);

    if ( read == buffer_size)
        return true;

    return false;
}

RCB_EXPORT bool LoadMotionFile(UINT in_rcb3,const char *in_motion_filename,UINT in_motion_index,UINT in_force_overwrite)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    RCBMotion m;
    if ( !m.ReadFromFile(in_motion_filename) )
        return false;

    UINT moveCount;
    UINT portsUsed; 
    UINT minute,hour,day,month;
    UINT button;
    char name[33];

    memset(name,0,33);

    if ( !GetMotionScenarioData(in_rcb3,in_motion_index,
                                moveCount,portsUsed, minute,hour,day,month,
                                button,name) )
        return false;

    if (( in_force_overwrite == 0) && ( moveCount != 0 ))
        return false;

    int pos = m.m_start;

    UINT positions[24];
    UINT speed;
    for(int i=0;i<m.m_item_count;i++)
    {
        speed = m.m_items[pos].m_params.front();
        std::list<int>::iterator it = m.m_items[pos].m_params.begin();
        speed = *it;
        for(int j=0;j<24;j++)
        {
            it++;
            positions[j] = *it;
        }

        if ( !SetPortsSpeedAndPosition(in_rcb3,in_motion_index,i,speed,positions,RCB3MakeOption(1,0)) )
            return false;

        pos = m.GetNextItem();
    }
    

    /*always report all ports used*/
    UINT used_ports = 0x0FFF;
    memset(name,0,33);
    if ( m.m_name )
    {
        int l = min((int)strlen(m.m_name),32);
        memcpy(name,m.m_name,l);
    }
    if ( !SetMotionScenarioData(in_rcb3,RCB3MakeOption(1,0),in_motion_index,m.m_item_count,used_ports, 
                                      13,10,13,10,
                                      m.m_control,name) )
      return false;

    return true;
}

RCB_EXPORT bool LoadScenarioFile(UINT in_rcb3,const char *in_scenario_filename,UINT in_scenario_index,UINT in_force_overwrite)
{
    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    RCBMotion m;
    if ( !m.ReadFromFile(in_scenario_filename) )
        return false;

    UINT moveCount;
    UINT portsUsed; 
    UINT minute,hour,day,month;
    UINT button;
    char name[33];

    memset(name,0,33);

    UINT in_scenarioIndex;
    MakeMotionScenarioIndex(in_scenarioIndex,-1,in_scenario_index);

    if ( !GetMotionScenarioData(in_rcb3,in_scenarioIndex,
                                moveCount,portsUsed, minute,hour,day,month,
                                button,name) )
        return false;

    if (( in_force_overwrite == 0) && ( moveCount != 0 ))
        return false;

    int pos = m.m_start;

    for(int i=0;i<m.m_item_count;i++)
    {
        UINT in_motionIndex = m.m_items[pos].m_params.front();

        if ( !SetMotionInScenario(in_rcb3,in_scenarioIndex,i,in_motionIndex,RCB3MakeOption(1,0)) )
            return false;

        pos = m.GetNextItem();
    }
    

    /*always report all ports used*/
    UINT used_ports = 0x0FFF;
    memset(name,0,33);
    if ( m.m_name )
    {
        int l = min((int)strlen(m.m_name),32);
        memcpy(name,m.m_name,l);
    }
    if ( !SetMotionScenarioData(in_rcb3,RCB3MakeOption(1,0),in_scenarioIndex,m.m_item_count,used_ports, 
                                      13,10,13,10,
                                      m.m_control,name) )
      return false;

    return true;
}

RCB_EXPORT bool LoadHomePositionFile(UINT in_rcb3,const char *in_homeposition_filename,UINT in_scenario_index,UINT in_force_overwrite)
{
    if ( in_homeposition_filename == NULL )
        return false;

    RCB3Interface *rcb = GetRCB(in_rcb3);
    if ( !rcb )
        return false;

    FILE *f;

    if ( fopen_s(&f,in_homeposition_filename,"r") != 0 )
        return false;

    UINT homePositions[24];
    for(int i=0;i<24;i++)
        homePositions[i] = 16384;

    char buff[256];
    
    while(!feof(f))
    {
        memset(buff,0,256);
        fgets(buff,256,f);
        if ( buff[0] == 'C' )
        {
            int channel,value;
            sscanf_s(buff,"CH%d=%d",&channel,&value);
            if ( channel >= 1 && channel <= 24 )
            {
                homePositions[channel] = value;
            }
        }
    }

    fclose(f);

    return SetPortsHomePosition(in_rcb3,homePositions,RCB3MakeOption(1,0));
}

int GetAnswerSizeForCommand(char command)
{
    return g_answer[(unsigned int)command];
}