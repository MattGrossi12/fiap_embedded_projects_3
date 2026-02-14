#include "thingspeak_con.h"

// -----------------------------------------------------------------------------
// ThingSpeak MQTT (MathWorks)
//
// Autenticação (ATUAL / suportada):
//   1) ThingSpeak > Devices > MQTT > Add a new device
//   2) Autorize o seu canal
//   3) Baixe as credenciais (Plain Text)
//   4) Preencha os campos abaixo
//
// Publicação (MQTT API):
//   - Broker: mqtt3.thingspeak.com
//   - Topic:  channels/<CHANNEL_ID>/publish
//   - Payload: field1=...&field2=...&field3=...&field4=...
//
// Referência oficial (tópico + payload):
//   https://www.mathworks.com/help/thingspeak/publishtoachannelfeed.html
// -----------------------------------------------------------------------------

// Broker oficial (sem TLS) - mais simples para Wokwi.
// Para TLS, use porta 8883 + WiFiClientSecure + CA (não incluído aqui).
static const char* TS_BROKER = "mqtt3.thingspeak.com";
static const int   TS_PORT   = 1883;

// >>> PREENCHA COM AS CREDENCIAIS DO SEU "MQTT DEVICE" (ThingSpeak)
static const char* TS_MQTT_CLIENT_ID = "ENTER_MQTT_DEVICE_CLIENT_ID";
static const char* TS_MQTT_USERNAME  = "ENTER_MQTT_DEVICE_USERNAME";
static const char* TS_MQTT_PASSWORD  = "ENTER_MQTT_DEVICE_PASSWORD";

// >>> PREENCHA COM O ID DO SEU CANAL
static const long  TS_CHANNEL_ID     = 123456;

WiFiClient tsClient;
PubSubClient thingspeak(tsClient);

unsigned long lastThingSpeak = 0;

void connectThingSpeak() {
  thingspeak.setServer(TS_BROKER, TS_PORT);

  while (!thingspeak.connected()) {
    Serial.print("[ThingSpeak] Conectando...");

    const bool ok = thingspeak.connect(
      TS_MQTT_CLIENT_ID,
      TS_MQTT_USERNAME,
      TS_MQTT_PASSWORD
    );

    if (ok) {
      Serial.println(" conectado!");
    } else {
      Serial.print(" falhou, rc=");
      Serial.print(thingspeak.state());
      Serial.println(" tentando em 2s");
      delay(2000);
    }
  }
}

static uint8_t bitmask4(const bool v[4]) {
  uint8_t m = 0;
  for (int i = 0; i < 4; i++) {
    if (v[i]) m |= (1u << i);
  }
  return m;
}

void publishThingSpeak(bool gas[4], bool ldr[4], bool gasAlarm) {
  char topic[64];
  snprintf(topic, sizeof(topic), "channels/%ld/publish", TS_CHANNEL_ID);

  // Campos (2-4) - definição livre:
  // field1: gasAlarm (0/1)
  // field2: gasMask  (bit0..bit3)
  // field3: luzMask  (bit0..bit3)
  // field4: WiFi RSSI (dBm)
  const uint8_t gasMask = bitmask4(gas);
  const uint8_t luzMask = bitmask4(ldr);
  const int rssi = WiFi.RSSI();

  char payload[160];
  snprintf(payload, sizeof(payload),
    "field1=%d&field2=%u&field3=%u&field4=%d",
    gasAlarm ? 1 : 0,
    gasMask,
    luzMask,
    rssi
  );

  const bool ok = thingspeak.publish(topic, payload);

  Serial.print("[ThingSpeak] Topic: ");
  Serial.println(topic);
  Serial.print("[ThingSpeak] Payload: ");
  Serial.println(payload);
  Serial.print("[ThingSpeak] publish() retornou: ");
  Serial.println(ok ? "true" : "false");
}
