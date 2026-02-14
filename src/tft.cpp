#include "tft.h"

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "boot_image_320x180.h"
#include <pgmspace.h>

// ------------------------------
// Pinos escolhidos
// ------------------------------
#define TFT_CS   42
#define TFT_DC   40
#define TFT_RST  41

#define TFT_SCK  38
#define TFT_MOSI 39
#define TFT_MISO 36

static Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
static bool layoutDrawn = false;

// ----------------------------------------------------
// Helpers de geometria (usa dimensões reais após rotation)
// ----------------------------------------------------
static inline int16_t W() { return (int16_t)tft.width(); }
static inline int16_t H() { return (int16_t)tft.height(); }

// ----------------------------------------------------
// Ajuste visual da “centralização” no módulo do Wokwi
// (recuo para a moldura não ficar colada na borda do framebuffer)
// ----------------------------------------------------
static const int16_t UI_MARGIN = 8;   // ajuste fino: 6, 8, 10...
static const int16_t UI_INNER  = 3;   // moldura interna

// ----------------------------------------------------
// Texto centralizado
// ----------------------------------------------------
static int16_t centerXForText(const char* text, uint8_t textSize) {
  int16_t x1, y1;
  uint16_t w, h;

  tft.setTextSize(textSize);
  tft.getTextBounds((char*)text, 0, 0, &x1, &y1, &w, &h);

  int16_t x = (W() - (int16_t)w) / 2;
  if (x < 0) x = 0;
  return x;
}

static void printCentered(int y, const char* text, uint8_t textSize, uint16_t color) {
  tft.setTextSize(textSize);
  tft.setTextColor(color);
  tft.setCursor(centerXForText(text, textSize), y);
  tft.print(text);
}

// ----------------------------------------------------
// Moldura + Header/Footer “recuados”
// ----------------------------------------------------
static void drawFrame() {
  const int16_t x = UI_MARGIN;
  const int16_t y = UI_MARGIN;
  const int16_t w = W() - (2 * UI_MARGIN);
  const int16_t h = H() - (2 * UI_MARGIN);

  // Moldura externa recuada
  tft.drawRect(x, y, w, h, ILI9341_DARKGREY);

  // Moldura interna
  tft.drawRect(x + UI_INNER, y + UI_INNER, w - 2 * UI_INNER, h - 2 * UI_INNER, ILI9341_DARKGREY);
}

static void drawHeader(const char* title) {
  drawFrame();

  // Linha do header dentro da área recuada
  const int16_t x1 = UI_MARGIN + 6;
  const int16_t x2 = W() - UI_MARGIN - 7;
  const int16_t y  = UI_MARGIN + 34;

  tft.drawLine(x1, y, x2, y, ILI9341_DARKGREY);

  if (title && title[0] != '\0') {
    printCentered(UI_MARGIN + 12, title, 2, ILI9341_WHITE);
  }
}

static void drawFooter(const char* footerText) {
  // Linha do footer dentro da área recuada
  const int16_t x1 = UI_MARGIN + 6;
  const int16_t x2 = W() - UI_MARGIN - 7;
  const int16_t y  = H() - UI_MARGIN - 35;

  tft.drawLine(x1, y, x2, y, ILI9341_DARKGREY);

  if (footerText && footerText[0] != '\0') {
    printCentered(H() - UI_MARGIN - 26, footerText, 2, ILI9341_WHITE);
  }
}

// ----------------------------------------------------
// LAYOUT FIXO DA TELA DE STATUS
// ----------------------------------------------------
static void drawStaticLayout() {
  tft.fillScreen(ILI9341_BLACK);

  drawHeader("Status do Sistema");
  drawFooter("FIAP + CPQD + SOFTEX");

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);

  // Rótulos fixos dentro da moldura
  tft.setCursor(UI_MARGIN + 12, 55);   tft.print("Luzes:");
  tft.setCursor(UI_MARGIN + 12, 140);  tft.print("Gas:");
  tft.setCursor(UI_MARGIN + 12, 175);  tft.print("MQTT:");

  layoutDrawn = true;
}

