using System;
using System.Runtime.InteropServices;
using System.Text;

/// <summary>
/// C# Wrapper for the RCB3_Interface DLL
/// See .h file or documentation for more details.
/// </summary>
/// 
public class RCB3Interface
{
    //Error reporting
    enum RCB3ERRORS {
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
        RCB3_LAST_ERROR
    };

    enum RCB3OPTIONS {
        RCB3_DEFAULT_OPTION = 0,
        RCB_STORE_TO_EEPROM = 2,
        RCB_FORCE_MOTION = 4
    };

    [DllImport("RCB3_Interface.dll")]
    public static extern bool CreateRCB3Interface(int in_comPort,int in_comSpeed,int in_model,ref uint out_rcb3Interface);


    //Creation/Destruction of the interface. 
    [DllImport("RCB3_Interface.dll")]
    public static extern bool DestroyRCB3Interface(uint in_rcb3Interface);


    [DllImport("RCB3_Interface.dll")] 
    public static extern uint GetLastRCB3Error();

    [DllImport("RCB3_Interface.dll")] 
    unsafe public static extern char* GetLastRCB3ErrorName(uint error);

    [DllImport("RCB3_Interface.dll")]
    unsafe public static extern char* GetLastRCB3FullErrorMessage();

    //value of 0 or 1 for each. Usage depends on command too.
    [DllImport("RCB3_Interface.dll")] 
    public static extern uint RCB3MakeOption(uint storeToEeprom,uint forceMotion); 

    bool MakeMotionScenarioIndex(ref uint out_motionScenarioIndex,int motion, int scenario)
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
            out_motionScenarioIndex = (uint)(80+scenario);
        else
            out_motionScenarioIndex = (uint)(motion);

