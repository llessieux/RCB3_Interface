/*
RCB3_Interface V 1.00

This program is free software; you can redistribute it freely.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Laurent Lessieux
Feb 2007

For more details
http://www.lessieux.com/mywiki/index.php?title=RCB3_Interface

*/

//Here goes all the C declarations for the external Interface.

#define RCB_EXPORT extern "C" __declspec(dllexport) 

#ifndef UBYTE
#define UBYTE unsigned char
#endif

#ifndef UINT
#define UINT unsigned int
#endif

enum {
    RCB3_MODEL_NORMAL,
    RCB3_MODEL_J
};

//Creation/Destruction of the interface. 
RCB_EXPORT bool CreateRCB3Interface(int in_comPort,int in_comSpeed,int in_model,UINT &out_rcb3Interface);
RCB_EXPORT bool CreateRCB3InterfaceSkipCheck(int in_comPort,int in_comSpeed,int in_model,UINT &out_rcb3Interface);
RCB_EXPORT bool CreateRCB3InterfaceViaBlueSmirf(int in_comPort,int in_comSpeed,int in_rcbmodel,int in_autodetectSpeed,int in_writeGoodConfigToFlash,int in_skipCheck,UINT &out_rcb3Interface);
RCB_EXPORT bool CreateRCB3InterfaceViaEZWism(char *in_address,int in_port,int in_rcbmodel,int in_skipCheck,UINT &out_rcb3Interface);

RCB_EXPORT bool DestroyRCB3Interface(UINT in_rcb3Interface);

//Error reporting
enum {
    RCB3_NO_ERROR,
    RCB3_CANNOT_OPEN_COM,
    RCB3_COM_WRITE_FAILED,
    RCB3_COM_READ_FAILED,
    RCB3_NOT_ENOUGH_MEMORY,
    RCB3_INVALID_INTERFACE,
    RCB3_INVALID_PORT,
    RCB_INVALID_CHECKSUM_RECEIVED,
    RCB3_INVALID_SCENARIO_INDEX,
    RCB3_INVALID_MOTION_INDEX,
    RCB3_INVALID_SLOT_INDEX,
    RCB3_INVALID_PARAMETER,
    RCB3_NO_RESPONSE,
    RCB3_FUNCTION_NOT_SUPPORTED_ON_THIS_MODEL,
    RCB3_LAST_ERROR
};

RCB_EXPORT UINT GetLastRCB3Error();
RCB_EXPORT char* GetLastRCB3ErrorName(UINT error);
RCB_EXPORT char* GetLastRCB3FullErrorMessage();

//value of 0 or 1 for each. Usage depends on command too.
RCB_EXPORT UINT RCB3MakeOption(UINT storeToEeprom,UINT forceMotion); 
enum {
    RCB3_DEFAULT_OPTION = 0,
    RCB_STORE_TO_EEPROM = 2,
    RCB_FORCE_MOTION = 4
};

inline bool MakeMotionScenarioIndex(UINT &out_motionScenarioIndex,int motion = -1, int scenario = -1)
{
    if ( motion == -1 && scenario == -1 )
        return false;
    if ( motion != -1 && scenario != -1 )
        return false;
    if ( motion < 0 || motion > 79 )
        return false;
    if ( scenario < 0 || scenario > 4 )
        return false;

    if ( scenario != -1 )
        out_motionScenarioIndex = 80+scenario;
    else
        out_motionScenarioIndex = motion;

    return true;
}

//Software Switches
RCB_EXPORT bool SetRCB3SoftwareSwitch(UINT in_rcb3,UINT in_SWITCH,UINT in_option);
RCB_EXPORT bool GetRCB3SoftwareSwitch(UINT in_rcb3,UINT &out_SWITCH,UINT in_option);

//Motion/Scenario Playback
RCB_EXPORT bool PlayMotion(UINT in_rcb3,UINT in_motionIndex,UINT in_option);
RCB_EXPORT bool PlayScenario(UINT in_rcb3,UINT in_scenarioIndex,UINT in_option);

//Interruption of port motion (one bit set per port to stop)
RCB_EXPORT bool StopPorts(UINT in_rcb3,UINT in_portsToStop);