// ----------------------------------------------------
// API
// ----------------------------------------------------
void tftBegin() {
  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
  SPI.setFrequency(20000000); // 20 MHz

  tft.begin();
  tft.setRotation(1); // 320x240 landscape

  tft.fillScreen(ILI9341_BLACK);
  drawHeader("");
  drawFooter("");
  printCentered(120, "Display inicializado", 2, ILI9341_WHITE);
  delay(400);
}


void tftShowBootScreen1() {
  layoutDrawn = false;

  tft.fillScreen(ILI9341_BLACK);
  drawHeader("");
  drawFooter("");

  printCentered(70,  "Sistema de monitoramento", 2, ILI9341_RED);
  printCentered(100, "IOT",                      2, ILI9341_RED);
  printCentered(130, "feito por",                2, ILI9341_RED);
  printCentered(160, "Matheus Grossi",           2, ILI9341_RED);
} 

void tftShowBootScreen2() {
  layoutDrawn = false;

  tft.fillScreen(ILI9341_BLACK);

  // Imagem 320x180 ocupa toda a largura, centraliza no eixo Y
  const int16_t x0 = 0;
  const int16_t y0 = (tft.height() - BOOT2_IMG_H) / 2; // (240-180)/2 = 30

  static uint16_t lineBuf[BOOT2_IMG_W];

  tft.startWrite();
  tft.setAddrWindow(x0, y0, BOOT2_IMG_W, BOOT2_IMG_H);

  // Se as cores ficarem “estranhas”, troque para true
  const bool SWAP_BYTES = false;

  for (int16_t y = 0; y < BOOT2_IMG_H; y++) {
    for (int16_t x = 0; x < BOOT2_IMG_W; x++) {
      uint32_t idx = (uint32_t)y * (uint32_t)BOOT2_IMG_W + (uint32_t)x;
      uint16_t c = pgm_read_word(&boot2_img_320x180[idx]);

      if (SWAP_BYTES) c = (uint16_t)((c << 8) | (c >> 8));
      lineBuf[x] = c;
    }
    tft.writePixels(lineBuf, BOOT2_IMG_W, true);
  }

  tft.endWrite();
}
  
void tftShowReadyScreen() {
  layoutDrawn = false;

  tft.fillScreen(ILI9341_BLACK);
  drawHeader("Status do Sistema");
  drawFooter("FIAP + CPQD + SOFTEX");

  printCentered(110, "Inicializando...", 2, ILI9341_WHITE);
  delay(200);
}

void tftUpdateStatus(bool ldr[4], bool gas[4], bool gasAlarm, bool mqttConnected) {
  if (!layoutDrawn) {
    drawStaticLayout();
  }

  // ---------------------------
  // Atualização parcial (sem flicker)
  // ---------------------------

  // Área Luzes (limpa apenas área interna)
  tft.fillRect(UI_MARGIN + 12, 80, W() - (2 * UI_MARGIN) - 24, 50, ILI9341_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);

  tft.setCursor(UI_MARGIN + 12, 80);
  tft.print("L1:"); tft.print(ldr[0] ? "ON  " : "OFF ");
  tft.print("L2:"); tft.print(ldr[1] ? "ON  " : "OFF ");

  tft.setCursor(UI_MARGIN + 12, 105);
  tft.print("L3:"); tft.print(ldr[2] ? "ON  " : "OFF ");
  tft.print("L4:"); tft.print(ldr[3] ? "ON  " : "OFF ");

  // Área Gas
  tft.fillRect(UI_MARGIN + 72, 140, W() - (2 * UI_MARGIN) - 84, 20, ILI9341_BLACK);
  tft.setCursor(UI_MARGIN + 72, 140);
  tft.print(gasAlarm ? "SOS " : "OK  ");
  tft.print("[");
  tft.print(gas[0] ? "1" : "0");
  tft.print(gas[1] ? "1" : "0");
  tft.print(gas[2] ? "1" : "0");
  tft.print(gas[3] ? "1" : "0");
  tft.print("]");

  // Área MQTT
  tft.fillRect(UI_MARGIN + 72, 175, 80, 20, ILI9341_BLACK);
  tft.setCursor(UI_MARGIN + 72, 175);
  tft.print(mqttConnected ? "ON " : "OFF");
}
