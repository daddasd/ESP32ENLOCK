#ifndef _RC522_H
#define _RC522_H

#include <SPI.h>
#include <MFRC522.h>
#include "Arduino.h"
#include <Preferences.h>

extern String storedCards; // 全局变量缓存卡号列表

enum class RC522_Mode
{
    authentication,
    establish
};

void RC522_Load_IDs();
void RC522_Init();
String RC522_Read(void);
bool RC522_Save_ID(void);
bool RC522_Load_ID(String cardID);
bool isCardStored(String cardID, String storedCards);
bool RC522_App(RC522_Mode mode);

#endif