//Set Port Speed and Position
RCB_EXPORT bool SetPortSpeedAndPosition(UINT in_rcb3,UINT in_portIndex,UINT in_speed,UINT in_position,UINT in_option);
RCB_EXPORT bool SetPortsSpeedAndPosition(UINT in_rcb3,UINT in_motionIndex,UINT in_slotIndex,UINT in_speed,const UINT *in_position /*24*/,UINT in_option);
RCB_EXPORT bool GetPortsSpeedAndPosition(UINT in_rcb3,UINT in_option,UINT in_motionIndex,UINT in_slotIndex,UINT &out_speed,UINT *out_position/*24*/);

RCB_EXPORT bool SetPortHomePosition(UINT in_rcb3,UINT in_portIndex,UINT in_homePosition,UINT in_option);
RCB_EXPORT bool SetPortsHomePosition(UINT in_rcb3,const UINT *in_homePositions/*24*/,UINT in_option);
RCB_EXPORT bool GetPortsHomePosition(UINT in_rcb3,UINT *out_homePositions/*24*/,UINT in_option);

//Teaching
RCB_EXPORT bool GetTeachedPosition(UINT in_rcb3,UINT *out_positions/*24*/);

//Scenario Edition
RCB_EXPORT bool SetMotionInScenario(UINT in_rcb3,UINT in_scenarioIndex,UINT in_slotIndex,UINT in_motionIndex,UINT in_option);
RCB_EXPORT bool GetMotionInScenario(UINT in_rcb3,UINT in_scenarioIndex,UINT in_slotIndex,UINT &out_motionIndex);

//Start switch/Power on Motion/Scenario
RCB_EXPORT bool SetStartupMotion(UINT in_rcb3,UINT in_motionScenarioIndexSwitchPressed,UINT in_motionScenarioIndexPowerOn,UINT in_option);
RCB_EXPORT bool GetStartupMotion(UINT in_rcb3,UINT &out_motionScenarioIndexSwitchPressed,UINT &out_motionScenarioIndexPowerOn);

//Low Power Configuration (in V)
RCB_EXPORT bool SetLowPowerThresholdAndMotion(UINT in_rcb3,float in_lowPower,UINT in_motionScenarioIndex,UINT in_option);
RCB_EXPORT bool GetLowPowerThresholdAndMotion(UINT in_rcb3,float &out_lowPower,UINT &out_motionScenarioIndex);

//RCB3 version (last byte will always be zero
RCB_EXPORT bool GetRCBVersion(UINT in_rcb3,char *out_version /*65*/);

//Analog Input 
//outputs/inputs (In V)

RCB_EXPORT bool GetAnalogInputs(UINT in_rcb3,float &out_power,float &out_ad1,float &out_ad2,float &out_ad3);
RCB_EXPORT bool GetAnalogInputsRAW(UINT in_rcb3,UINT &out_power,UINT &out_ad1,UINT &out_ad2,UINT &out_ad3);

//Channel = 0,1 or 2
//Set to -1V for automatic detection but should not be done with ROM option
RCB_EXPORT bool SetAnalogBaseValue(UINT in_rcb3,UINT in_channel,float in_baseValue,UINT in_option);
RCB_EXPORT bool SetAnalogBaseValues(UINT in_rcb3,const float *in_baseValue /*3*/,UINT in_option);
RCB_EXPORT bool GetAnalogBaseValues(UINT in_rcb3,float *out_baseValue /*3*/,UINT in_option);

RCB_EXPORT bool SetAnalogBaseValueRAW(UINT in_rcb3,UINT in_channel,UINT in_baseValue,UINT in_option);
RCB_EXPORT bool SetAnalogBaseValuesRAW(UINT in_rcb3,const UINT *in_baseValue /*3*/,UINT in_option);
RCB_EXPORT bool GetAnalogBaseValuesRAW(UINT in_rcb3,UINT *out_baseValue /*3*/,UINT in_option);

//RealTime Mixing
RCB_EXPORT bool SetRealTimeMixing(UINT in_rcb3,UINT in_channel,const UINT *in_mixingFactors/*24*/,const UINT *in_reverseFactor/*24*/,const UINT *in_set/*24*/,UINT in_option);
RCB_EXPORT bool GetRealTimeMixing(UINT in_rcb3,UINT in_channel,UINT *out_mixingFactors/*24*/,UINT *out_reverseFactor/*24*/,UINT *out_set/*24*/,UINT in_option);

