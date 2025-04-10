#ifndef _SCREEN_H
#define _SCREEN_H


#include <SoftwareSerial.h>
#include "Arduino.h"
#include "NETtime.h"

extern SoftwareSerial ScreenSerial;

void Screen_Init(void);
void sendTimeToDisplay();
void Serial_Screen_Task(void *param);
void sendCommandToDisplay(const char *command);
#endif