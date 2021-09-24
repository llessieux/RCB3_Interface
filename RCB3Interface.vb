'
' RCB3_Interface V 1.00
'This program is free software; you can redistribute it freely.
'
'This program is distributed in the hope that it will be useful,
'but WITHOUT ANY WARRANTY; without even the implied warranty of
'MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
'
'Laurent Lessieux
'Feb 2007
'
'For more details
'http://www.lessieux.com/mywiki/index.php?title=RCB3_Interface


Imports Microsoft.VisualBasic


'Creation/Destruction of the interface. 
Public Declare Function CreateRCB3Interface Lib "RCB3_Interface.dll" Alias "CreateRCB3Interface" (ByVal in_comPort As Integer, ByVal in_comSpeed As Integer, ByVal in_model As Integer, ByRef out_rcb3Interface As UInteger) As Boolean
Public Declare Function DestroyRCB3Interface Lib "RCB3_Interface.dll" Alias "DestroyRCB3Interface" (ByVal in_rcb3Interface As UInteger) As Boolean

Public Declare Function GetLastRCB3Error Lib "RCB3_Interface.dll" Alias " GetLastRCB3Error" () As UInteger
Public Declare Function GetLastRCB3ErrorName Lib "RCB3_Interface.dll" Alias "GetLastRCB3ErrorName" (ByVal myerror As UInteger) As Byte()
Public Declare Function GetLastRCB3FullErrorMessage Lib "RCB3_Interface.dll" Alias "GetLastRCB3FullErrorMessage" () As Byte()

'value of 0 or 1 for each. Usage depends on command too.
Public Declare Function RCB3MakeOption Lib "RCB3_Interface.dll" Alias "RCB3MakeOption" (ByVal storeToEeprom As UInteger, ByVal forceMotion As UInteger) As UInteger

Const RCB3_DEFAULT_OPTION = 0
Const RCB_STORE_TO_EEPROM = 2
Const RCB_FORCE_MOTION = 4

Public Function MakeMotionScenarioIndex(ByRef out_motionScenarioIndex As UInteger, ByVal motion As Integer, ByVal scenario As Integer) As Boolean
    On Error Resume Next
    If (motion = -1 And scenario = -1) Then
        MakeMotionScenarioIndex = False
    End If
    If (motion <> -1 And scenario <> -1) Then
        MakeMotionScenarioIndex = False
    End If
    If (motion < 0 Or motion > 79) Then
        MakeMotionScenarioIndex = False
    End If
    If (scenario < 0 Or scenario > 4) Then
        MakeMotionScenarioIndex = False
    End If
    If (scenario <> -1) Then
        out_motionScenarioIndex = 80 + scenario
    Else
        out_motionScenarioIndex = motion
    End If
    MakeMotionScenarioIndex = True
End Function

'Software Switches
Public Declare Function SetRCB3SoftwareSwitch Lib "RCB3_Interface.dll" Alias "SetRCB3SoftwareSwitch" (ByVal in_rcb3 As UInteger, ByVal in_SWITCH As UInteger, ByVal in_option As UInteger) As Boolean
Public Declare Function GetRCB3SoftwareSwitch Lib "RCB3_Interface.dll" Alias "GetRCB3SoftwareSwitch" (ByVal in_rcb3 As UInteger, ByRef out_SWITCH As UInteger, ByVal in_option As UInteger) As Boolean

'Motion/Scenario Playback
Public Declare Function PlayMotion Lib "RCB3_Interface.dll" Alias "PlayMotion" (ByVal in_rcb3 As UInteger, ByVal in_motionIndex As UInteger, ByVal in_option As UInteger) As Boolean
Public Declare Function PlayScenario Lib "RCB3_Interface.dll" Alias "PlayScenario" (ByVal in_rcb3 As UInteger, ByVal in_scenarioIndex As UInteger, ByVal in_option As UInteger) As Boolean

