/*
 * @Author: 'daddasd' '3323169544@qq.com'
 * @Date: 2025-03-15 14:51:43
 * @LastEditors: 'daddasd' '3323169544@qq.com'
 * @LastEditTime: 2025-03-16 18:37:43
 * @FilePath: \EN_LOOK\lib\OTA\OTA.H
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _OTA_H
#define _OTA_H


#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h> // 使用正确的头文件
#include <ArduinoOTA.h>
#include <esp_ota_ops.h>
#include <Preferences.h> // 用于flash存储

void Get_Version(void);
void Write_Version();
void update_started();
void update_finished();
void update_progress(int cur, int total);
void update_error(int err);
void updateBin();

#endif