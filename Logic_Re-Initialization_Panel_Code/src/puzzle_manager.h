#ifndef PUZZLE_MANAGER_H
#define PUZZLE_MANAGER_H

#include <Arduino.h>
#include <SdFat.h>
#include <isoCards.h>
#include <MainBoard.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7796S.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_ImageReader.h>
#include <Adafruit_NeoPixel.h>
#include "globalFlags.h"
#include <string.h>
#include "logic_evaluator.h"
#include "logic_wire_lights.h"
#include "cardParser.h"

#define DEBUG_PUZZLE_MANAGER true
#define DEBUG_SHOW_GATE_STATUS false
#define DEBUG_COUNT_LOGIC_INPUTS true

#define PUZZLE_PATH "/Puzzles/"
#define PUZZLE_SOLUTION_FILENAME "sol.txt"
#define PUZZLE_PROMPT_FILENAME "prompt.bmp"
#define PUZZLE_SUCCESS_FILENAME "success.bmp"

#define NUM_ISO_CARDS_LOGIC_PANEL 1
#define LOGIC_GATE_SLOT_NUMBER 0x8888
#define LOGIC_GATE_MUX_NUMBER 1

#define LOGIC_GATES_W 4
#define LOGIC_GATES_H 4
#define LOGIC_GATES_PORTS_USED 4

#define LCD_ROTATION 3
#define LCD_H 320
#define LCD_W 480


void initPuzzleManager(Adafruit_NeoPixel* strip, uint16_t offset, Adafruit_ST7796S* screen);
void setPuzzleDemoMode(bool isSet);
void tickPuzzleManager();

#endif