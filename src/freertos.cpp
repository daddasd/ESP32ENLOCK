/*
 * @Author: 'lin' '11252700+display23@user.noreply.gitee.com'
 * @Date: 2025-03-13 14:59:11
 * @LastEditors: 'daddasd' '3323169544@qq.com'
 * @LastEditTime: 2025-04-17 20:49:31
 * @FilePath: \EN_LOOK\src\freertos.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "freertos.h"

TaskHandle_t xSerialScreenTaskHandle = NULL;
TaskHandle_t xRC522RegisteTaskHandle = NULL;
TaskHandle_t xRC522AuthenticationTaskHandle = NULL;
SemaphoreHandle_t xServoUnlockSemaphore = NULL;           // 创建一个二进制信号号量用于解锁
SemaphoreHandle_t xFinger_AuthenticationSemaphore = NULL; // 创建一个二进制信号号量用于指纹认证
SemaphoreHandle_t xFinger_EnrollSemaphore = NULL;         // 创建一个二进制信号号量用于指纹注册
Servo myServo;                                            // 创建舵机对象
const int servoPin = 1;                                   // 推荐使用 GPIO2（避免 GPIO1）
uint8_t RX_Data[10] = {0};
void UNLOCK()
{
    myServo.write(0); // 舵机转到 0 度
    vTaskDelay(pdMS_TO_TICKS(1000));
    myServo.write(90); // 舵机转到 180 度
    vTaskDelay(pdMS_TO_TICKS(1000));
}
void All_Init(void)
{
    myServo.attach(servoPin); // 初始化舵机，默认频率为 50Hz
    RC522_Init();
    Linked_Network();
    delay(1000);
    Finger_Init();
    delay(1000);
    MQTT_init();
    Screen_Init();
}

/**
 * @brief RC522认证任务
 *
 * @param param
 */
void RC522_Authentication_Task(void *param)
{
    while (true)
    {
        if (RC522_Load_ID(RC522_Read()))
        {
            sendCommandToDisplay("page attestationYES");
            xSemaphoreGive(xServoUnlockSemaphore); // 解锁成功释放信号量
        }
        else
        {
            // Serial.println("RC522 认证失败...");
        }
        vTaskDelay(pdMS_TO_TICKS(20));
        /* code */
    }
}
/**
 * @brief RC522注册任务
 *
 * @param param
 */
