#ifndef _FREERTOS_H
#define _FREERTOS_H

#include "RC522.h"
#include "FINGER.h"
#include "Arduino.h"
#include "MQTT.H"



enum Screen_Mode
{
    Finger_authentication,
    Finger_Register,
    RC522_Register
};
void All_Init(void);
void ALL_CreateTasks(void);

#endif