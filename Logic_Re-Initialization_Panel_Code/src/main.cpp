#include "globalFlags.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_timer.h"
#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>
#include <MainBoard.h>
#include <isoCards.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7796S.h>
#include <esp_random.h>
#include <Wire.h>
#include <Ethernet3.h>
#include <SdFat.h>
#include "cardParser.h"
#include "Ethernet3.h"
#include <Adafruit_ILI9341.h>
#include <Adafruit_ImageReader.h>
#include "logic_evaluator.h"
#include <Adafruit_Neopixel.h>
#include "logic_wire_lights.h"
#include "puzzle_manager.h"
#include "InputManager.h"

#define TEST_WRITE_TO_CARD false
#define TEST_WRITE_TO_CARD_NAME "MISSING-NO"
/*
Curr Test Cards:
  - TEMP_CONTROL
  - AIR_SCRUBBERS

*/

#define LED_COUNT 20

extern SPIClass mainBoardSpi;
extern SdFat mainBoardSD;
Adafruit_ImageReader reader(mainBoardSD);

Adafruit_ST7796S display1(&mainBoardSpi, MAIN_BOARD_LCD_1_CS, MAIN_BOARD_LCD_DC, MAIN_BOARD_LCD_RESET);
Adafruit_ST7796S display2(&mainBoardSpi, MAIN_BOARD_LCD_2_CS, MAIN_BOARD_LCD_DC, MAIN_BOARD_LCD_RESET);
// Adafruit_ILI9341 disp(&mainBoardSpi, MAIN_BOARD_LCD_2_CS, MAIN_BOARD_LCD_DC, MAIN_BOARD_LCD_RESET);

EthernetClass ethMain;

Adafruit_NeoPixel mainStrip = Adafruit_NeoPixel(LOGIC_WIRE_NUM_LEDS, MAIN_BOARD_WS2812_PIN, NEO_RGB+NEO_KHZ800); 

static TimerHandle_t logicGridTimer;
static TimerHandle_t inputTimer;
static TimerHandle_t ISOCardTimer;
volatile bool shouldUpdateGrid = false;
volatile bool shouldUpdateButtons = false;
volatile bool shouldCheckInputs = false;
volatile bool shouldCheckISOCards = false;


void setLogicGateUpdateFlag(TimerHandle_t handle);
void setInputUpdateFlag(TimerHandle_t handle);
void setISOUpdateFlag(TimerHandle_t handle);


void setupGeneralLightStrips();


void setup() {
  MainBoardStart(true);
  Serial.println("Logic Re-Initialization Panel");

  initInputs();

  setupGeneralLightStrips();

  initISOCards(NUM_ISO_CARDS_LOGIC_PANEL, &mainStrip);

  initPuzzleManager(&mainStrip, 0, &display2);

  pinMode(MAIN_BOARD_LCD_1_CS, INPUT_PULLUP);
  pinMode(MAIN_BOARD_LCD_2_CS, INPUT_PULLUP);

  analogWrite(MAIN_BOARD_LCD_BRIGHTNESS, 0xFF);
  analogWrite(MAIN_BOARD_LCD_RESET, 0);
  delay(10);
  analogWrite(MAIN_BOARD_LCD_RESET, 0xFF);
  display2.init(LCD_H, LCD_W, 0, 0, ST7796S_BGR);
  display2.invertDisplay(true);
  display2.setRotation(LCD_ROTATION);
  display2.fillScreen(0);
  Adafruit_Image image;
  ImageReturnCode readLogoFromFile = reader.loadBMP("/Pictures/spaceship.bmp", image);
  if(readLogoFromFile == IMAGE_SUCCESS){
    image.draw(display2, 0, 0);
  } else{
    display2.setTextSize(3);
    display2.print("Critical Error! Logic Re-Initializer Failed To start!");
  }

  logicGridTimer = xTimerCreate("Shot Reload", pdMS_TO_TICKS(500), pdTRUE, NULL, setLogicGateUpdateFlag);

  if(logicGridTimer == NULL){
    Serial.println("Failed to start Update Grid ISR!");
  } else{
    xTimerStart(logicGridTimer, 0);
  }

  inputTimer = xTimerCreate("Shot Reload", pdMS_TO_TICKS(10), pdTRUE, NULL, setInputUpdateFlag);

  if(inputTimer == NULL){
    Serial.println("Failed to start Update Grid ISR!");
  } else{
    xTimerStart(inputTimer, 0);
  }

  ISOCardTimer = xTimerCreate("Shot Reload", pdMS_TO_TICKS(150), pdTRUE, NULL, setISOUpdateFlag);

  if(ISOCardTimer == NULL){
    Serial.println("Failed to start Update Grid ISR!");
  } else{
    xTimerStart(ISOCardTimer, 0);
  }

  #if TEST_WRITE_TO_CARD
  ISO_CARD freshCardToWrite;
  freshCardToWrite.slotID = LOGIC_GATE_SLOT_NUMBER;
  char* stringName = TEST_WRITE_TO_CARD_NAME;
  memcpy(freshCardToWrite.payload, stringName, strlen(stringName));
  freshCardToWrite.payloadLen = strlen(stringName);
  Serial.print("Payload Len: ");
  Serial.println(freshCardToWrite.payloadLen);

  while(!attemptToWriteToCard(freshCardToWrite)){
    Serial.println("Write to card failed, trying again");
    delay(500);
  }
  #endif

  // setPuzzleDemoMode(true);

  Serial.print("Initialization Complete @");
  Serial.print(millis());
  Serial.println("ms");
}


void loop() {
  if(shouldUpdateGrid){
    shouldUpdateGrid = false;
    tickPuzzleManager();
  }
  if(shouldCheckInputs){
    shouldCheckInputs = false;
    tickInputs();
  }
  if(shouldCheckISOCards){
    shouldCheckISOCards = false;
    tickParseLogicCards();
  }
}

void setLogicGateUpdateFlag(TimerHandle_t handle){
  shouldUpdateGrid = true;
}


void setInputUpdateFlag(TimerHandle_t handle){
  shouldCheckInputs = true;
}


void setISOUpdateFlag(TimerHandle_t handle){
  shouldCheckISOCards = true;
}


void setupGeneralLightStrips(){
  mainStrip.updateLength(LOGIC_WIRE_NUM_LEDS + ISO_CARDS_NUM_LEDS_PER_CARD);
  mainStrip.begin();
  mainStrip.setBrightness(LOGIC_WIRE_BRIGHTNESS);
  mainStrip.fill(mainStrip.Color(255,0,0));
  mainStrip.show();
  delay(250);
  mainStrip.fill(mainStrip.Color(0,255,0));
  mainStrip.show();
  delay(250);
  mainStrip.fill(mainStrip.Color(0,0,255));
  mainStrip.show();
  delay(250);
  mainStrip.clear();
  mainStrip.show();
}