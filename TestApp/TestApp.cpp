/*
 * File:         TestApp.cpp
 * Date:         January, 2007
 * Description:  Simple Test Application for the RCB3 Interface
 * Author:       Laurent Lessieux
 *
 * Copyright (c) 2006 Laurent Lessieux
 *               www.lessieux.com
 */

#include "stdafx.h"
#include <windows.h>

#include "..\RCB3_Interface\RCB3_Interface.h"
#include <memory.h>
#include <list>

class motion
{
public:
    int m_index;
    int m_moveCount;
};

void DisplayError()
{
    UINT error = GetLastRCB3Error();
    printf("Error = %s \n",GetLastRCB3ErrorName(error));
    printf("Full message %s \n",GetLastRCB3FullErrorMessage());
}
int _tmain(int argc, _TCHAR* argv[])
{
    UINT myrcb;
#if 1
    if ( !CreateRCB3InterfaceViaBlueSmirf(8,115200,0,0,0,0,myrcb) )
    {
        DisplayError();
        return 0;
    }
#else
    if ( !CreateRCB3Interface(6,115200,0,myrcb) )
    {
        DisplayError();
        return 0;
    }
#endif
    char version[65];
    if ( !GetRCBVersion(myrcb,version) )
        DisplayError();
    else
    {
        printf("Success: version = %s\n",version);
    }

    float power,ad1,ad2,ad3;
    if ( !GetAnalogInputs(myrcb,power,ad1,ad2,ad3) )
        DisplayError();
    else
        printf("Success: power = %fV, ad1 = %fV, ad2 = %fV, ad3 = %fV\n",power,ad1,ad2,ad3);

    UINT myswitch;
    GetRCB3SoftwareSwitch(myrcb,myswitch,0);
    myswitch &= ~(0x104);
    myswitch |= 0x200;
    SetRCB3SoftwareSwitch(myrcb,myswitch,0);
    
    /*
    SetPortSpeedAndPosition(myrcb,0,10,100,0);
    UINT position[24];
    for(int i=0;i<24;i++)
        position[i] = 0;
    //SetPortsSpeedAndPosition(myrcb,0,0,200,position,0);
    SetPortsSpeedAndPosition(myrcb,0,0,10,position,0);
    for(int i=0;i<24;i++)
        position[i] = 100;
    SetPortsSpeedAndPosition(myrcb,0,1,40,position,0);
   */

    std::list<motion> valid_motions;
    int used_slots = 0;
    for(int i=0;i<85;i++)
    {
        UINT out_moveCount;
        UINT out_portsUsed; 
        UINT out_minute,out_hour,out_day,out_month;
        UINT out_button;
        char out_name[33];

        memset(out_name,0,33);
        bool ok = GetMotionScenarioData(myrcb,i,
                                      out_moveCount,
                                      out_portsUsed, 
                                      out_minute,out_hour,out_day,out_month,
                                      out_button,out_name );
        if ( !ok )
        {
            DisplayError();
        }
        else
        {
            int v = i;
            if ( i < 80 )
                printf("Motion");
            else
            {
                v -= 80;
                printf("Scenario");
            }
            printf(" %d ",v);
            if ( used_slots )
            {
                if ( out_moveCount == 0 )
                    printf("is unavailable because of previous motion occupying the slots\n");
                else
                    printf("is in conflict with the previous motion reporting previous usage of the slots\n");
            }
            else 
            {
                if ( out_moveCount == 0 )
                    printf("is empty\n");
                else
                {
                    motion m;
                    m.m_index = i;
                    m.m_moveCount = out_moveCount;
                    valid_motions.push_back(m);
                    printf("uses %d moves/motions, is activated by button %d and is named %s\n",out_moveCount,out_button,out_name);
                }
            }
        }
        if ( used_slots )
            used_slots -= 30;
        else
            used_slots = out_moveCount - 30;

        if ( used_slots < 0 )
            used_slots = 0;
    }

    if ( valid_motions.size() != 0 )
    {
        std::list<motion>::iterator it = valid_motions.begin();
        while ( it != valid_motions.end() )
        {
            motion m = *it;
            int index = m.m_index;
            it++;
            if ( index < 79 )
            {
                printf("Dumping Motion %d \n",index);
                for(int i=0;i<m.m_moveCount;i++)
                {
                    UINT speed;
                    UINT position[24];
                    if ( GetPortsSpeedAndPosition(myrcb,RCB_STORE_TO_EEPROM,index,i,speed,position)  )
                    {
                        printf("Speed[%d]=%d\n",i,speed);                        
                        printf("Position[%d] =",i);
                        for(int j=0;j<24;j++)
                            printf("%d ",position[j]);
                        printf("\n");
                    }
                    else
                        DisplayError();
                }

            }
            else
            {
                index -= 80;
                printf("Dumping Scenario %d \n",index);
                for(int i=0;i<m.m_moveCount;i++)
                {
                    UINT motionIndex;
                    if ( GetMotionInScenario(myrcb,index,i,motionIndex)  )
                    {
                        printf("Motion[%d] = %d\n",i,motionIndex);
                    }
                    else
                        DisplayError();
                }
            }
        }

    }
    
    /*UINT ports;
    for(int i=0;i<50;i++)
    {
        Sleep(1000);
        GetLastPortsStatus(myrcb,ports,position);
    }*/

    if ( !DestroyRCB3Interface(myrcb) )
        DisplayError();

	return 0;
}

