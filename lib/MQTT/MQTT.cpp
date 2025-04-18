#include "MQTT.h"

// WIFI配置
#define WIFISSID "EN_ROOM"         // WIFI名称
#define WIFIPSW "qianrushi1409"   // WIFI密码

// MQTT服务器设置
#define MQTT_SERVER "bemfa.com"                    // MQTT服务器
#define MQTT_SERVER_PORT 9501                      // MQTT服务器端口
#define MQTT_ID "cb042eda51f84e809babad0b0038765f" // 巴法云密钥
#define MQTT_TOPIC "ENLOOK002"                     // 主题

WiFiClient wifiClient;
PubSubClient client(wifiClient);

long lastMsg = 0;

int Unlock = 0;
// 定义你自定义的消息

/*
 * MQTT初始化函数：连接WIFI、设置MQTT服务器和回调函数，并完成MQTT连接和订阅
 */
/*
 * MQTT初始化函数（增加20秒超时）
 */
void MQTT_init()
{
    Serial.begin(115200);

    // 连接WIFI
    WiFi.begin(WIFISSID, WIFIPSW);
    unsigned long wifiStartTime = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - wifiStartTime > 20000)
        { // WiFi连接也设置20秒超时
            Serial.println("WiFi连接超时！");
            return; // 直接退出函数
        }
        Serial.println("正在连接WiFi...");
        delay(500);
    }
    Serial.print("WiFi连接成功。IP地址: ");
    Serial.println(WiFi.localIP());

    // 配置MQTT服务器及回调
    client.setServer(MQTT_SERVER, MQTT_SERVER_PORT);
    client.setCallback(callback);

    // 尝试连接MQTT（含20秒超时）
    unsigned long mqttStartTime = millis();
    bool mqttConnected = false;

    while (!client.connected())
    {
        // 检查是否超时
        if (millis() - mqttStartTime > 20000)
        {
            Serial.println("MQTT连接超时（20秒）！");
            break;
        }

        Serial.println("正在连接MQTT...");
        if (client.connect(MQTT_ID))
        {
            Serial.println("MQTT连接成功。");
            client.subscribe(MQTT_TOPIC);
            mqttConnected = true;
            break;
        }
        else
        {
            Serial.print("连接失败，错误码: ");
            Serial.print(client.state());

            // 动态计算剩余等待时间（不超过5秒）
            unsigned long elapsed = millis() - mqttStartTime;
            unsigned long remaining = 20000 - elapsed;
            if (remaining <= 0)
                break; // 已超时

            unsigned long retryDelay = (remaining < 5000) ? remaining : 5000;
            Serial.printf("，%lu秒后重试...\n", retryDelay / 1000);
            delay(retryDelay); // 按需等待
        }
    }

    // 最终状态检查
    if (!mqttConnected)
    {
        Serial.println("MQTT初始化失败，请检查网络或配置！");
    }
}

/*
 * 发送消息函数：每隔2秒将自定义的消息发布到指定主题
 */
void MQTT_sendMessage(const char *message)
{
    client.publish(MQTT_TOPIC, message);
    Serial.print("Published message: ");
    Serial.println(message);
}
void MQTT_sendMessage_Int(int number)
{
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d", number);
    client.publish(MQTT_TOPIC, buffer);
    Serial.printf("Published number: %d\n", number);
}
// 接收到消息时的回调函数
void callback(char *topic, byte *payload, unsigned int length)
{
    // 将接收到的payload转换为以'\0'结尾的字符串
    char receivedData[length + 1];
    for (unsigned int i = 0; i < length; i++)
    {
        receivedData[i] = (char)payload[i];
    }
    receivedData[length] = '\0'; // 添加字符串结束标志

    //Serial.println(receivedData);

    // 调用处理函数对接收到的数据进行处理
    processData(receivedData);
}

/*
 * 处理数据的函数，根据接收到的数据执行不同的操作
 * 例如：如果接收到 "ON"，则执行打开设备的操作；如果接收到 "OFF"，则关闭设备
 */
void processData(const char *data)
{
    Serial.print("Processing data: ");
    Serial.println(data);

    // 示例处理：根据指令执行相应操作
    if (strcmp(data, "on") == 0)
    {
        Serial.println("Action: Turn device ON");
        Unlock = 1;
        // 在这里添加打开设备的代码，例如 digitalWrite(LED_PIN, HIGH);
    }
    else if (strcmp(data, "update") == 0)
    {
        Serial.println("EPS32_OTAING");
        updateBin();
        // 在这里添加关闭设备的代码，例如 digitalWrite(LED_PIN, LOW);
    }
    else
    {
        Serial.println("Action: Unknown command, no action taken.");
        // 可以添加其他处理逻辑
    }
}

/*
 * 接收消息函数：检测MQTT连接状态并调用 client.loop() 处理订阅消息
 */
void MQTT_receiveMessage()
{
    // 如果MQTT断开连接则尝试重连
    if (!client.connected())
    {
        while (!client.connected())
        {
            Serial.println("Reconnecting to MQTT...");
            if (client.connect(MQTT_ID))
            {
                Serial.println("Reconnected to MQTT.");
                client.subscribe(MQTT_TOPIC);
            }
            else
            {
                Serial.print("Failed, rc=");
                Serial.print(client.state());
                Serial.println(". Retrying in 5 seconds...");
            }
        }
    }
    client.loop();
}
