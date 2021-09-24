#pragma once

class com;

class RCB3Interface
{
public:
    RCB3Interface(int comPort,int speed = 115200);
    RCB3Interface(int comPort,int speed ,bool autodetectSpeed ,bool writeConfigToFlash );

    //For Ezurio Wism
    RCB3Interface(char *address,int port);

    ~RCB3Interface();

    void CheckUpdate();

    bool ReadRemoteControlData(UBYTE *buffer = NULL,int *read0 = NULL);
    bool ReadAnalogData(UBYTE *buffer = NULL,int *read0 = NULL);
    bool ReadPortsData(UBYTE *buffer = NULL,int *read0 = NULL);

    bool WaitForSignal();
    bool SendCommand(UBYTE *command,UINT size);
    bool ReadAnswer(UBYTE *command,UINT size);

    bool m_receivingNotification;
    int m_model;
    com *m_comPort;
    float m_lastPower;
    float m_lastAD1;
    float m_lastAD2;
    float m_lastAD3;
    UINT m_lastPortsDone;
    UINT m_lastPortsMotion[24];
    UBYTE m_lastRemoteControlData[7];

private:
    static bool m_possibleCollision[256];
    int m_lastCommand;
};
