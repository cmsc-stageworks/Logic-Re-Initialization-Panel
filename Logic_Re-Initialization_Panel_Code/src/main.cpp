#define DEBUG_NFC_ISO_CARDS true

#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>
#include <MainBoard.h>
#include <isoCards.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7796S.h>
#include <Wire.h>
#include <Ethernet3.h>
#include <SdFat.h>
#include "cardParser.h"
#include <Adafruit_ILI9341.h>

#define LOGIC_GATES_W 4
#define LOGIC_GATES_H 4
#define LOGIC_GATES_PORTS_USED 4

#define LCD_H 320
#define LCD_W 480

const uint8_t portsUsedForGates[LOGIC_GATES_PORTS_USED] = {MAIN_BOARD_ANALOG_PORT_1, MAIN_BOARD_ANALOG_PORT_2, MAIN_BOARD_ANALOG_PORT_3, MAIN_BOARD_ANALOG_PORT_4};

#define LED_COUNT 20

extern SPIClass mainBoardSpi;

Adafruit_ST7796S display1(&mainBoardSpi, MAIN_BOARD_LCD_1_CS, MAIN_BOARD_LCD_DC, MAIN_BOARD_LCD_RESET);
Adafruit_ST7796S display2(&mainBoardSpi, MAIN_BOARD_LCD_2_CS, MAIN_BOARD_LCD_DC, MAIN_BOARD_LCD_RESET);
// Adafruit_ILI9341 disp(&mainBoardSpi, MAIN_BOARD_LCD_2_CS, MAIN_BOARD_LCD_DC, MAIN_BOARD_LCD_RESET);

void showScreenDebug(LOGIC_CARD* readValues,uint16_t parsed_W,uint16_t parsed_H){
  
}

void setup() {
  MainBoardStart(false);
  Serial.println("Logic Re-Initialization Panel");

  setupParseLogicCards(LOGIC_GATES_W, LOGIC_GATES_H, portsUsedForGates ,LOGIC_GATES_PORTS_USED);

  pinMode(MAIN_BOARD_LCD_1_CS, INPUT_PULLUP);
  pinMode(MAIN_BOARD_LCD_2_CS, INPUT_PULLUP);

  analogWrite(MAIN_BOARD_LCD_BRIGHTNESS, 1024);
  analogWrite(MAIN_BOARD_LCD_RESET, 0);
  delay(10);
  analogWrite(MAIN_BOARD_LCD_RESET, 1024);
  display1.init(320, 480, 0, 0, ST7796S_BGR);
  display1.invertDisplay(true);
  display1.fillScreen(display1.color565(255,0,0));
  display2.init(320, 480, 0, 0, ST7796S_BGR);
  display2.invertDisplay(true);
  display2.fillScreen(display2.color565(255,0,0));
  delay(250);
  display1.fillScreen(display1.color565(0,255,0));
  unsigned long startFill = millis();
  display2.fillScreen(display2.color565(0,255,0));
  Serial.printf("Screen fill time: %d\n", millis() - startFill);
  delay(250);
  display1.fillScreen(display1.color565(0,0,255));
  display2.fillScreen(display2.color565(0,0,255));
  delay(250);
  display1.fillScreen(0);
  display2.fillScreen(0);
  display1.setCursor(0,0);
  display2.setCursor(0,0);
  display1.setTextColor(0xFFFF);
  display2.setTextColor(0xFFFF);
  display1.print("Screen ONE initialized");
  display2.print("Screen TWO initialized");

  // initISOCards(8);
  // addISOCard(0x01,)

  Serial.print("Initialization Complete @");
  Serial.print(millis());
  Serial.println("ms");
}


void loop() {
  tickParseLogicCards();

  static uint16_t parsed_W, parsed_H;
  static LOGIC_CARD* readValues = getParsedGates(&parsed_W, &parsed_H);

  Serial.println("Current Gate status:");
  for(int h = 0; h < 4; h++){
    for(int w = 0; w < 4; w++){
      Serial.printf("%02x ", readValues[h + w * parsed_W]);
    }
    Serial.println();
  }
  
  delay(500);
}

