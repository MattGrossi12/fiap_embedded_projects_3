#ifndef mqtt_con_h
#define mqtt_con_h

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// Variáveis de conexão:
extern PubSubClient mqtt;
extern WiFiClientSecure espClient;

// Variáveis de temporização
extern unsigned long lastMqtt;
extern unsigned long lastLcd;

// Variáveis a serem publicadas (definidas no seu .ino)
extern bool gasDetected[4];
extern bool ldrState[4];
extern bool anyGasDetected;

// Chamadas:
void connectMQTT();
void publishSensors();

#endif
