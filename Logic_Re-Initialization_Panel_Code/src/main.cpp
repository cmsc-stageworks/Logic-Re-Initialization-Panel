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
#include "Ethernet3.h"
#include <Adafruit_ILI9341.h>
#include <Adafruit_ImageReader.h>
#include "logic_evaluator.h"

#define LOGIC_GATES_W 4
#define LOGIC_GATES_H 4
#define LOGIC_GATES_PORTS_USED 4

#define LCD_ROTATION 3
#define LCD_H 320
#define LCD_W 480

const uint8_t portsUsedForGates[LOGIC_GATES_PORTS_USED] = {MAIN_BOARD_ANALOG_PORT_1, MAIN_BOARD_ANALOG_PORT_2, MAIN_BOARD_ANALOG_PORT_3, MAIN_BOARD_ANALOG_PORT_4};

#define LED_COUNT 20

extern SPIClass mainBoardSpi;
extern SdFat mainBoardSD;
Adafruit_ImageReader reader(mainBoardSD);

Adafruit_ST7796S display1(&mainBoardSpi, MAIN_BOARD_LCD_1_CS, MAIN_BOARD_LCD_DC, MAIN_BOARD_LCD_RESET);
Adafruit_ST7796S display2(&mainBoardSpi, MAIN_BOARD_LCD_2_CS, MAIN_BOARD_LCD_DC, MAIN_BOARD_LCD_RESET);
// Adafruit_ILI9341 disp(&mainBoardSpi, MAIN_BOARD_LCD_2_CS, MAIN_BOARD_LCD_DC, MAIN_BOARD_LCD_RESET);

EthernetClass ethMain;

uint16_t getLogicColor(wire_state state){
  switch(state){
    case NOTHING:
      return ILI9341_DARKGREY;
    case FALSE:
      return ILI9341_RED;
    case TRUE:
      return ILI9341_OLIVE;
    case UNDEFINED:
      return ILI9341_CYAN;
    case ERROR:
      return ILI9341_YELLOW;
    case UNFILLED_INPUT:
      return ILI9341_PURPLE;
    default:
      return ILI9341_ORANGE;
  }
}

void setupScreenDebug(){
  display2.fillScreen(ILI9341_BLACK);
  for(int row = 0; row < 5; row++){
    display2.drawLine(LCD_W/2 - LCD_H/2, ((LCD_H-1) * row)/4 , LCD_W/2 + LCD_H/2,  ((LCD_H-1) * row)/4, ILI9341_LIGHTGREY);
  }
  for(int col = 0; col < 5; col++){
    display2.drawLine(LCD_W/2 - LCD_H/2 + LCD_H/4 * col, 0, LCD_W/2 - LCD_H/2  + LCD_H/4 * col,  LCD_H, ILI9341_LIGHTGREY);
  }
}

void showScreenDebug(LOGIC_CARD* readValues,uint16_t parsed_W,uint16_t parsed_H, logic_grid_wires* logic){
  for(int row = 0; row < 4; row++){
    for(int col = 0; col < 4; col++){
      LOGIC_CARD currCard = readValues[col * 4 + row];
      bool populated = (currCard & 0x08) != 0;
      if(populated){
        Serial.printf("Row: %d Col: %d populated with value: %d\n", row, col, currCard);
      }
      display2.drawChar(((LCD_H-1) * (col * 4 + 1))/16 + LCD_W/2 - LCD_H/2, ((LCD_H-1) * (row * 2 + 1))/8, 
        (populated ? ((currCard & 0x04) != 0) ? '1' : '0' : ' '), ILI9341_WHITE, ILI9341_BLACK, 1);
      display2.drawChar(((LCD_H-1) * (col * 4 + 2))/16 + LCD_W/2 - LCD_H/2, ((LCD_H-1) * (row * 2 + 1))/8, 
        (populated ? ((currCard & 0x02) != 0) ? '1' : '0' : ' '), ILI9341_WHITE, ILI9341_BLACK, 1);
      display2.drawChar(((LCD_H-1) * (col * 4 + 3))/16 + LCD_W/2 - LCD_H/2, ((LCD_H-1) * (row * 2 + 1))/8, 
        (populated ? ((currCard & 0x01) != 0) ? '1' : '0' : ' '), ILI9341_WHITE, ILI9341_BLACK, 1);
      int16_t rectX1 = ((LCD_H-1) * (col * 4))/16 + LCD_W/2 - LCD_H/2 + 4;
      int16_t rectY1 = ((LCD_H-1) * (row * 2 + 1))/8;
      display2.fillRect(rectX1, rectY1, 10, 10, getLogicColor(logic->wires[col][row]));
      rectX1 = ((LCD_H-1) * (col * 4 + 4))/16 + LCD_W/2 - LCD_H/2 - 1;
      display2.fillRect(rectX1, rectY1, -10, 10, getLogicColor(logic->wires[col + 1][row]));
    }
  }
}

