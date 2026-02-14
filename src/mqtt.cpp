#include "mqtt_con.h"

// Configurações do Broker
const char* MQTT_BROKER    = "hd747018.ala.us-east-1.emqxsl.com";
const int   MQTT_PORT      = 8883;
const char* MQTT_CLIENTID  = "ESP32-WOKWI-SENSORES-FIAP";
const char* MQTT_USER      = "MATHEUS";
const char* MQTT_PASS      = "123";
const char* MQTT_PUB_TOPIC = "wokwi/sensores/fiap";

// Objetos globais
WiFiClientSecure espClient;
PubSubClient mqtt(espClient);

unsigned long lastMqtt = 0;
unsigned long lastLcd  = 0;

//-------------------------------------------------
// MQTT
//-------------------------------------------------
void connectMQTT() {
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);

  while (!mqtt.connected()) {
    Serial.print("[MQTT] Conectando...");
    if (mqtt.connect(MQTT_CLIENTID, MQTT_USER, MQTT_PASS)) {
      Serial.println(" conectado!");
    } else {
      Serial.print(" falhou, rc=");
      Serial.print(mqtt.state());
      Serial.println(" tentando em 2s");
      delay(2000);
    }
  }
}

//-------------------------------------------------
// Publicação MQTT (JSON)
//-------------------------------------------------
void publishSensors() {
  char payload[384];

  snprintf(payload, sizeof(payload),
    "{"
      "\"gas_alarm\": %d,"
      "\"gas1\": %d, \"gas2\": %d, \"gas3\": %d, \"gas4\": %d,"
      "\"luz1\": %d, \"luz2\": %d, \"luz3\": %d, \"luz4\": %d"
    "}",
    anyGasDetected ? 1 : 0,
    gasDetected[0] ? 1 : 0,
    gasDetected[1] ? 1 : 0,
    gasDetected[2] ? 1 : 0,
    gasDetected[3] ? 1 : 0,
    ldrState[0] ? 1 : 0,
    ldrState[1] ? 1 : 0,
    ldrState[2] ? 1 : 0,
    ldrState[3] ? 1 : 0
  );

  mqtt.publish(MQTT_PUB_TOPIC, payload);

  Serial.print("[MQTT] Publicado: ");
  Serial.println(payload);
}
