/*
 * File:         BlueSmirf.cpp
 * Date:         January, 2007
 * Author:       Laurent Lessieux
 *
 * Copyright (c) 2006 Laurent Lessieux
 *               www.lessieux.com
 */

#include "stdafx.h"
#include "BlueSmirf.h"
#include "com.h"

int speeds[] = { 1200,2400,4800,9600,19200,38400,57600,115200, 0 };
int speeds_inv[] = { 5,10,20,39,79,157,236,472, 0 };

com *BlueSmirf::CreateComPortViaBlueSmirf(int port,int speed,bool autodetectSpeed,bool writeConfigToFlash)
{
    com *comport = NULL;
    int index = 0;
    int written;
    int read;
    char cr=0xD;
    char lf=0xA;
    char buffer[512];
    char answer[64];
    char expected_ok[6] = { 0xd, 0xa, 'O','K', 0xd,0xa };

    if ( autodetectSpeed )
    {      
        while (speeds[index] != 0 )
        {
            comport = new com(port,speeds[index]);
            if ( comport->IsOK() )
            {                
                sprintf_s(buffer,512,"+++%c",cr);
                comport->WriteCOM(buffer,(int)strlen(buffer),&written);
                
                comport->ReadCOM(answer,6,&read);
                if ( read == 6 )
                {                  
                    //check
                    if ( strncmp(answer,expected_ok,6) == 0 )
                    {
                        //break if ok.
                        break;
                    }
                }
                else 
                {
                    //Try sending an AT command, we might be in AT mode
                    sprintf_s(buffer,512,"ATVER,ver1%c",cr);
                    comport->WriteCOM(buffer,(int)strlen(buffer),&written);

                    comport->ReadCOM(answer,6,&read);
                    if ( read == 6 )
                    {                  
                        //check
                        if ( strncmp(answer,expected_ok,6) == 0 )
                        {
                            //Skip the version we don't really care
                            comport->ReadCOM(answer,2,&read); //cr,lf
                            do
                            {
                                comport->ReadCOM(answer,1,&read);  //version string + cr,lf
                            }while(answer[0] != lf );
                            //break if ok.
                            break;
                        }
                    }
                }
            }
            delete comport;
            index++;
        }
        if ( speeds[index] == 0 )
            return NULL;
    }
    else
    {
        comport = new com(port,speed);

        if ( comport->IsOK() )
        {                
            sprintf_s(buffer,512,"+++%c",cr);
            comport->WriteCOM(buffer,(int)strlen(buffer),&written);
            
            comport->ReadCOM(answer,6,&read);
            if ( read == 6 )
            {                  
                //check if it's what we expected
                if ( strncmp(answer,expected_ok,6) != 0 )
                {                    
                    delete comport;
                    return NULL;
                }
            }
            else 
            {
                //Try sending an AT command, we might be in AT mode
                sprintf_s(buffer,512,"AT%c",cr);
                comport->WriteCOM(buffer,(int)strlen(buffer),&written);

                comport->ReadCOM(answer,6,&read);
                if ( read == 6 )
                {                  
                    //check
                    if ( strncmp(answer,expected_ok,6) != 0 )
                    {
                        delete comport;
                        return NULL;
                    }
                }
            }
        }
        else
            return comport; //It will fail in the rcb3 constructor with the proper error message
    }

    if ( writeConfigToFlash && autodetectSpeed )
    {
        int i=0;
        while ((speeds[i] != speed )&&(speeds[i] != 0 ))
        {
            i++;
        }
        if ( speeds[i] == 0 )
        {
            delete comport;
            return NULL;
        }
        
        sprintf_s(buffer,512,"ATSW20,%d,0,0,1%c",speeds_inv[i],cr);
        comport->WriteCOM(buffer,(int)strlen(buffer),&written);

        delete comport;
        //Reopen the port with the wanted speed.
        comport = new com(port,speed);

        if ( comport == NULL || !comport->IsOK())
        {
            delete comport;
            return NULL;
        }
    }

    //Switch the BlueSmirf Module to 115200bps
    sprintf_s(buffer,512,"ATSW20,472,0,0,0%c",cr);
    comport->WriteCOM(buffer,(int)strlen(buffer),&written);

    Sleep(10);

    //Configure Fast Raw data Mode
    //comport->WriteCOM("ATMD",4,&written); //Normal mode
    comport->WriteCOM("ATMF",4,&written); //Fast Mode because we don't want to fall back into AT mode and we might be above 60K (probably)
    comport->WriteCOM(&cr,1,&written);
    comport->ReadCOM(answer,6,&read);
    if ( read == 6 )
    {
        //check
        if ( strncmp(answer,expected_ok,6) == 0 )
        {           
            return comport;
        }
    }
    else
    {
        comport->WriteCOM(&cr,1,&written);
        comport->ReadCOM(answer,1,&read);
        if (( read == 1) && (answer[0] == cr )) //we will assume that we are already in FastMode and that the RCB3 is responding.
        {
            return comport;
        }
    }

    delete comport;
    return NULL;
}

BlueSmirf::BlueSmirf()
{
}