'Interruption of port motion (one bit set per port to stop)
Public Declare Function StopPorts Lib "RCB3_Interface.dll" Alias "StopPorts" (ByVal in_rcb3 As UInteger, ByVal in_portsToStop As UInteger) As Boolean

'Set Port Speed and Position
Public Declare Function SetPortSpeedAndPosition Lib "RCB3_Interface.dll" Alias "SetPortSpeedAndPosition" (ByVal in_rcb3 As UInteger, ByVal in_portIndex As UInteger, ByVal in_speed As UInteger, ByVal in_position As UInteger, ByVal in_option As UInteger) As Boolean
Public Declare Function SetPortsSpeedAndPosition Lib "RCB3_Interface.dll" Alias "SetPortsSpeedAndPosition" (ByVal in_rcb3 As UInteger, ByVal in_motionIndex As UInteger, ByVal in_slotIndex As UInteger, ByVal in_speed As UInteger, ByVal in_position() As UInteger, ByVal in_option As UInteger) As Boolean  'in_position size = 24
Public Declare Function GetPortsSpeedAndPosition Lib "RCB3_Interface.dll" Alias "GetPortsSpeedAndPosition" (ByVal in_rcb3 As UInteger, ByVal in_option As UInteger, ByVal in_motionIndex As UInteger, ByVal in_slotIndex As UInteger, ByRef out_speed As UInteger, ByVal out_position() As UInteger) As Boolean 'out_position size = 24

Public Declare Function SetPortHomePosition Lib "RCB3_Interface.dll" Alias "SetPortHomePosition" (ByVal in_rcb3 As UInteger, ByVal in_portIndex As UInteger, ByVal in_homePosition As UInteger, ByVal in_option As UInteger) As Boolean
Public Declare Function SetPortsHomePosition Lib "RCB3_Interface.dll" Alias "SetPortsHomePosition" (ByVal in_rcb3 As UInteger, ByVal in_homePositions() As UInteger, ByVal in_option As UInteger) As Boolean 'in_homePositions size = 24
Public Declare Function GetPortsHomePosition Lib "RCB3_Interface.dll" Alias "GetPortsHomePosition" (ByVal in_rcb3 As UInteger, ByVal out_homePositions() As UInteger, ByVal in_option As UInteger) As Boolean 'out_homePosition size = 24

'Teaching
Public Declare Function GetTeachedPosition Lib "RCB3_Interface.dll" Alias "GetTeachedPosition" (ByVal in_rcb3 As UInteger, ByVal out_positions() As UInteger) As Boolean 'out_positions size = 24

'Scenario Edition
Public Declare Function SetMotionInScenario Lib "RCB3_Interface.dll" Alias "SetMotionInScenario" (ByVal in_rcb3 As UInteger, ByVal in_scenarioIndex As UInteger, ByVal in_slotIndex As UInteger, ByVal in_motionIndex As UInteger, ByVal in_option As UInteger) As Boolean
Public Declare Function GetMotionInScenario Lib "RCB3_Interface.dll" Alias "GetMotionInScenario" (ByVal in_rcb3 As UInteger, ByVal in_scenarioIndex As UInteger, ByVal in_slotIndex As UInteger, ByRef out_motionIndex As UInteger) As Boolean

'Start switch/Power on Motion/Scenario
Public Declare Function SetStartupMotion Lib "RCB3_Interface.dll" Alias "SetStartupMotion" (ByVal in_rcb3 As UInteger, ByVal in_motionScenarioIndexSwitchPressed As UInteger, ByVal in_motionScenarioIndexPowerOn As UInteger, ByVal in_option As UInteger) As Boolean
Public Declare Function GetStartupMotion Lib "RCB3_Interface.dll" Alias "GetStartupMotion" (ByVal in_rcb3 As UInteger, ByRef out_motionScenarioIndexSwitchPressed As UInteger, ByRef out_motionScenarioIndexPowerOn As UInteger) As Boolean

