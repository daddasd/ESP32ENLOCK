#include "Arduino.h"
#include "MQTT.h"
#include "Finger.h"
#include "RC522.h"
#include "OTA.h"

void setup()
{
  All_Init();
  Get_Version();
  ALL_CreateTasks();
  // MQTT_init();
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(5000));
}