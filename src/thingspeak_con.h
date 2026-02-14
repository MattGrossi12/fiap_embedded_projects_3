#ifndef thingspeak_con_h
#define thingspeak_con_h

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

// Cliente MQTT do ThingSpeak (separado do broker do seu streaming)
extern WiFiClient tsClient;
extern PubSubClient thingspeak;

// Temporização (respeita o rate limit do ThingSpeak)
extern unsigned long lastThingSpeak;

// Intervalo mínimo recomendado (o ThingSpeak normalmente aceita 1 update a cada ~15s)
static const unsigned long THINGSPEAK_MIN_INTERVAL_MS = 16000;

// Conexão + publicação
void connectThingSpeak();
void publishThingSpeak(bool gas[4], bool ldr[4], bool gasAlarm);

#endif
