/*
 * File:         BlueSmirf.h
 * Date:         January, 2007
 * Description:  Class to setup the com via the BlueSmirf Module
 * Author:       Laurent Lessieux
 *
 * Copyright (c) 2006 Laurent Lessieux
 *               www.lessieux.com
 */

#pragma once

class com;

class BlueSmirf
{
public:
    static com *CreateComPortViaBlueSmirf(int comPort,int speed,bool autodetectSpeed,bool writeConfigToFlash);
private:
    BlueSmirf();
};