'Low Power Configuration (in V)
Public Declare Function SetLowPowerThresholdAndMotion Lib "RCB3_Interface.dll" Alias "SetLowPowerThresholdAndMotion" (ByVal in_rcb3 As UInteger, ByVal in_lowPower As Single, ByVal in_motionScenarioIndex As UInteger, ByVal in_option As UInteger) As Boolean
Public Declare Function GetLowPowerThresholdAndMotion Lib "RCB3_Interface.dll" Alias "GetLowPowerThresholdAndMotion" (ByVal in_rcb3 As UInteger, ByRef out_lowPower As Single, ByRef out_motionScenarioIndex As UInteger) As Boolean

'Scenario Port used:
Public Declare Function SetScenarioPortUsage Lib "RCB3_Interface.dll" Alias "SetScenarioPortUsage" (ByVal in_rcb3 As UInteger, ByVal in_portsUsed As UInteger, ByVal in_option As UInteger) As Boolean
Public Declare Function GetScenarioPortUsage Lib "RCB3_Interface.dll" Alias "GetScenarioPortUsage" (ByVal in_rcb3 As UInteger, ByRef out_portsUsed As UInteger) As Boolean

'RCB3 version (last byte will always be zero
Public Declare Function GetRCBVersion Lib "RCB3_Interface.dll" Alias "GetRCBVersion" (ByVal in_rcb3 As UInteger, ByVal out_version() As Byte) As Boolean 'out_version size = 65

'Analog Input 
'outputs/inputs (In V)

Public Declare Function GetAnalogInputs Lib "RCB3_Interface.dll" Alias "GetAnalogInputs" (ByVal in_rcb3 As UInteger, ByRef out_power As Single, ByRef out_ad1 As Single, ByRef out_ad2 As Single, ByRef out_ad3 As Single) As Boolean

'Channel = 0,1 or 2
'Set to -1V for automatic detection but should not be done with ROM option
Public Declare Function SetAnalogBaseValue Lib "RCB3_Interface.dll" Alias "SetAnalogBaseValue" (ByVal in_rcb3 As UInteger, ByVal in_channel As UInteger, ByVal in_baseValue As Single, ByVal in_option As UInteger) As Boolean
Public Declare Function SetAnalogBaseValues Lib "RCB3_Interface.dll" Alias "SetAnalogBaseValues" (ByVal in_rcb3 As UInteger, ByVal in_baseValue() As Single, ByVal in_option As UInteger) As Boolean 'in_baseValue size = 3
Public Declare Function GetAnalogBaseValues Lib "RCB3_Interface.dll" Alias "GetAnalogBaseValues" (ByVal in_rcb3 As UInteger, ByVal out_baseValue() As Single, ByVal in_option As UInteger) As Boolean 'out_baseValue size = 3

'RealTime Mixing
Public Declare Function SetRealTimeMixing Lib "RCB3_Interface.dll" Alias "SetRealTimeMixing" (ByVal in_rcb3 As UInteger, ByVal in_channel As UInteger, ByVal in_mixingFactors() As UInteger, ByVal in_reverseFactor() As UInteger, ByVal in_set() As UInteger, ByVal in_option As UInteger) As Boolean 'all arrays are of size = 24
Public Declare Function GetRealTimeMixing Lib "RCB3_Interface.dll" Alias "GetRealTimeMixing" (ByVal in_rcb3 As UInteger, ByVal in_channel As UInteger, ByVal out_mixingFactors() As UInteger, ByVal out_reverseFactor() As UInteger, ByVal out_set() As UInteger, ByVal in_option As UInteger) As Boolean 'all arrays are of size = 24

'Configuration of Motion playback on AD Deltas.
'0 or 1 to enable or disable motion playback

