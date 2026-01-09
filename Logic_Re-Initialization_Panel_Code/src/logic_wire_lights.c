#include "logic_wire_lights.h"
#include "logic_evaluator.h"
#include <adafruit_neopixel.h>
#include <Arduino.h>


static Adafruit_Neopixel* use_strip;
static uint16_t stripOffset;
static bool shouldUpdateStrip;
/*getpixel color to return color*/
static uint32_t getLogicColor(wire_state state){
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

void init_wire_lights(uint16_t offset, Adafruit_NeoPixel* strip, bool updateStrip, bool setupStrip) {
    use_strip = strip;
    stripOffset = offset;
    shouldUpdateStrip = updateStrip;

    if(setupStrip){
        use_strip->updateLength(LOGIC_WIRE_NUM_LEDS);
        use_strip->begin();
        use_strip->clear();
        use_strip->show();
    }
}

void update_wire_lights( logic_grid_wires* wires) {

}