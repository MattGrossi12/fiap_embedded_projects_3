#include <Arduino.h>
#include <WiFi.h>

#include "wifi_con.h"
#include "thingspeak_con.h"
#include "tft.h"

//-------------------------------------------------
// Mapeamento de pinos
//-------------------------------------------------
const uint8_t GAS_PINS[4] = {11, 12, 13, 14};
#define SIN_DE_GAS  2

const uint8_t LDR_PINS[4]  = {9, 46, 3, 10};
const uint8_t RELE_PINS[4] = {16, 17, 18, 8};

// Mapeamento pedido (botão -> relé):
// GPIO7 -> relé 16 (canal 0)
// GPIO6 -> relé 17 (canal 1)
// GPIO5 -> relé 18 (canal 2)
// GPIO4 -> relé  8 (canal 3)
const uint8_t BTN_PINS[4] = {7, 6, 5, 4};

//-------------------------------------------------
// Variáveis globais (telemetria)
//-------------------------------------------------
bool gasDetected[4] = {false, false, false, false};
bool ldrState[4]    = {false, false, false, false};  // estado final do relé (saída)
bool anyGasDetected = false;

//-------------------------------------------------
// Botões (toggle) + debounce
//-------------------------------------------------
static bool buttonLatched[4] = {false, false, false, false};

static bool lastStableBtn[4] = {LOW, LOW, LOW, LOW};   // pull-down: solto=LOW
static bool lastRawBtn[4]    = {LOW, LOW, LOW, LOW};
static unsigned long lastDebounceTime[4] = {0, 0, 0, 0};

static const unsigned long DEBOUNCE_MS = 40;

//-------------------------------------------------
// Pisca-alerta do gás
//-------------------------------------------------
bool gasBlinkState = false;
unsigned long lastGasBlink = 0;
const unsigned long GAS_BLINK_INTERVAL = 1000;

//-------------------------------------------------
// Setup
//-------------------------------------------------
void setup() {
  Serial.begin(115200);

  // Entradas gás
  for (int i = 0; i < 4; i++) pinMode(GAS_PINS[i], INPUT);

  // Entradas LDR
  for (int i = 0; i < 4; i++) pinMode(LDR_PINS[i], INPUT);

  // Botões com pull-down EXTERNO (solto=LOW / pressionado=HIGH)
  for (int i = 0; i < 4; i++) {
    pinMode(BTN_PINS[i], INPUT);
    lastRawBtn[i] = digitalRead(BTN_PINS[i]);
    lastStableBtn[i] = lastRawBtn[i];
  }

  // Sirene/LED gás
  pinMode(SIN_DE_GAS, OUTPUT);
  digitalWrite(SIN_DE_GAS, LOW);

  // Saídas relés/LEDs
  for (int i = 0; i < 4; i++) {
    pinMode(RELE_PINS[i], OUTPUT);
    digitalWrite(RELE_PINS[i], LOW);
  }

  // TFT
  tftBegin();
  tftShowBootScreen1();

  // Wi-Fi
  connectWiFi();
  delay(500);

  // ThingSpeak MQTT
  tftShowBootScreen2();
  connectThingSpeak();
  delay(500);

  tftShowReadyScreen();
}

//-------------------------------------------------
// Loop principal
//-------------------------------------------------
void loop() {
  //-------------------------------------------------
  // Mantém conexão no ThingSpeak
  //-------------------------------------------------
  if (!thingspeak.connected()) {
    connectThingSpeak();
  }
  thingspeak.loop();

  //-------------------------------------------------
  // Leitura dos sensores de gás
  //-------------------------------------------------
  anyGasDetected = false;
  for (int i = 0; i < 4; i++) {
    gasDetected[i] = !digitalRead(GAS_PINS[i]); // mantém sua inversão (ativo em LOW)
    if (gasDetected[i]) anyGasDetected = true;
  }

  //-------------------------------------------------
  // Botões: toggle com debounce + OR com LDR
  //-------------------------------------------------
  unsigned long now = millis();

  for (int i = 0; i < 4; i++) {
    bool raw = digitalRead(BTN_PINS[i]);

    // Debounce: se mudou leitura bruta, reinicia timer
    if (raw != lastRawBtn[i]) {
      lastRawBtn[i] = raw;
      lastDebounceTime[i] = now;
    }

    // Se passou o tempo de debounce, considera como estado estável
    if ((now - lastDebounceTime[i]) >= DEBOUNCE_MS) {
      // Pull-down: clique na borda de subida (LOW -> HIGH)
      if (lastStableBtn[i] == LOW && raw == HIGH) {
        buttonLatched[i] = !buttonLatched[i]; // TOGGLE
      }
      lastStableBtn[i] = raw;
    }

    // LDR
    bool sensorLdr = digitalRead(LDR_PINS[i]);

    // OR: liga se sensor OU latch estiver ativo
    ldrState[i] = (sensorLdr || buttonLatched[i]);

    // aciona relé
    digitalWrite(RELE_PINS[i], ldrState[i]);
  }

  //-------------------------------------------------
  // Alerta de gás (pisca se qualquer sensor detectar)
  //-------------------------------------------------
  if (anyGasDetected) {
    if (now - lastGasBlink >= GAS_BLINK_INTERVAL) {
      lastGasBlink = now;
      gasBlinkState = !gasBlinkState;
      digitalWrite(SIN_DE_GAS, gasBlinkState);
    }
  } else {
    digitalWrite(SIN_DE_GAS, LOW);
    gasBlinkState = false;
  }

  //-------------------------------------------------
  // TFT (1 Hz) - mostra status do ThingSpeak
  //-------------------------------------------------
  static unsigned long lastLcdLocal = 0;
  if (now - lastLcdLocal >= 1000) {
    lastLcdLocal = now;
    tftUpdateStatus(ldrState, gasDetected, anyGasDetected, thingspeak.connected());
  }

  //-------------------------------------------------
  // Publish ThingSpeak - respeita rate limit
  //-------------------------------------------------
  static unsigned long lastTs = 0;
  if (now - lastTs >= THINGSPEAK_MIN_INTERVAL_MS) {
    lastTs = now;
    publishThingSpeak(gasDetected, ldrState, anyGasDetected);
  }
}