Public Declare Function SetMotionPlaybackforAD Lib "RCB3_Interface.dll" Alias "SetMotionPlaybackforAD" (ByVal in_rcb3 As UInteger, ByVal in_motionScenarioIndices() As UInteger, ByVal in_levels() As UInteger, ByVal in_reverse() As UInteger, ByVal in_mixingSwitch() As UInteger, ByVal in_option As UInteger) As Boolean  'all arrays of size = 6
Public Declare Function GetMotionPlaybackforAD Lib "RCB3_Interface.dll" Alias "GetMotionPlaybackforAD" (ByVal in_rcb3 As UInteger, ByVal in_option As UInteger, ByVal out_motionScenarioIndices() As UInteger, ByVal out_levels() As UInteger, ByVal out_reverse() As UInteger, ByVal out_mixingSwitch() As UInteger) As Boolean 'all arrays of size = 6

'Remote Control
Public Declare Function GetDataFromRemoteControl Lib "RCB3_Interface.dll" Alias "GetDataFromRemoteControl" (ByVal in_rcb3 As UInteger, ByVal out_remoteControl() As Byte) As Boolean 'out_remoteControl size = 7
Public Declare Function SetDataFromRemoveControlOverwrite Lib "RCB3_Interface.dll" Alias "SetDataFromRemoveControlOverwrite" (ByVal in_rcb3 As UInteger, ByVal in_option As UInteger, ByVal in_button As UInteger, ByVal in_analogChannel() As UInteger) As Boolean 'in_analogChannel size = 4

'Serial Extension
Public Declare Function SetSerialExtensionAnalogBaseValues Lib "RCB3_Interface.dll" Alias "SetSerialExtensionAnalogBaseValues" (ByVal in_rcb3 As UInteger, ByVal in_baseValue() As Byte, ByVal in_option As UInteger) As Boolean 'in_baseValue size = 4
Public Declare Function GetSerialExtensionAnalogBaseValues Lib "RCB3_Interface.dll" Alias "GetSerialExtensionAnalogBaseValues" (ByVal in_rcb3 As UInteger, ByVal in_option As UInteger, ByVal out_baseValue() As Byte) As Boolean 'out_baseValue size = 4

'ICS Mode
'1 bit per port
Public Declare Function SetPortsInICSMode Lib "RCB3_Interface.dll" Alias "SetPortsInICSMode" (ByVal in_rcb3 As UInteger, ByVal in_portsInICS As UInteger, ByVal in_option As UInteger) As Boolean
Public Declare Function GetPortsInICSMode Lib "RCB3_Interface.dll" Alias "GetPortsInICSMode" (ByVal in_rcb3 As UInteger, ByRef out_portsInICS As UInteger) As Boolean

'RedVersion Port configuration
Public Declare Function SetRedVersionConfiguration Lib "RCB3_Interface.dll" Alias "SetRedVersionConfiguration" (ByVal in_rcb3 As UInteger, ByVal in_portNumber As UInteger, ByVal in_backup As Byte, ByVal in_stretch() As Byte, ByVal in_speed() As Byte, ByVal in_pulsePunch As UInteger, ByVal in_deadBand As UInteger, ByVal in_damping As UInteger, ByVal in_protectionTime As UInteger, ByVal in_reverse As UInteger, ByVal in_lowerPositionLimit As UInteger, ByVal in_upperPositionLimit As UInteger, ByVal in_freeTime As UInteger) As Boolean 'arrays size = 3
Public Declare Function GetRedVersionConfiguration Lib "RCB3_Interface.dll" Alias "GetRedVersionConfiguration" (ByVal in_rcb3 As UInteger, ByVal in_portNumber As UInteger, ByRef out_backup As Byte, ByVal out_stretch() As Byte, ByVal out_speed() As Byte, ByRef out_pulsePunch As UInteger, ByRef out_deadBand As UInteger, ByRef out_damping As UInteger, ByRef out_protectionTime As UInteger, ByRef out_reverse As UInteger, ByRef out_lowerPositionLimit As UInteger, ByRef out_upperPositionLimit As UInteger, ByRef out_freeTime As UInteger) As Boolean 'arrays size = 3

'Serial Servo Port configuration

