#ifndef _FINGER_H_
#define _FINGER_H_

#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>


void Finger_Init(void);
bool Finger_Authentication();
uint8_t findFreeID();
bool Finger_Enroll();

#endif