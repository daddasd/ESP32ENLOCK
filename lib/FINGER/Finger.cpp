#include "Finger.h"

// 在头文件中定义常量
#define FINGERPRINT_CAPACITY 200 // 根据传感器实际容量修改
#define FINGERPRINT_NOFREEINDEX 0xFF
HardwareSerial mySerial(2); // 使用 ESP32 的硬件串口2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void Finger_Init(void)
{
    Serial.begin(115200);
    mySerial.begin(57600, SERIAL_8N1, 16, 17); // RX = GPIO16, TX = GPIO17
    finger.begin(57600);
    const uint32_t FingerWindows = 8000; // 认证窗口，单位毫秒（这里为8秒）
    uint32_t startTime = millis();
    while(millis()-startTime<FingerWindows)
    {
        if (finger.verifyPassword())
        {
            Serial.println("指纹传感器初始化成功！");
            break;
        }
        else
        {
            Serial.println("未找到指纹传感器 :(");
        }
    }
}

bool Finger_Authentication()
{
    Serial.println("请将手指放在传感器上...");
    while (finger.getImage() != FINGERPRINT_OK)
        vTaskDelay(100);
    if (finger.image2Tz() != FINGERPRINT_OK)
    {
        Serial.println("图像转换失败");
        return false;
    }

    if (finger.fingerFastSearch() != FINGERPRINT_OK)
    {
        Serial.println("未找到匹配的指纹");
        return false;
    }
    Serial.print("找到ID #");
    Serial.print(finger.fingerID);
    Serial.print("，匹配度：");
    Serial.println(finger.confidence);
    return true;
}
// 查找第一个可用的指纹ID
uint8_t findFreeID()
{
    for (uint8_t id = 0; id < FINGERPRINT_CAPACITY; id++)
    { // 假设最大容量为200
        if (finger.loadModel(id) != FINGERPRINT_OK)
        {
            return id; // 返回第一个不可用模板的ID（即空闲位置）
        }
    }
    return FINGERPRINT_NOFREEINDEX; // 0xFF表示没有可用ID
}

bool Finger_Enroll()
{
    uint8_t id = findFreeID();

    if (id == FINGERPRINT_NOFREEINDEX)
    {
        Serial.println("指纹数据库已满，无法注册新指纹");
        return false;
    }
    Serial.println("准备注册新指纹...");
    Serial.print("分配到的ID: ");
    Serial.println(id);

    // 第一次指纹采集
    Serial.println("请放置手指...");
    while (finger.getImage() != FINGERPRINT_OK)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    if (finger.image2Tz(1) != FINGERPRINT_OK)
    {
        Serial.println("第一次图像转换失败");
        return false;
    }
    Serial.println("采集完成，请移开手指");
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 第二次指纹采集
    Serial.println("请再次放置同一手指...");
    while (finger.getImage() != FINGERPRINT_OK)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    if (finger.image2Tz(2) != FINGERPRINT_OK)
    {
        Serial.println("第二次图像转换失败");
        return false;
    }

    // 创建指纹模型
    if (finger.createModel() != FINGERPRINT_OK)
    {
        Serial.println("指纹特征不匹配，请重试");
        return false;
    }

    // 存储指纹模板
    if (finger.storeModel(id) != FINGERPRINT_OK)
    {
        Serial.println("存储指纹失败");
        return false;
    }

    Serial.print("指纹注册成功！ID: ");
    Serial.println(id);
    return true;
}

