#ifndef LOGIC_WIRE_LIGHTS_H
#define LOGIC_WIRE_LIGHTS_H

#include <Arduino.h>
#include <Adafruit_Neopixel.h>
#include "logic_evaluator.h"

#define LOGIC_WIRE_NUM_LEDS 20
#define LOGIC_WIRE_BRIGHTNESS 127

#define LOGIC_WIRE_NOTHING_COLOR 0,0,0
#define LOGIC_WIRE_FALSE_COLOR 255,0,0
#define LOGIC_WIRE_TRUE_COLOR 0,255,0
#define LOGIC_WIRE_UNDEFINED_COLOR 0,255,255
#define LOGIC_WIRE_ERROR_COLOR 255,255,0
#define LOGIC_WIRE_UNFILLED_INPUT_COLOR 255,0,255
#define LOGIC_WIRE_DEFAULT_COLOR 255,127,0

/*
Initializes logic lights
@param offset the offset for the grid lights to start
@param strip the strip of LEDs
@param updateStrip should update the strip on write
*/
void init_wire_lights(uint16_t offset, Adafruit_NeoPixel* strip, bool updateStrip, bool setupStrip);

/*
Updtates logic lights
@param logic_grid_wries updates the grid for the logic wires
*/
void update_wire_lights(logic_grid_wires* wires);

#endif // LOGIC_WIRE_LIGHTS_H

