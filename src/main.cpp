#include "Arduino.h"
#include "MQTT.h"
#include "Finger.h"
#include "RC522.h"
#include "OTA.h"
#include "screen.h"
#include "freertos.h"
#include "NETtime.h"
void setup()
{
  All_Init();
  Get_Version();
  ALL_CreateTasks();
}

void loop()
{
 //delay(1000);
  vTaskDelay(pdMS_TO_TICKS(200));
}