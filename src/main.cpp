/*
 * @Author: 'daddasd' '3323169544@qq.com'
 * @Date: 2025-03-12 15:17:26
 * @LastEditors: 'daddasd' '3323169544@qq.com'
 * @LastEditTime: 2025-04-10 20:00:47
 * @FilePath: \EN_LOOK\src\main.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "Arduino.h"
#include "MQTT.h"
#include "Finger.h"
#include "RC522.h"
#include "OTA.h"
#include "screen.h"
#include "NETtime.h"
void setup()
{
  All_Init();
  Get_Version();
  ALL_CreateTasks();
}

void loop()
{
  // delay(500);
  vTaskDelay(pdMS_TO_TICKS(5000));
}