Public Declare Function SetSerialServoConfiguration Lib "RCB3_Interface.dll" Alias "SetSerialServoConfiguration" (ByVal in_rcb3 As UInteger, ByVal in_portNumber As UInteger, ByVal in_backup As Byte, ByVal in_stretch() As Byte, ByVal in_speed() As Byte, ByVal in_pulsePunch As UInteger, ByVal in_deadBand As UInteger, ByVal in_damping As UInteger, ByVal in_protectionTime As UInteger, ByVal in_reverse As UInteger, ByVal in_lowerPositionLimit As UInteger, ByVal in_upperPositionLimit As UInteger, ByVal in_freeTime As UInteger, ByVal in_stretchSIO As UInteger, ByVal in_speedSIO As UInteger, ByVal in_offset As UInteger, ByVal in_ID As UInteger, ByVal in_p1 As UInteger, ByVal in_p2 As UInteger, ByVal in_n1 As UInteger, ByVal in_n2 As UInteger) As Boolean 'arrays size = 3
Public Declare Function GetSerialServoConfiguration Lib "RCB3_Interface.dll" Alias "GetSerialServoConfiguration" (ByVal in_rcb3 As UInteger, ByVal in_portNumber As UInteger, ByRef out_backup As Byte, ByVal out_stretch() As Byte, ByVal out_speed() As Byte, ByRef out_pulsePunch As UInteger, ByRef out_deadBand As UInteger, ByRef out_damping As UInteger, ByRef out_protectionTime As UInteger, ByRef out_reverse As UInteger, ByRef out_lowerPositionLimit As UInteger, ByRef out_upperPositionLimit As UInteger, ByRef out_freeTime As UInteger, ByRef out_stretchSIO As UInteger, ByRef out_speedSIO As UInteger, ByRef out_offset As UInteger, ByRef out_ID As UInteger, ByRef out_p1 As UInteger, ByRef out_p2 As UInteger, ByRef out_n1 As UInteger, ByRef out_n2 As UInteger) As Boolean 'arrays size = 3

Public Declare Function GetSerialServoStretch Lib "RCB3_Interface.dll" Alias "GetSerialServoStretch" (ByVal in_rcb3 As UInteger, ByVal out_stretch() As UInteger) As Boolean 'out_stretch size = 24
Public Declare Function GetSerialServoSpeed Lib "RCB3_Interface.dll" Alias "GetSerialServoSpeed" (ByVal in_rcb3 As UInteger, ByVal out_speed() As UInteger) As Boolean 'out_speed size = 24


'PDS Servo Port configuration
Public Declare Function SetPDSConfiguration Lib "RCB3_Interface.dll" Alias "SetPDSConfiguration" (ByVal in_rcb3 As UInteger, ByVal in_portNumber As UInteger, ByVal in_backup As Byte, ByVal in_stretch As UInteger, ByVal in_speed As UInteger, ByVal in_pulsePunch As UInteger, ByVal in_deadBand As UInteger, ByVal in_damping As UInteger, ByVal in_failSafeTime As UInteger, ByVal in_lowerPositionLimit As UInteger, ByVal in_upperPositionLimit As UInteger) As Boolean
Public Declare Function GetPDSConfiguration Lib "RCB3_Interface.dll" Alias "GetPDSConfiguration" (ByVal in_rcb3 As UInteger, ByVal in_portNumber As UInteger, ByRef out_backup As Byte, ByRef out_stretch As UInteger, ByRef out_speed As UInteger, ByRef out_pulsePunch As UInteger, ByRef out_deadBand As UInteger, ByRef out_damping As UInteger, ByRef out_failSafeTime As UInteger, ByRef out_lowerPositionLimit As UInteger, ByRef out_upperPositionLimit As UInteger) As Boolean

Public Declare Function SetPDSPortsInUse Lib "RCB3_Interface.dll" Alias "SetPDSPortsInUse" (ByVal in_rcb3 As UInteger, ByVal in_pdsPortsInUse As UInteger, ByVal in_option As UInteger) As Boolean
Public Declare Function GetPDSPortsInUse Lib "RCB3_Interface.dll" Alias "GetPDSPortsInUse" (ByVal in_rcb3 As UInteger, ByRef out_pdsPortsInUse As UInteger) As Boolean