//Configuration of Motion playback on AD Deltas.
//0 or 1 to enable or disable motion playback
//RCB_EXPORT bool SetMotionPlaybackforAD(UINT in_rcb3,UINT in_sc1AD1,UINT in_sc2AD1,UINT in_sc3AD2,UINT in_sc4AD2,UINT in_sc5AD3,UINT in_sc6AD3);

RCB_EXPORT bool SetMotionPlaybackforAD(UINT in_rcb3,const UINT *in_motionScenarioIndices/*6*/,
                                                    const UINT *in_levels/*6*/,
                                                    const UINT *in_reverse/*6*/,
                                                    const UINT *in_mixingSwitch/*6*/,
                                                    UINT in_option);

RCB_EXPORT bool GetMotionPlaybackforAD(UINT in_rcb3,UINT in_option,
                                       UINT *out_motionScenarioIndices/*6*/,
                                       UINT *out_levels/*6*/,
                                       UINT *out_reverse/*6*/,
                                       UINT *out_mixingSwitch/*6*/
                                       );

//Remote Control
RCB_EXPORT bool GetDataFromRemoteControl(UINT in_rcb3,char *out_remoteControl/*7*/);
RCB_EXPORT bool SetDataFromRemoveControlOverwrite(UINT in_rcb3,UINT in_option,UINT in_button,const UINT *in_analogChannel/*4*/);

//Serial Extension
RCB_EXPORT bool SetSerialExtensionAnalogBaseValues(UINT in_rcb3,const UBYTE *in_baseValue/*4*/,UINT in_option);
RCB_EXPORT bool GetSerialExtensionAnalogBaseValues(UINT in_rcb3,UINT in_option,UBYTE *out_baseValue/*4*/);

//ICS Mode
//1 bit per port
RCB_EXPORT bool SetPortsInICSMode(UINT in_rcb3,UINT in_portsInICS,UINT in_option);
RCB_EXPORT bool GetPortsInICSMode(UINT in_rcb3,UINT &out_portsInICS);

//RedVersion Port configuration
RCB_EXPORT bool SetRedVersionConfiguration(UINT in_rcb3,UINT in_portNumber,UBYTE in_backup,const UBYTE *in_stretch/*3*/,
                                           const UBYTE *in_speed/*3*/,UINT in_pulsePunch,UINT in_deadBand,UINT in_damping,
                                           UINT in_protectionTime, UINT in_reverse, 
                                           UINT in_lowerPositionLimit,UINT in_upperPositionLimit,
                                           UINT in_freeTime);

RCB_EXPORT bool GetRedVersionConfiguration(UINT in_rcb3,UINT in_portNumber,UBYTE &out_backup,UBYTE *out_stretch/*3*/,
                                           UBYTE *out_speed/*3*/,UINT &out_pulsePunch,UINT &out_deadBand,UINT &out_damping,
                                           UINT &out_protectionTime, UINT &out_reverse, 
                                           UINT &out_lowerPositionLimit,UINT &out_upperPositionLimit,
                                           UINT &out_freeTime);

//Serial Servo Port configuration

RCB_EXPORT bool SetSerialServoConfiguration(UINT in_rcb3,UINT in_portNumber,UBYTE in_backup,const UBYTE *in_stretch/*3*/,
                                           const UBYTE *in_speed/*3*/,UINT in_pulsePunch,UINT in_deadBand,
                                           UINT in_damping,UINT in_protectionTime, UINT in_reverse, 
                                           UINT in_lowerPositionLimit,UINT in_upperPositionLimit,
                                           UINT in_freeTime,UINT in_stretchSIO,UINT in_speedSIO,
                                           UINT in_offset,UINT in_ID, UINT in_p1,UINT in_p2, UINT in_n1, UINT in_n2);

RCB_EXPORT bool GetSerialServoConfiguration(UINT in_rcb3,UINT in_portNumber,UBYTE &out_backup,UBYTE *out_stretch/*3*/,
                                           UBYTE *out_speed/*3*/,UINT &out_pulsePunch,UINT &out_deadBand,
                                           UINT &out_damping,UINT &out_protectionTime, UINT &out_reverse, 
                                           UINT &out_lowerPositionLimit,UINT &out_upperPositionLimit,
                                           UINT &out_freeTime,UINT &out_stretchSIO,UINT &out_speedSIO,
                                           UINT &out_offset,UINT &out_ID, UINT &out_p1,UINT &out_p2, UINT &out_n1, UINT &out_n2);

