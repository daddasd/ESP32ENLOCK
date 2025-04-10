/*
 * @Author: 'lin' '11252700+display23@user.noreply.gitee.com'
 * @Date: 2025-03-13 14:59:11
 * @LastEditors: 'daddasd' '3323169544@qq.com'
 * @LastEditTime: 2025-04-10 19:50:54
 * @FilePath: \EN_LOOK\src\freertos.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "freertos.h"

SemaphoreHandle_t xServoUnlockSemaphore = NULL; // 创建一个二进制信号号量用于解锁

void All_Init(void)
{
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
        vTaskDelay(pdMS_TO_TICKS(50));
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
    UBaseType_t originalPriority = uxTaskPriorityGet(xTaskGetCurrentTaskHandle());
    vTaskPrioritySet(NULL, 3);
    const uint32_t registrationWindow = 10000; // 注册窗口，单位毫秒（这里为10秒）
    uint32_t startTime = millis();
    bool registrationSuccess = false;

    // 在注册窗口内不断尝试注册
    while (millis() - startTime < registrationWindow)
    {
        // 尝试注册卡号
        if (RC522_Save_ID()) // 注意：确保这里是函数调用，即 RC522_Save_ID()
        {
            Serial.println("注册成功！");
            sendCommandToDisplay("page registerYES");
            registrationSuccess = true;
            break; // 注册成功，退出循环
        }
        else
        {
            Serial.println("等待卡片进行注册...");
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // 每500ms尝试一次，避免过于频繁
    }
    // 根据注册结果释放信号量触发舵机解锁
    if (registrationSuccess)
    {
        xSemaphoreGive(xServoUnlockSemaphore);
    }
    else
    {
        sendCommandToDisplay("page registerNO");
        Serial.println("注册任务超时，未成功注册。");
    }
    vTaskPrioritySet(NULL, originalPriority);
    vTaskDelete(NULL); // 注册任务执行完毕后自我删除
}
/**
 * @brief 指纹模块认证任务
 *
 * @param param
 */
void Finger_Authentication_Task(void *param)
{
    UBaseType_t originalPriority = uxTaskPriorityGet(xTaskGetCurrentTaskHandle());
    vTaskPrioritySet(NULL, 3);
    const uint32_t AuthenticationWindow = 8000; // 认证窗口，单位毫秒（这里为10秒）
    uint32_t startTime = millis();
    bool AuthenticationSuccess = false;
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
    // 根据认证结果释放信号量触发舵机解锁
    if (AuthenticationSuccess)
    {
        xSemaphoreGive(xServoUnlockSemaphore);
    }
    else
    {
        sendCommandToDisplay("page attestationNO");
        Serial.println("指纹认证任务超时。");
    }
    vTaskPrioritySet(NULL, originalPriority);
    vTaskDelete(NULL); // 注册任务执行完毕后自我删除
}
/**
 * @brief 指纹注册任务
 *
 * @param param
 */
void Finger_Enroll_Task(void *param)
{
    UBaseType_t originalPriority = uxTaskPriorityGet(xTaskGetCurrentTaskHandle());
    vTaskPrioritySet(NULL, 3);
    const uint32_t EnrollWindow = 14000; // 认证窗口，单位毫秒（这里为14秒）
    uint32_t startTime = millis();
    bool EnrollSuccess = false;
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
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    // 根据认证结果释放信号量触发舵机解锁
    if (EnrollSuccess)
    {
        xSemaphoreGive(xServoUnlockSemaphore);
    }
    else
    {
        sendCommandToDisplay("page registerNO");
        Serial.println("指纹注册任务超时。");
    }
    vTaskPrioritySet(NULL, originalPriority);
    vTaskDelete(NULL); // 注册任务执行完毕后自我删除
}
void MQTT_UnLOCK_Task(void *param)
{
    while (true)
    {
        MQTT_receiveMessage();
        if (Unlock == 1)
        {
            xSemaphoreGive(xServoUnlockSemaphore);
            sendCommandToDisplay("page attestationNO");
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
            Serial.println("舵机复位...");
        }
        vTaskDelay(200 / portTICK_PERIOD_MS); // 延时200ms，防止占用过多CPU
    }
}

void Serial_Time_Task(void *param)
{
    while(true)
    {
        sendTimeToDisplay();
        vTaskDelay(pdMS_TO_TICKS(1000)); // 每100ms检查一次串口
    }
}

void Serial_Screen_Task(void *param)
{
    while (true)
    {
        if (ScreenSerial.available() > 0)
        {
            int command = Serial.parseInt();
            switch (command)
            {
            case Screen_Mode::RC522_Register:
                /* code */
                Serial.println("收到注册RC522指令，启动注册任务...");
                xTaskCreate(RC522_Register_Task, "RC522_Register", 2048, NULL, 1, NULL);
                break;
            case Screen_Mode::Finger_Register:
                Serial.println("收到注册Finger指令，启动注册任务...");
                xTaskCreate(Finger_Enroll_Task, "Finger_Enroll", 2048, NULL, 1, NULL);
                break;
            case Screen_Mode::Finger_authentication:
                Serial.println("收到注册Finger认证，启动认证任务...");
                xTaskCreate(Finger_Authentication_Task, "Finger_Authentication", 2048, NULL, 1, NULL);
                break;
            case 33: // 密码解锁成功
                xSemaphoreGive(xServoUnlockSemaphore);
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200)); // 每100ms检查一次串口
    }
}


void ALL_CreateTasks(void)
{
    xServoUnlockSemaphore = xSemaphoreCreateBinary();
    xSemaphoreTake(xServoUnlockSemaphore, 0); // 默认情况下，信号量被锁定
    xTaskCreate(RC522_Authentication_Task, "RC522_Authentication", 2048, NULL, 2, NULL);
    xTaskCreate(ServoControlTask, "Unlock", 1024, NULL, 1, NULL);
    xTaskCreate(Serial_Screen_Task, "Serial_Screen", 2048, NULL, 1, NULL);
    xTaskCreate(MQTT_UnLOCK_Task, "MQTT_Unlock", 8192, NULL, 1, NULL);
    xTaskCreate(Serial_Time_Task, " Serial_Time_Task", 4065, NULL, 1, NULL);
    //   xTaskCreate(Finger_Authentication_Task, "Finger_Authentication", 2048, NULL, 1, NULL);
    //   xTaskCreate(Finger_Enroll_Task, "Finger_Enroll", 2048, NULL, 1, NULL);
}