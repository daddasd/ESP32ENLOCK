#ifndef MQTT_H
#define MQTT_H
#include <WiFi.h>
#include "PubSubClient.h"
#include "freertos.h"
#include "Arduino.h"
#include "OTA.h"

extern int Unlock;

void MQTT_init();
void MQTT_sendMessage(const char *message);
void callback(char *topic, byte *payload, unsigned int length);
void processData(const char *data);
void MQTT_receiveMessage();

#endif