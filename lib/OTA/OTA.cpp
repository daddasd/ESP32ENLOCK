/*
 * @Author: 'daddasd' '3323169544@qq.com'
 * @Date: 2025-04-10 21:16:52
 * @LastEditors: 'daddasd' '3323169544@qq.com'
 * @LastEditTime: 2025-04-14 21:00:29
 * @FilePath: \EN_LOOK\lib\OTA\OTA.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "OTA.h"

#define CUSTOM_VERSION "v3.0.1"

String upUrl = "http://bin.bemfa.com/b/1BcY2IwNDJlZGE1MWY4NGU4MDliYWJhZDBiMDAzODc2NWY=ENLOOK002.bin"; // 更新的地址

Preferences preferences;
/**********************************/

void Get_Version(void)
{
    preferences.begin("firmware", false); // 可读写模式
    // 直接将自定义版本写入flash
    preferences.putString("version", CUSTOM_VERSION);
    String storedVersion = preferences.getString("version", "");
    Serial.println("Stored firmware version: " + storedVersion);
    preferences.end();
}

void Write_Version()
{
    preferences.begin("firmware", false); // 可读写模式
    preferences.putString("version", CUSTOM_VERSION);
    Serial.println("Stored new firmware version: " + String(CUSTOM_VERSION));
}

void update_started()
{
    Serial.println("CALLBACK: HTTP update process started");
}

void update_finished()
{
    Serial.println("CALLBACK: HTTP update process finished");
}

void update_progress(int cur, int total)
{
    Serial.printf("CALLBACK: Progress %d of %d bytes...\n", cur, total);
}

void update_error(int err)
{
    Serial.printf("CALLBACK: Update error %d\n", err);
}

void updateBin()
{
    WiFiClient UpdateClient;

    // 设置OTA回调函数（使用全局实例对象 httpUpdate）
    httpUpdate.onStart(update_started);
    httpUpdate.onEnd(update_finished);
    httpUpdate.onProgress(update_progress);
    httpUpdate.onError(update_error);
    // 执行固件更新
    t_httpUpdate_return ret = httpUpdate.update(UpdateClient, upUrl);
    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
        Serial.println("[update] Update failed.");
        break;
    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("[update] No updates.");
        break;
    case HTTP_UPDATE_OK:
        Serial.println("[update] Update successful.");
        Write_Version();
        break;
    }
}
