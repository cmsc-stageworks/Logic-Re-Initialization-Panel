#ifndef LOGIC_WIRE_LIGHTS_H
#define LOGIC_WIRE_LIGHTS_H

#include "logic_evaluator.h"
#include <adafruit_neopixel.h>
#include <Arduino.h>

#define LOGIC_WIRE_NUM_LEDS 20

/*
Initializes logic lights
@param offset the offset for the grid lights to start
@param strip the strip of LEDs
@param updateStrip should update the strip on write
*/
void init_wire_lights(uint16_t offset, Adafruit_NeoPixel* strip, bool updateStrip=true, bool setupStrip=true);

/*
Updtates logic lights
@param logic_grid_wries updates the grid for the logic wires
*/
void update_wire_lights( logic_grid_wires* wires);

#endif // LOGIC_WIRE_LIGHTS_H

