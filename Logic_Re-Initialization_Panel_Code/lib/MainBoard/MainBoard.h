#ifndef MAIN_BOARD_H
#define MAIN_BOARD_H

#include "MainBoardPinout.h"
#include <TCA9555.h>

#define MAIN_BOARD_DIGITAL_PORT_1 0
#define MAIN_BOARD_DIGITAL_PORT_2 1
#define MAIN_BOARD_DIGITAL_PORT_3 2
#define MAIN_BOARD_DIGITAL_PORT_4 3
#define MAIN_BOARD_DIGITAL_PINS_PER_PORT 16

#define MAIN_BOARD_ANALOG_PORT_1 4
#define MAIN_BOARD_ANALOG_PORT_2 5
#define MAIN_BOARD_ANALOG_PORT_3 6
#define MAIN_BOARD_ANALOG_PORT_4 7
#define MAIN_BOARD_ANALOG_PORT_5 8
#define MAIN_BOARD_ANALOG_PORT_6 9
#define MAIN_BOARD_ANALOG_PORT_7 10
#define MAIN_BOARD_ANALOG_PORT_8 11
#define MAIN_BOARD_ANALOG_PINS_PER_PORT 12

int MainBoardStart(bool initSD = true);

static bool mainBoardInitI2CMux();
bool mainBoardSetI2CBus(uint8_t bus);
static bool mainBoardInitI2CMux();

bool mainBoardDigitalPinMode(uint8_t pin, uint8_t mode);
bool mainBoard16DigitalPinMode(uint8_t chip, uint16_t mode);
uint16_t mainBoardGet16DigitalInput(uint8_t chip);
bool mainBoardWriteDigitalOutput(uint8_t pin, uint8_t val);
bool mainBoardWrite16DigitalOutput(uint8_t chip, uint16_t val);
uint32_t mainBoardGetAnalogMux(uint8_t address);

#endif