void setup() {
  MainBoardStart(true);
  Serial.println("Logic Re-Initialization Panel");

  setupParseLogicCards(LOGIC_GATES_W, LOGIC_GATES_H, portsUsedForGates ,LOGIC_GATES_PORTS_USED);

  pinMode(MAIN_BOARD_LCD_1_CS, INPUT_PULLUP);
  pinMode(MAIN_BOARD_LCD_2_CS, INPUT_PULLUP);

  analogWrite(MAIN_BOARD_LCD_BRIGHTNESS, 0xFF);
  analogWrite(MAIN_BOARD_LCD_RESET, 0);
  delay(10);
  analogWrite(MAIN_BOARD_LCD_RESET, 0xFF);
  display1.init(LCD_H, LCD_W, 0, 0, ST7796S_BGR);
  display1.invertDisplay(true);
  display1.fillScreen(display1.color565(255,0,0));
  display1.setRotation(LCD_ROTATION);
  display2.init(LCD_H, LCD_W, 0, 0, ST7796S_BGR);
  display2.invertDisplay(true);
  display2.fillScreen(display2.color565(255,0,0));
  display2.setRotation(LCD_ROTATION);
  delay(100);
  display1.fillScreen(display1.color565(0,255,0));
  unsigned long startFill = millis();
  display2.fillScreen(display2.color565(0,255,0));
  Serial.printf("Screen fill time: %d\n", millis() - startFill);
  delay(100);
  display1.fillScreen(display1.color565(0,0,255));
  display2.fillScreen(display2.color565(0,0,255));
  delay(100);
  display1.fillScreen(0);
  display2.fillScreen(0);
  display1.setCursor(0,0);
  display2.setCursor(0,0);
  display1.setTextColor(0xFFFF);
  display2.setTextColor(0xFFFF);
  display1.print("Screen ONE initialized");
  display2.print("Screen TWO initialized");
  delay(500);
  // GFXcanvas16 canvas(LCD_W, LCD_H);
  Adafruit_Image image;
  startFill = millis();
  ImageReturnCode readLogoFromFile = reader.loadBMP("/Pictures/spaceship.bmp", image);
  unsigned long loadTime = millis() - startFill;
  if(readLogoFromFile == IMAGE_SUCCESS){
    image.draw(display2, 0, 0);
  }
  Serial.printf("Buffer image load time: %d\nBuffer image load + write time: %d\n", loadTime, millis() - startFill);
  startFill = millis();
  Serial.printf("Image Print Returned: %d\n", reader.drawBMP("/Pictures/spaceship.bmp", display2, 0, 0, true));
  Serial.printf("Direct image write time: %d\n", millis() - startFill);
  delay(1000);
  setupScreenDebug();
  // initISOCards(8);
  // addISOCard(0x01,)

  Serial.print("Initialization Complete @");
  Serial.print(millis());
  Serial.println("ms");
}


void loop() {
  tickParseLogicCards();

  static uint8_t logicTest = 0;

  static uint16_t parsed_W, parsed_H;
  static LOGIC_CARD* readValues = getParsedGates(&parsed_W, &parsed_H);

  static logic_grid_wires gridWires;
  static logic_grid grid;

  Serial.println("Current Gate status:");
  for(int h = 0; h < 4; h++){
    for(int w = 0; w < 4; w++){
      Serial.printf("%02x ", readValues[h + w * parsed_W]);
    }
    Serial.println();
  }

  gridWires.wires[0][0] = ((logicTest & 0b00010000) != 0) ? TRUE : FALSE;
  gridWires.wires[0][1] = ((logicTest & 0b00100000) != 0) ? TRUE : FALSE;
  gridWires.wires[0][2] = ((logicTest & 0b01000000) != 0) ? TRUE : FALSE;
  gridWires.wires[0][3] = ((logicTest & 0b10000000) != 0) ? TRUE : FALSE;

  readLogicGrid(&grid);
  populateLogicGridWireState(&grid,&gridWires);

  Serial.println("Grid State:");
  for(int h = 0; h < 4; h++){
    for(int w = 0; w < 4; w++){
      Serial.printf("%02x ", grid.blocks[w][h]);
      // Serial.print(grid.blocks[h][w],BIN);
      // Serial.print(' ');
    }
    Serial.println();
  }
  Serial.println();

  logicTest += 16;
  delay(500);
  showScreenDebug(readValues, parsed_W, parsed_H, &gridWires);
}