        return true;
    }

    //Software Switches
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetRCB3SoftwareSwitch(uint in_rcb3,uint in_SWITCH,uint in_option);

    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetRCB3SoftwareSwitch(uint in_rcb3,ref uint out_SWITCH,uint in_option);

    //Motion/Scenario Playback
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool PlayMotion(uint in_rcb3,uint in_motionIndex,uint in_option);

    [DllImport("RCB3_Interface.dll")] 
    public static extern bool PlayScenario(uint in_rcb3,uint in_scenarioIndex,uint in_option);

    //Interruption of port motion (one bit set per port to stop)
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool StopPorts(uint in_rcb3,uint in_portsToStop);

    //Set Port Speed and Position
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetPortSpeedAndPosition(uint in_rcb3,uint in_portIndex,uint in_speed,uint in_position,uint in_option);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetPortsSpeedAndPosition(uint in_rcb3,uint in_motionIndex,uint in_slotIndex,uint in_speed,uint[] in_position /*24*/,uint in_option);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetPortsSpeedAndPosition(uint in_rcb3,uint in_option,uint in_motionIndex,uint in_slotIndex,ref uint out_speed,uint[] out_position/*24*/);

    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetPortHomePosition(uint in_rcb3,uint in_portIndex,uint in_homePosition,uint in_option);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetPortsHomePosition(uint in_rcb3,uint[] in_homePositions/*24*/,uint in_option);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetPortsHomePosition(uint in_rcb3,uint[] out_homePositions/*24*/,uint in_option);

    //Teaching
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetTeachedPosition(uint in_rcb3,uint[] out_positions/*24*/);

    //Scenario Edition
    [DllImport("RCB3_Interface.dll")]
    public static extern bool SetMotionInScenario(uint in_rcb3, uint in_scenarioIndex, uint in_slotIndex, uint in_motionIndex, uint in_option);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetMotionInScenario(uint in_rcb3,uint in_scenarioIndex,uint in_slotIndex,ref uint out_motionIndex);

    //Start switch/Power on Motion/Scenario
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetStartupMotion(uint in_rcb3,uint in_motionScenarioIndexSwitchPressed,uint in_motionScenarioIndexPowerOn,uint in_option);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetStartupMotion(uint in_rcb3,ref uint out_motionScenarioIndexSwitchPressed,ref uint out_motionScenarioIndexPowerOn);

    //Low Power Configuration (in V)
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetLowPowerThresholdAndMotion(uint in_rcb3,float in_lowPower,uint in_motionScenarioIndex,uint in_option);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetLowPowerThresholdAndMotion(uint in_rcb3,ref float out_lowPower,ref uint out_motionScenarioIndex);

    //Scenario Port used:
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetScenarioPortUsage(uint in_rcb3,uint in_portsUsed,uint in_option);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetScenarioPortUsage(uint in_rcb3,ref uint out_portsUsed);

    //RCB3 version (last byte will always be zero
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetRCBVersion(uint in_rcb3,StringBuilder out_version /*65*/);

    //Analog Input 
    //outputs/inputs (In V)

    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetAnalogInputs(uint in_rcb3,ref float out_power,ref float out_ad1,ref float out_ad2,ref float out_ad3);

    //Channel = 0,1 or 2
    //Set to -1V for automatic detection but should not be done with ROM option
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetAnalogBaseValue(uint in_rcb3,uint in_channel,float in_baseValue,uint in_option);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetAnalogBaseValues(uint in_rcb3,float[] in_baseValue /*3*/,uint in_option);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetAnalogBaseValues(uint in_rcb3,float[] out_baseValue /*3*/,uint in_option);

    //RealTime Mixing
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetRealTimeMixing(uint in_rcb3,uint in_channel,uint[] in_mixingFactors/*24*/,uint[] in_reverseFactor/*24*/,uint[] in_set/*24*/,uint in_option);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetRealTimeMixing(uint in_rcb3,uint in_channel,uint[] out_mixingFactors/*24*/,uint[] out_reverseFactor/*24*/,uint[] out_set/*24*/,uint in_option);

    //Configuration of Motion playback on AD Deltas.
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetMotionPlaybackforAD(uint in_rcb3,uint[] in_motionScenarioIndices/*6*/,
                                                        uint[] in_levels/*6*/,
                                                        uint[] in_reverse/*6*/,
                                                        uint[] in_mixingSwitch/*6*/,
                                                        uint in_option);

    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetMotionPlaybackforAD(uint in_rcb3,uint in_option,
                                           uint[] out_motionScenarioIndices/*6*/,
                                           uint[] out_levels/*6*/,
                                           uint[] out_reverse/*6*/,
                                           uint[] out_mixingSwitch/*6*/
                                           );

    //Remote Control
    [DllImport("RCB3_Interface.dll")]
    public static extern bool GetDataFromRemoteControl(uint in_rcb3, StringBuilder out_remoteControl/*7*/);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetDataFromRemoveControlOverwrite(uint in_rcb3,uint in_option,uint in_button,uint[] in_analogChannel/*4*/);

    //Serial Extension
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetSerialExtensionAnalogBaseValues(uint in_rcb3,byte[] in_baseValue/*4*/,uint in_option);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetSerialExtensionAnalogBaseValues(uint in_rcb3,uint in_option,byte[] out_baseValue/*4*/);

    //ICS Mode
    //1 bit per port
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetPortsInICSMode(uint in_rcb3,uint in_portsInICS,uint in_option);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetPortsInICSMode(uint in_rcb3,ref uint out_portsInICS);

    //RedVersion Port configuration
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetRedVersionConfiguration(uint in_rcb3,uint in_portNumber,byte in_backup,byte[] in_stretch/*3*/,
                                               byte[] in_speed/*3*/,uint in_pulsePunch,uint in_deadBand,uint in_damping,
                                               uint in_protectionTime, uint in_reverse, 
                                               uint in_lowerPositionLimit,uint in_upperPositionLimit,
                                               uint in_freeTime);

    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetRedVersionConfiguration(uint in_rcb3,uint in_portNumber,ref byte out_backup,byte[] out_stretch/*3*/,
                                               byte[] out_speed/*3*/,ref uint out_pulsePunch,ref uint out_deadBand,ref uint out_damping,
                                               ref uint out_protectionTime, ref uint out_reverse, 
                                               ref uint out_lowerPositionLimit,ref uint out_upperPositionLimit,
                                               ref uint out_freeTime);

    //Serial Servo Port configuration

    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetSerialServoConfiguration(uint in_rcb3,uint in_portNumber,byte in_backup,byte[] in_stretch/*3*/,
                                               byte[] in_speed/*3*/,uint in_pulsePunch,uint in_deadBand,
                                               uint in_damping,uint in_protectionTime, uint in_reverse, 
                                               uint in_lowerPositionLimit,uint in_upperPositionLimit,
                                               uint in_freeTime,uint in_stretchSIO,uint in_speedSIO,
                                               uint in_offset,uint in_ID, uint in_p1,uint in_p2, uint in_n1, uint in_n2);

    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetSerialServoConfiguration(uint in_rcb3,uint in_portNumber,ref byte out_backup,byte[] out_stretch/*3*/,
                                               byte[] out_speed/*3*/,ref uint out_pulsePunch,ref uint out_deadBand,
                                               ref uint out_damping,ref uint out_protectionTime, ref uint out_reverse, 
                                               ref uint out_lowerPositionLimit,ref uint out_upperPositionLimit,
                                               ref uint out_freeTime,ref uint out_stretchSIO,ref uint out_speedSIO,
                                               ref uint out_offset,ref uint out_ID, ref uint out_p1,ref uint out_p2, ref uint out_n1, ref uint out_n2);

    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetSerialServoStretch(uint in_rcb3,uint[] out_stretch/*24*/);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetSerialServoSpeed(uint in_rcb3,uint[] out_speed/*24*/);


    //PDS Servo Port configuration
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetPDSConfiguration(uint in_rcb3,uint in_portNumber,byte in_backup,uint in_stretch,
                                       uint in_speed,uint in_pulsePunch,uint in_deadBand,uint in_damping,
                                       uint in_failSafeTime,
                                       uint in_lowerPositionLimit,uint in_upperPositionLimit
                                       );

    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetPDSConfiguration(uint in_rcb3,uint in_portNumber,ref byte out_backup,ref uint out_stretch,
                                       ref uint out_speed,ref uint out_pulsePunch,ref uint out_deadBand,ref uint out_damping,
                                       ref uint out_failSafeTime,
                                       ref uint out_lowerPositionLimit,ref uint out_upperPositionLimit
                                       );

    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetPDSPortsInUse(uint in_rcb3,uint in_pdsPortsInUse,uint in_option);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetPDSPortsInUse(uint in_rcb3,ref uint out_pdsPortsInUse);


    //Default Port Position
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetPortsDefaultPosition(uint in_rcb3,uint[] in_defaultPositions/*24*/);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetPortsDefaultPosition(uint in_rcb3,uint[] out_defaultPositions/*24*/);

    //Port Names
    [DllImport("RCB3_Interface.dll")]
    public static extern bool SetPortName(uint in_rcb3, uint in_option, uint in_portIndex, StringBuilder in_name);
    [DllImport("RCB3_Interface.dll")]
    public static extern bool GetPortName(uint in_rcb3, uint in_portIndex, StringBuilder out_name /*32*/);


    //Motion/Scenario Edition
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool SetMotionScenarioData(uint in_rcb3,uint in_option,uint in_motionScenarioIndex,uint in_moveCount,uint in_portsUsed, 
                                          uint in_minute,uint in_hour,uint in_day,uint in_month,
                                          uint in_button, StringBuilder in_name /*32 max*/);

    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetMotionScenarioData(uint in_rcb3,uint in_motionScenarioIndex,
                                          ref uint out_moveCount,
                                          ref uint out_portsUsed, 
                                          ref uint out_minute,ref uint out_hour,ref uint out_day,ref uint out_month,
                                          ref uint out_button, StringBuilder out_name /*32 max*/);

    //Timer based Notification (0.5s) when activated in the Software Switches.
    [DllImport("RCB3_Interface.dll")]
    public static extern bool GetLastDataFromRemoteControl(uint in_rcb3, StringBuilder out_remoteData /*7*/);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetLastPortsStatus(uint in_rcb3,ref uint out_portsDone,uint[] out_portMotionDone/*24*/);
    [DllImport("RCB3_Interface.dll")] 
    public static extern bool GetLastPowerAndADValues(uint in_rcb3,ref float out_power,ref float out_ad1,ref float out_ad2,ref float out_ad3);

    public RCB3Interface()
	{
	}
}
