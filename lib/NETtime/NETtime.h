#ifndef _NETTIME_H
#define _NETTIME_H

#include <Arduino.h>
#include "WiFi.h"
#include <NTPClient.h>
#include <WiFiUdp.h>


extern int year;
extern int month;
extern int day;
extern int hour;
extern int minute;
extern int second;
extern float LUX;

extern const char *ssid;
extern const char *password;
void Linked_Network();
void Get_NET_Time();

#endif