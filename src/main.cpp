#include "Arduino.h"
#include "MQTT.h"
#include "Finger.h"
#include "RC522.h"
#include "OTA.h"
#include "screen.h"
#include "freertos.h"
#include "NETtime.h"
#include "esp_task_wdt.h"

// 定义看门狗超时时间（单位：毫秒）
#define WDT_TIMEOUT 20000 // 20秒

// 定义任务句柄
TaskHandle_t taskHandle = NULL;

// 看门狗喂狗任务
void watchdogTask(void *pvParameters)
{
  while (true)
  {
    // 喂狗
    esp_task_wdt_reset();
    // 任务....
    Serial.println(".....");
  }
}

void setup()
{
  Serial.begin(115200);
  All_Init();
  pinMode(2, INPUT_PULLUP); // 启用内部上拉电阻
  Get_Version();
  ALL_CreateTasks();
  // 初始化看门狗定时器
  esp_task_wdt_init(WDT_TIMEOUT, true);

  // 创建任务
  xTaskCreate(watchdogTask, "Watchdog Task", 2048, NULL, 1, &taskHandle);

  // 将看门狗加到任务
  esp_task_wdt_add(taskHandle);
}

void loop()
{
  vTaskDelay(200);
  // 在主循环中可以执行其他任务
}