void RC522_Register_Task(void *param)
{
    const uint16_t timeout = 50; // 50次 * 200ms = 10秒
    uint16_t remaining_time = timeout;

    while (remaining_time--)
    {
        if (RC522_Save_ID())
        {
            // 成功逻辑
            vTaskDelete(NULL); // 删除自身
            return;            // 安全退出
        }
        else
        {
            Serial.printf("请重试");
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // 超时逻辑（10秒未成功）
    Serial.printf("注册超时");
    vTaskResume(xSerialScreenTaskHandle); // 确保恢复其他任务
    vTaskResume(xRC522AuthenticationTaskHandle);
    vTaskDelete(NULL); // 删除自身
}
/**
 * @brief 指纹模块认证任务（修改为执行一次后删除自身）
 * @param param
 */
void Finger_Authentication_Task(void *param)
{
    const uint32_t AuthenticationWindow = 8000; // 认证窗口 8 秒

    // 仅执行一次认证流程（不再用 while(true) 循环）

    uint32_t startTime = millis();
    bool AuthenticationSuccess = false;
    vTaskSuspend(xSerialScreenTaskHandle); // 挂起串口屏任务

    // 在 8 秒窗口期内尝试认证
    while (millis() - startTime < AuthenticationWindow)
    {
        if (Finger_Authentication())
        {
            Serial.println("指纹认证成功");
            sendCommandToDisplay("page attestationYES");
            AuthenticationSuccess = true;
            break;
        }
        else
        {
            Serial.println("等待手指放入...");
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // 恢复串口屏任务（无论成功或超时）
    vTaskResume(xSerialScreenTaskHandle);

    // 根据结果触发舵机解锁或显示超时
    if (AuthenticationSuccess)
    {
        xSemaphoreGive(xServoUnlockSemaphore);
    }
    else
    {
        sendCommandToDisplay("page attestationNO");
        Serial.println("指纹认证任务超时。");
    }

    // 删除自身任务（确保所有资源已清理）
    vTaskDelete(NULL);
}
/**
 * @brief 指纹注册任务（修改为执行一次后删除自身）
 * @param param
 */
void Finger_Enroll_Task(void *param)
{
    const uint32_t EnrollWindow = 14000; // 注册窗口 14 秒
    // 仅执行一次注册流程（不再用 while(true) 循环）
    uint32_t startTime = millis();
    bool EnrollSuccess = false;
    vTaskSuspend(xSerialScreenTaskHandle); // 挂起串口屏任务
    // 在 14 秒窗口期内尝试注册
    while (millis() - startTime < EnrollWindow)
    {
        if (Finger_Enroll())
        {
            Serial.println("指纹注册成功");
            sendCommandToDisplay("page registerYES");
            EnrollSuccess = true;
            break;
        }
        else
        {
            Serial.println("等待手指放入...");
        }
        vTaskDelay(pdMS_TO_TICKS(200)); // 200ms 检查间隔
    }

    // 恢复串口屏任务（无论成功或超时）
    vTaskResume(xSerialScreenTaskHandle);

    // 根据结果触发舵机解锁或显示超时
    if (EnrollSuccess)
    {
        xSemaphoreGive(xServoUnlockSemaphore);
    }
    else
    {
        sendCommandToDisplay("page registerNO");
        Serial.println("指纹注册任务超时。");
    }

    // 删除自身任务（确保所有资源已清理）
    vTaskDelete(NULL);
}
void MQTT_UnLOCK_Task(void *param)
{
    while (true)
    {
        MQTT_receiveMessage();
        if (Unlock == 1)
        {
            xSemaphoreGive(xServoUnlockSemaphore);
            sendCommandToDisplay("page attestationYES");
            Unlock = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
void ServoControlTask(void *pvParameters)
{
    while (true)
    {
        // 等待 RC522 或指纹认证任务释放信号量
        if (xSemaphoreTake(xServoUnlockSemaphore, portMAX_DELAY) == pdTRUE)
        {
            // 当信号量被释放后执行下面的操作，解锁舵机
            Serial.println("解锁舵机...");
            UNLOCK();
            Serial.println("舵机复位...");
        }
        vTaskDelay(300); // 延时200ms，防止占用过多CPU
    }
}

void Serial_Time_Task(void *param)
{
    while (true)
    {
        sendTimeToDisplay();
        vTaskDelay(pdMS_TO_TICKS(500)); // 每100ms检查一次串口
    }
}

void Serial_Screen_Task(void *param)
{
    while (true)
    {
        memset(RX_Data, 0, sizeof(RX_Data));
        if (ScreenSerial.available() > 0)
        {
            size_t bytesRead = ScreenSerial.readBytes(RX_Data, 1);
            // 打印接收到的原始数据（十六进制格式）
            Serial.print("[DEBUG] Received RAW Data: ");
            for (size_t i = 0; i < bytesRead; i++)
            {
                Serial.print(RX_Data[i], HEX); // 以HEX格式打印
                Serial.print(" ");
            }
            Serial.println();
            // 修正条件判断：仅当匹配成功时（返回 0）进入分支
            if (memcmp(RX_Data, "\x06", 1) == 0)
            { // 注意 == 0
                Serial.println("收到解锁Servo指令");
                memset(RX_Data, 0, sizeof(RX_Data)); // 全部置为 '\0'
                xSemaphoreGive(xServoUnlockSemaphore);
            }
            else if (memcmp(RX_Data, "\x05", 1) == 0)
            { // 注意 == 0
                Serial.println("收到注册RC522指令,启动注册任务...");
                memset(RX_Data, 0, sizeof(RX_Data)); // 全部置为 '\0'
                xTaskCreate(RC522_Register_Task, "RC522_Register", 4065, NULL, 1, &xRC522RegisteTaskHandle);
            }
            else if (memcmp(RX_Data, "\x03", 1) == 0)
            { // 注意 == 0
                Serial.println("收到认证Finger指令,启动认证任务...");
                memset(RX_Data, 0, sizeof(RX_Data)); // 全部置为 '\0'
                xTaskCreate(Finger_Authentication_Task, "Finger_Authentication", 4048, NULL, 1, NULL);
            }
            else if (memcmp(RX_Data, "\x07", 1) == 0)
            { // 注意 == 0
                Serial.println("收到注册Finger指令,启动注册任务...");
                memset(RX_Data, 0, sizeof(RX_Data)); // 全部置为 '\0'
                xTaskCreate(Finger_Enroll_Task, "Finger_Enroll", 4048, NULL, 1, NULL);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void ALL_CreateTasks(void)
{
    xServoUnlockSemaphore = xSemaphoreCreateBinary(); // 解锁信号量
    xSemaphoreTake(xServoUnlockSemaphore, 0);         // 默认情况下，信号量被锁定
    xTaskCreate(RC522_Authentication_Task, "RC522_Authentication", 2048, NULL, 2, &xRC522AuthenticationTaskHandle);
    xTaskCreate(ServoControlTask, "Unlock", 1024, NULL, 1, NULL);
    xTaskCreate(Serial_Screen_Task, "Serial_Screen", 2048, NULL, 1, &xSerialScreenTaskHandle);
    xTaskCreate(MQTT_UnLOCK_Task, "MQTT_Unlock", 8192, NULL, 1, NULL);
    xTaskCreate(Serial_Time_Task, " Serial_Time_Task", 4065, NULL, 1, NULL);
    vTaskSuspend(xRC522RegisteTaskHandle); // 初始化挂起任务
}