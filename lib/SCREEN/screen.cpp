#include "screen.h"


// 定义软串口对象：RX=2, TX=3
SoftwareSerial ScreenSerial(2, 3); // RX, TX

void Screen_Init(void)
{
    ScreenSerial.begin(9600);
}

void sendTimeToDisplay()
{
    // 1. 获取网络时间（确保该函数已更新全局时间变量）
    Get_NET_Time();

    // 2. 直接使用全局变量（避免冗余复制，除非有特殊需求）
    char timeStr[20];
    sprintf(timeStr, "%04d-%02d-%02d %02d:%02d:%02d",
            year, month, day, hour, minute, second);

    // 3. 发送到串口屏（关键修正点：使用print而非println避免换行符干扰）
    ScreenSerial.print("t0.txt=\""); // 指令头
    ScreenSerial.print(timeStr);     // 时间数据
    ScreenSerial.print("\"");        // 闭合引号（不用println!）

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