'Default Port Position
Public Declare Function SetPortsDefaultPosition Lib "RCB3_Interface.dll" Alias "SetPortsDefaultPosition" (ByVal in_rcb3 As UInteger, ByVal in_defaultPositions() As UInteger) As Boolean 'in_defaultPositions size = 24
Public Declare Function GetPortsDefaultPosition Lib "RCB3_Interface.dll" Alias "GetPortsDefaultPosition" (ByVal in_rcb3 As UInteger, ByVal out_defaultPositions() As UInteger) As Boolean 'out_defaultPositions size = 24

'Port Names
Public Declare Function SetPortName Lib "RCB3_Interface.dll" Alias "SetPortName" (ByVal in_rcb3 As UInteger, ByVal in_option As UInteger, ByVal in_portIndex As UInteger, ByVal in_name() As Byte) As Boolean 'name size = 32
Public Declare Function GetPortName Lib "RCB3_Interface.dll" Alias "GetPortName" (ByVal in_rcb3 As UInteger, ByVal in_portIndex As UInteger, ByVal out_name() As Byte) As Boolean 'name size = 32


'Motion/Scenario Edition
Public Declare Function SetMotionScenarioData Lib "RCB3_Interface.dll" Alias "SetMotionScenarioData" (ByVal in_rcb3 As UInteger, ByVal in_option As UInteger, ByVal in_motionScenarioIndex As UInteger, ByVal in_moveCount As UInteger, ByVal in_portsUsed As UInteger, ByVal in_minute As UInteger, ByVal in_hour As UInteger, ByVal in_day As UInteger, ByVal in_month As UInteger, ByVal in_button As UInteger, ByVal in_name() As Byte) As Boolean 'name size = 32
Public Declare Function GetMotionScenarioData Lib "RCB3_Interface.dll" Alias "GetMotionScenarioData" (ByVal in_rcb3 As UInteger, ByVal in_motionScenarioIndex As UInteger, ByRef out_moveCount As UInteger, ByRef out_portsUsed As UInteger, ByRef out_minute As UInteger, ByRef out_hour As UInteger, ByRef out_day As UInteger, ByRef out_month As UInteger, ByRef out_button As UInteger, ByVal out_name() As Byte) As Boolean 'name size = 32

'Timer based Notification (0.5s) when activated in the Software Switches.
Public Declare Function GetLastDataFromRemoteControl Lib "RCB3_Interface.dll" Alias "GetLastDataFromRemoteControl" (ByVal in_rcb3 As UInteger, ByVal out_remoteData() As Byte) As Boolean 'out_remoteData size = 7
Public Declare Function GetLastPortsStatus Lib "RCB3_Interface.dll" Alias "GetLastPortsStatus" (ByVal in_rcb3 As UInteger, ByRef out_portsDone As UInteger, ByVal out_portMotionDone() As UInteger) As Boolean 'out_portMotionDone size = 24
Public Declare Function GetLastPowerAndADValues Lib "RCB3_Interface.dll" Alias "GetLastPowerAndADValues" (ByVal in_rcb3 As UInteger, ByRef out_power As Single, ByRef out_ad1 As Single, ByRef out_ad2 As Single, ByRef out_ad3 As Single) As Boolean

'Serial  Extension
Public Declare Function SendBytesToCOMPort Lib "RCB3_Interface.dll" Alias "SendBytesToCOMPort" (ByVal in_rcb3 As UInteger, ByVal in_buffer_to_send() As Byte, ByVal in_buffer_size As UInteger) As Boolean
Public Declare Function ReceiveBytesFromCOMPort Lib "RCB3_Interface.dll" Alias "ReceiveBytesFromCOMPort" (ByVal in_rcb3 As UInteger, ByVal out_buffer_to_send() As Byte, ByVal in_buffer_size As UInteger) As Boolean
