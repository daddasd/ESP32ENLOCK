/*
 * @Author: 'daddasd' '3323169544@qq.com'
 * @Date: 2025-04-09 21:27:47
 * @LastEditors: 'daddasd' '3323169544@qq.com'
 * @LastEditTime: 2025-04-14 11:05:21
 * @FilePath: \EN_LOOK\lib\SCREEN\screen.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "screen.h"

// 定义软串口对象：RX=2, TX=3
SoftwareSerial ScreenSerial(2, 3); // RX, TX
void Screen_Init(void)
{
    ScreenSerial.begin(115200);
}

void sendTimeToDisplay()
{
    // 1. 获取网络时间（确保该函数已更新全局时间变量）
    Get_NET_Time();

    // 2. 直接使用全局变量（避免冗余复制，除非有特殊需求）
    char timeStr[20];
    sprintf(timeStr, "%04d-%02d-%02d %02d:%02d",
            year, month, day, hour, minute);

    // 3. 发送到串口屏（关键修正点：使用print而非println避免换行符干扰）
    ScreenSerial.print("time.txt=\""); // 指令头
    ScreenSerial.print(timeStr);       // 时间数据
    ScreenSerial.print("\"");          // 闭合引号（不用println!）

    // 4. 发送3个0xFF结束符（严格按协议）
    ScreenSerial.write(0xFF);
    ScreenSerial.write(0xFF);
    ScreenSerial.write(0xFF);

    // 可选：调试输出（通过硬件串口监控）
    // Serial.print("[Debug] Sent: t0.txt=\"");
    // Serial.print(timeStr);
    // Serial.println("\" + 0xFF*3");
}
void sendCommandToDisplay(const char *command)
{
    // 1. 发送完整指令字符串
    ScreenSerial.print(command);

    // 2. 发送3个0xFF结束符（严格按协议）
    ScreenSerial.write(0xFF);
    ScreenSerial.write(0xFF);
    ScreenSerial.write(0xFF);

    // 可选调试输出（取消注释即可使用）
    // Serial.print("[Debug] Sent: ");
    // Serial.print(command);
    // Serial.println(" + 0xFF*3");
}

