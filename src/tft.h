#ifndef TFT_H
#define TFT_H

#include <Arduino.h>

// Inicializa SPI + ILI9341
void tftBegin();

// Telas de abertura (equivalentes às do LCD antigo)
void tftShowBootScreen1();
void tftShowBootScreen2();
void tftShowReadyScreen();

// Tela principal de status (1 Hz)
void tftUpdateStatus(bool ldr[4], bool gas[4], bool gasAlarm, bool mqttConnected);

#endif

