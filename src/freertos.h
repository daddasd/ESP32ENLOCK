/*
 * @Author: 'daddasd' '3323169544@qq.com'
 * @Date: 2025-03-13 14:59:43
 * @LastEditors: 'daddasd' '3323169544@qq.com'
 * @LastEditTime: 2025-04-11 14:06:58
 * @FilePath: \EN_LOOK\src\freertos.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _FREERTOS_H
#define _FREERTOS_H

#include "RC522.h"
#include "FINGER.h"
#include "Arduino.h"
#include "MQTT.H"
#include "screen.h"
#include <ESP32Servo.h>

enum Screen_Mode
{
    Finger_authentication = 3,
    Finger_Register = 7,
    RC522_Register = 5
};
void All_Init(void);
void ALL_CreateTasks(void);

#endif