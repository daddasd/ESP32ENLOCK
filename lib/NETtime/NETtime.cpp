/*
 * @Author: 'daddasd' '3323169544@qq.com'
 * @Date: 2025-03-18 20:36:07
 * @LastEditors: 'daddasd' '3323169544@qq.com'
 * @LastEditTime: 2025-03-18 20:37:17
 * @FilePath: \EN_LOOK\lib\NETtime\NETtime.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "NetTime.h"

const char *ssid = "XZZ";
const char *password = "qwer1234qwer";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "203.107.6.88", 8 * 3600, 60000);

int year;
int month;
int day;
int hour;
int minute;
int second;

// 时间结构体定义
struct DateTime
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};

// 将Unix时间戳转换为日期时间（考虑时区偏移）
DateTime getDateTime(unsigned long epochTime)
{
    DateTime dt;
    // 计算调整后的总秒数（包括时区）
    // epochTime += 8 * 3600; // UTC+8时区调整

    // 计算天数
    int days = epochTime / 86400L;

    // 计算年
    dt.year = 1970;
    while (1)
    {
        int leap = (dt.year % 4 == 0 && dt.year % 100 != 0) || (dt.year % 400 == 0);
        int yearDays = leap ? 366 : 365;
        if (days >= yearDays)
        {
            days -= yearDays;
            dt.year++;
        }
        else
        {
            break;
        }
    }

    // 计算月日
    int monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((dt.year % 4 == 0 && dt.year % 100 != 0) || (dt.year % 400 == 0))
    {
        monthDays[1] = 29;
    }

    dt.month = 1;
    for (int i = 0; i < 12; i++)
    {
        if (days >= monthDays[i])
        {
            days -= monthDays[i];
            dt.month++;
        }
        else
        {
            break;
        }
    }
    dt.day = days + 1; // 天数从1开始

    // 计算时分秒
    long remaining = epochTime % 86400L;
    dt.hour = remaining / 3600;
    remaining %= 3600;
    dt.minute = remaining / 60;
    dt.second = remaining % 60;

    return dt;
}

void Linked_Network(void)
{
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    int wifiRetry = 0;
    while (WiFi.status() != WL_CONNECTED && wifiRetry < 10)
    { // 增加超时判断
        delay(500);
        Serial.print(".");
        wifiRetry++;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi 连接成功");
    }
    else
    {
        Serial.println("\nWiFi 连接失败！");
        return;
    }

    timeClient.begin();
    int ntpRetry = 0;
    while (!timeClient.update() && ntpRetry < 5)
    { // 增加 NTP 重试
        timeClient.forceUpdate();
        delay(1000);
        ntpRetry++;
    }
}

void Get_NET_Time(void)
{
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();

    DateTime dt = getDateTime(epochTime);
    year = dt.year;
    month = dt.month;
    day = dt.day;
    hour = dt.hour;
    minute = dt.minute;
    second = dt.second;
    // Serial.printf("当前时间：%04d-%02d-%02d %02d:%02d:%02d\n",
    //               dt.year, dt.month, dt.day,
    //               dt.hour, dt.minute, dt.second);
}
