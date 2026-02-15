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
//   - Payload: field1=...&field2=...&...&field8=...
//
// Referência oficial (tópico + payload):
//   https://www.mathworks.com/help/thingspeak/publishtoachannelfeed.html
// -----------------------------------------------------------------------------

// Broker oficial (sem TLS) - mais simples para Wokwi.
// Para TLS, use porta 8883 + WiFiClientSecure + CA (não incluído aqui).
static const char* TS_BROKER = "mqtt3.thingspeak.com";
static const int   TS_PORT   = 1883;

// >>> PREENCHA COM AS CREDENCIAIS DO SEU "MQTT DEVICE" (ThingSpeak)
static const char* TS_MQTT_CLIENT_ID = "LjorDzQRMQk9HSYgBzMnDQw";
static const char* TS_MQTT_USERNAME  = "LjorDzQRMQk9HSYgBzMnDQw";
static const char* TS_MQTT_PASSWORD  = "lZrnaeBTtIITotTutPvLq6/i";

// >>> PREENCHA COM O ID DO SEU CANAL
static const long  TS_CHANNEL_ID     = 3264524;

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

void publishThingSpeak(bool gas[4], bool ldr[4], bool gasAlarm) {
  (void)gasAlarm; // não usado (mantido apenas para compatibilidade da assinatura)

  char topic[64];
  snprintf(topic, sizeof(topic), "channels/%ld/publish", TS_CHANNEL_ID);

  // Mapeamento (8 dados):
  // field1..field4 = Relés 1..4 (ldrState)
  // field5..field8 = Gás  1..4 (gasDetected)
  char payload[220];
  snprintf(payload, sizeof(payload),
    "field1=%d&field2=%d&field3=%d&field4=%d&"
    "field5=%d&field6=%d&field7=%d&field8=%d",
    ldr[0] ? 1 : 0,
    ldr[1] ? 1 : 0,
    ldr[2] ? 1 : 0,
    ldr[3] ? 1 : 0,
    gas[0] ? 1 : 0,
    gas[1] ? 1 : 0,
    gas[2] ? 1 : 0,
    gas[3] ? 1 : 0
  );

  const bool ok = thingspeak.publish(topic, payload);

  Serial.print("[ThingSpeak] Topic: ");
  Serial.println(topic);
  Serial.print("[ThingSpeak] Payload: ");
  Serial.println(payload);
  Serial.print("[ThingSpeak] publish() retornou: ");
  Serial.println(ok ? "true" : "false");
}
