#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>
#include <MainBoard.h>
#include <Adafruit_ILI9341.h>
#include <Wire.h>
#include <Ethernet3.h>
#include <SdFat.h>
#include "cardParser.h"

#define LOGIC_GATES_W 4
#define LOGIC_GATES_H 4
#define LOGIC_GATES_PORTS_USED 4

const uint8_t portsUsedForGates[LOGIC_GATES_PORTS_USED] = {MAIN_BOARD_ANALOG_PORT_1, MAIN_BOARD_ANALOG_PORT_2, MAIN_BOARD_ANALOG_PORT_3, MAIN_BOARD_ANALOG_PORT_4};

#define LED_COUNT 20

const uint8_t pinDemux[4][3] = {{0x2,0x4,0x0}, {0x3,0x1,0x5}, {0x9,0x7,0xB}, {0xA, 0x8, 0x6}};

void setup() {
  MainBoardStart(false);
  Serial.println("Logic Re-Initialization Panel");

  setupParseLogicCards(LOGIC_GATES_W, LOGIC_GATES_H, portsUsedForGates ,LOGIC_GATES_PORTS_USED);

  Serial.print("Initialization Complete @");
  Serial.print(millis());
  Serial.println("ms");
}


void loop() {
  tickParseLogicCards();


  // old code for checking the sensors manually
  // for(int hall_sensor = 0; hall_sensor < 3/*LOGIC_GATES_H * LOGIC_GATES_W * LOGIC_GATES_SENSORS_PER_GATE*/; hall_sensor++){
  //   if(hall_sensor % 3 == 0){
  //     Serial.println();
  //     Serial.printf("Sensor %d: ", hall_sensor / 3 + 1);
  //   }
  //   Serial.print(mainBoardGetAnalogMux(pinDemux[0][hall_sensor]));
  //   if(hall_sensor % 3 != 2){
  //     Serial.print(',');
  //   }
  // }
  delay(500);
}