RCB_EXPORT bool GetSerialServoStretch(UINT in_rcb3,UINT *out_stretch/*24*/);
RCB_EXPORT bool GetSerialServoSpeed(UINT in_rcb3,UINT *out_speed/*24*/);
RCB_EXPORT bool SetSerialServoPortUsage(UINT in_rcb3,UINT in_portsUsed,UINT in_option);
RCB_EXPORT bool GetSerialServoPortUsage(UINT in_rcb3,UINT &out_portsUsed);


//PDS Servo Port configuration
RCB_EXPORT bool SetPDSConfiguration(UINT in_rcb3,UINT in_portNumber,UBYTE in_backup,UINT in_stretch,
                                   UINT in_speed,UINT in_pulsePunch,UINT in_deadBand,UINT in_damping,
                                   UINT in_failSafeTime,
                                   UINT in_lowerPositionLimit,UINT in_upperPositionLimit
                                   );

RCB_EXPORT bool GetPDSConfiguration(UINT in_rcb3,UINT in_portNumber,UBYTE &out_backup,UINT &out_stretch,
                                   UINT &out_speed,UINT &out_pulsePunch,UINT &out_deadBand,UINT &out_damping,
                                   UINT &out_failSafeTime,
                                   UINT &out_lowerPositionLimit,UINT &out_upperPositionLimit
                                   );

RCB_EXPORT bool SetPDSPortsInUse(UINT in_rcb3,UINT in_pdsPortsInUse,UINT in_option);
RCB_EXPORT bool GetPDSPortsInUse(UINT in_rcb3,UINT &out_pdsPortsInUse);


//Default Port Position
RCB_EXPORT bool SetPortsDefaultPosition(UINT in_rcb3,const UINT *in_defaultPositions/*24*/);
RCB_EXPORT bool GetPortsDefaultPosition(UINT in_rcb3,UINT *out_defaultPositions/*24*/);

//Port Names
RCB_EXPORT bool SetPortName(UINT in_rcb3,UINT in_option,UINT in_portIndex,const char *in_name);
RCB_EXPORT bool GetPortName(UINT in_rcb3,UINT in_portIndex,char *out_name /*32*/);


//Motion/Scenario Edition
RCB_EXPORT bool SetMotionScenarioData(UINT in_rcb3,UINT in_option,UINT in_motionScenarioIndex,UINT in_moveCount,UINT in_portsUsed, 
                                      UINT in_minute,UINT in_hour,UINT in_day,UINT in_month,
                                      UINT in_button,const char *in_name /*32 max*/);

RCB_EXPORT bool GetMotionScenarioData(UINT in_rcb3,UINT in_motionScenarioIndex,
                                      UINT &out_moveCount,
                                      UINT &out_portsUsed, 
                                      UINT &out_minute,UINT &out_hour,UINT &out_day,UINT &out_month,
                                      UINT &out_button,char *out_name /*32 max*/);

//Timer based Notification (0.5s) when activated in the Software Switches.
RCB_EXPORT bool GetLastDataFromRemoteControl(UINT in_rcb3,char *out_remoteData /*7*/);
RCB_EXPORT bool GetLastPortsStatus(UINT in_rcb3,UINT &out_portsDone,UINT *out_portMotionDone/*24*/);
RCB_EXPORT bool GetLastPowerAndADValues(UINT in_rcb3,float &out_power,float &out_ad1,float &out_ad2,float &out_ad3);


/////////////////////////
//Serial  Extension
/////////////////////////
RCB_EXPORT bool SendBytesToCOMPort(UINT in_rcb3,const char *in_buffer,UINT in_buffer_size,int in_which_controler);
RCB_EXPORT bool ReceiveBytesFromCOMPort(UINT in_rcb3,char *out_buffer,UINT in_buffer_size);


/////////////////////////
//Files  Extension
/////////////////////////

RCB_EXPORT bool LoadMotionFile(UINT in_rcb3,const char *in_motion_filename,UINT in_motion_index,UINT in_force_overwrite);
RCB_EXPORT bool LoadScenarioFile(UINT in_rcb3,const char *in_scenario_filename,UINT in_scenario_index,UINT in_force_overwrite);
RCB_EXPORT bool LoadHomePositionFile(UINT in_rcb3,const char *in_homeposition_filename,UINT in_scenario_index,UINT in_force_overwrite);
