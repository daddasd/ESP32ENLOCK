/*
 * @Author: 'lin' '11252700+display23@user.noreply.gitee.com'
 * @Date: 2025-03-13 14:59:11
 * @LastEditors: 'daddasd' '3323169544@qq.com'
 * @LastEditTime: 2025-04-14 17:21:18
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
    // Finger_Init();
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
    static uint16_t time = 50; // 10秒
    while (true)
    {
        vTaskSuspend(xSerialScreenTaskHandle); // 挂起串口屏任务
        vTaskSuspend(xRC522AuthenticationTaskHandle);//挂起RC522认证任务
        while (time--)
        {
            if (RC522_Save_ID()) // 注册成功
            {
                Serial.printf("RCC注册成功");
                sendCommandToDisplay("page registerYES");
                xSemaphoreGive(xServoUnlockSemaphore);
                vTaskResume(xSerialScreenTaskHandle);  // 恢复串口屏任务
                vTaskResume(xRC522AuthenticationTaskHandle);
                vTaskSuspend(xRC522RegisteTaskHandle); // 挂起自己
                break;
            }
            else
            {
                Serial.printf("请重试");
            }
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        time = 50;
        vTaskResume(xSerialScreenTaskHandle);  // 恢复串口屏任务
        vTaskResume(xRC522AuthenticationTaskHandle);
        vTaskSuspend(xRC522RegisteTaskHandle); // 挂起自己
    }
}
/**
 * @brief 指纹模块认证任务
 *
 * @param param
 */
void Finger_Authentication_Task(void *param)
{
    UBaseType_t originalPriority = uxTaskPriorityGet(xTaskGetCurrentTaskHandle());
    vTaskPrioritySet(NULL, 5);
    const uint32_t AuthenticationWindow = 8000; // 认证窗口，单位毫秒（这里为8秒）

    while (true)
    {
        if (xSemaphoreTake(xFinger_AuthenticationSemaphore, portMAX_DELAY) == pdTRUE)
        {
            uint32_t startTime = millis();
            bool AuthenticationSuccess = false;
            vTaskSuspend(xSerialScreenTaskHandle); // 挂起串口屏任务
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
                vTaskResume(xSerialScreenTaskHandle); // 恢复串口屏任务
            }
            else
            {
                sendCommandToDisplay("page attestationNO");
                Serial.println("指纹认证任务超时。");
                vTaskResume(xSerialScreenTaskHandle); // 恢复串口屏任务
            }
            vTaskPrioritySet(NULL, originalPriority);
        }
        vTaskDelay(200 / portTICK_PERIOD_MS); // 延时200ms，防止占用过多CPU
    }
}
/**
 * @brief 指纹注册任务
 *
 * @param param
 */
void Finger_Enroll_Task(void *param)
{
    UBaseType_t originalPriority = uxTaskPriorityGet(xTaskGetCurrentTaskHandle());
    vTaskPrioritySet(NULL, 5);
    const uint32_t EnrollWindow = 14000; // 认证窗口，单位毫秒（这里为14秒）
    while (true)
    {
        if (xSemaphoreTake(xFinger_EnrollSemaphore, portMAX_DELAY) == pdTRUE)
        {
            vTaskSuspend(xSerialScreenTaskHandle); // 挂起串口屏任务
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
                vTaskResume(xSerialScreenTaskHandle); // 恢复串口屏任务
            }
            else
            {
                sendCommandToDisplay("page registerNO");
                Serial.println("指纹注册任务超时。");
                vTaskResume(xSerialScreenTaskHandle); // 恢复串口屏任务
            }
            vTaskPrioritySet(NULL, originalPriority);
        }
        vTaskDelay(20 / portTICK_PERIOD_MS); // 延时200ms，防止占用过多CPU
    }
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
            UNLOCK();
            Serial.println("舵机复位...");
            vTaskResume(xSerialScreenTaskHandle); // 恢复串口屏任务
        }
        vTaskDelay(20 / portTICK_PERIOD_MS); // 延时200ms，防止占用过多CPU
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
                xSemaphoreGive(xServoUnlockSemaphore);
            }
            else if (memcmp(RX_Data, "\x05", 1) == 0)
            { // 注意 == 0
                Serial.println("收到注册RC522指令,启动注册任务...");
                vTaskResume(xRC522RegisteTaskHandle); // 恢复注册任务
            }
            else if (memcmp(RX_Data, "\x03", 1) == 0)
            { // 注意 == 0
              // Serial.println("收到认证Finger指令,启动认证任务...");
              // xSemaphoreGive(xFinger_AuthenticationSemaphore);
            }
            else if (memcmp(RX_Data, "\x07", 1) == 0)
            { // 注意 == 0
              // Serial.println("收到注册Finger指令,启动注册任务...");
              // xSemaphoreGive(xFinger_EnrollSemaphore);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
// FreeRTOS专用定时复位任务
void ResetTask(void *pvParameters)
{
    const TickType_t xDelay = pdMS_TO_TICKS(3600000); // 1小时

    for (;;)
    {
        vTaskDelay(xDelay);

        // 执行复位前的清理工作
        printf("System will reset now!\n");
        fflush(stdout);
        // 执行复位
        esp_restart(); // ESP32
                       // vTaskEndScheduler(); // 其他FreeRTOS系统
    }
}
void ALL_CreateTasks(void)
{
    xServoUnlockSemaphore = xSemaphoreCreateBinary();           // 解锁信号量
    xFinger_AuthenticationSemaphore = xSemaphoreCreateBinary(); // Finger认证信号量
    xFinger_EnrollSemaphore = xSemaphoreCreateBinary();         // Finger注册信号量
    xSemaphoreTake(xServoUnlockSemaphore, 0);                   // 默认情况下，信号量被锁定
    xSemaphoreTake(xFinger_AuthenticationSemaphore, 0);         // 默认情况下，信号量被锁定
    xSemaphoreTake(xFinger_EnrollSemaphore, 0);                 // 默认情况下，信号量被锁定
    xTaskCreate(RC522_Authentication_Task, "RC522_Authentication", 2048, NULL, 2,&xRC522AuthenticationTaskHandle);
    xTaskCreate(RC522_Register_Task, "RC522_Register", 16384, NULL, 1, &xRC522RegisteTaskHandle);
    xTaskCreate(ServoControlTask, "Unlock", 1024, NULL, 1, NULL);
    xTaskCreate(Serial_Screen_Task, "Serial_Screen", 2048, NULL, 1, &xSerialScreenTaskHandle);
    xTaskCreate(MQTT_UnLOCK_Task, "MQTT_Unlock", 8192, NULL, 1, NULL);
    xTaskCreate(Serial_Time_Task, " Serial_Time_Task", 4065, NULL, 1, NULL);
    xTaskCreate(Finger_Authentication_Task, "Finger_Authentication", 2048, NULL, 1, NULL);
    xTaskCreate(Finger_Enroll_Task, "Finger_Enroll", 2048, NULL, 1, NULL);
    xTaskCreate(ResetTask, "Reset", 2048, NULL, 1, NULL);
    vTaskSuspend(xRC522RegisteTaskHandle); // 初始化挂起任务
}