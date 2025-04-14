/*
 * @Author: 'lin' '11252700+display23@user.noreply.gitee.com'
 * @Date: 2025-03-13 12:54:59
 * @LastEditors: 'daddasd' '3323169544@qq.com'
 * @LastEditTime: 2025-03-16 18:57:05
 * @FilePath: \EN_LOOK\lib\RC522\RC522.cpp
 * @Description: RC522读取和存储卡号
 */

#include "RC522.h"

#define RST_PIN 9 // RST引脚接GPIO9
#define SS_PIN 15 // SDA引脚接GPIO15

MFRC522 mfrc522(SS_PIN, RST_PIN); // 创建MFRC522实例
Preferences preferences1;
String storedCards; // 全局变量缓存卡号列表

void RC522_Init(void)
{
    Serial.begin(115200);
    SPI.begin(14, 12, 13, SS_PIN); // 初始化SPI，指定SCK=14,MISO=12,MOSI=13,SS=15
    mfrc522.PCD_Init();            // 初始化MFRC522
    RC522_Load_IDs();              // 一次性读取所有ID
    Serial.println("RFID读卡器已就绪...");
}

String RC522_Read(void)
{
    // 检测是否有新卡片
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        return "";
    }

    // 读取卡片信息
    if (!mfrc522.PICC_ReadCardSerial())
    {
        return "";
    }

    // 构建卡号字符串
    String cardID = "";
    Serial.print("卡片UID：");
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        // 格式化卡号为16进制字符串
        cardID += String(mfrc522.uid.uidByte[i], HEX);
        if (i < mfrc522.uid.size - 1)
        {
            cardID += ":";
        }
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();

    // 停止读卡
    mfrc522.PICC_HaltA();

    return cardID;
}

bool isCardStored(String cardID, String storedCards)
{
    // 检查存储字符串中是否包含目标卡号
    int index = storedCards.indexOf(cardID);
    if (index >= 0)
    {
        // 判断是否为完整匹配（防止子串误匹配）
        int before = (index == 0) ? 0 : storedCards[index - 1];
        int after = (index + cardID.length() == storedCards.length()) ? 0 : storedCards[index + cardID.length()];
        if ((before == 0 || before == ',') && (after == 0 || after == ','))
        {
            return true;
        }
    }
    return false;
}

bool RC522_Save_ID(void)
{
    // 打开命名空间 Card_ID（读写模式）
    preferences1.begin("Card_ID", false);

    // 读取当前卡号
    String cardID = RC522_Read();
    if (cardID == "")
    {
        Serial.println("未读取到有效卡号！");
        preferences1.end();
        return false; // 如果没有读取到卡号，返回 false
    }

    // 获取已存储的卡号列表
    String storedCards = preferences1.getString("cardList", "");

    // 判断卡号是否已存在
    if (isCardStored(cardID, storedCards))
    {
        Serial.println("卡号已存在，跳过存储！");
        preferences1.end();
        return false; // 如果卡号已存在，返回 false
    }
    else
    {
        // 拼接新的卡号到已存储的卡号列表
        if (storedCards != "")
        {
            storedCards += ",";
        }
        storedCards += cardID;

        // 存储更新后的卡号列表
        preferences1.putString("cardList", storedCards);
        RC522_Load_IDs();
        Serial.println("卡号已保存到Flash中！");
    }

    // 关闭命名空间
    preferences1.end();

    return true; // 如果成功保存卡号，返回 true
}

bool RC522_Load_ID(String cardID)
{
    // 打开命名空间 Card_ID（只读模式）
    preferences1.begin("Card_ID", true);

    // 从Flash中读取所有卡号
    String storedCards = preferences1.getString("cardList", "无数据");
    // Serial.print("存储的卡号列表：");
    // Serial.println(storedCards);

    // 检查是否存在指定卡号
    bool exists = isCardStored(cardID, storedCards);

    // 关闭命名空间
    preferences1.end();

    // 返回卡号是否存在的结果
    return exists;
}

void RC522_Load_IDs()
{
    // 打开命名空间 Card_ID（只读模式）
    preferences1.begin("Card_ID", true);
    // 从Flash中读取所有卡号
    storedCards = preferences1.getString("cardList", "无数据");
    Serial.print("已加载的卡号列表：");
    Serial.println(storedCards);
    // 关闭命名空间
    preferences1.end();
}

bool RC522_App(RC522_Mode mode)
{
    String cardID; // 将 cardID 的声明移到 switch 语句外部
    switch (mode)
    {
    case RC522_Mode::authentication: // 认证ID
        cardID = RC522_Read();       // 不要在此重新声明变量
        if (cardID != "")
        {
            if (isCardStored(cardID, storedCards))
            {
                Serial.println("卡号存在！");
                return true;
            }
            else
            {
                Serial.println("卡号不存在！");
                return false;
            }
        }
        break; // 这里的 break 用于退出 switch

    case RC522_Mode::establish: // 创建ID
        if (RC522_Save_ID())
        {
            Serial.println("卡号保存成功！");
            RC522_Load_IDs();
            return true;
        }
        else
        {
            Serial.println("卡号保存失败！");
            return false;
        }
        break; // 这里的 break 用于退出 switch
    }
    // default 语句必须在 switch 内部
    return false; // 如果没有匹配的情况，返回 false
}