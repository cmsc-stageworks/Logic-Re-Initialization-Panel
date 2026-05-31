#include <Arduino.h>
#include "logic_wire_lights.h"
#include "logic_evaluator.h"
#include <Adafruit_Neopixel.h>


static Adafruit_NeoPixel* use_strip;
static uint16_t stripOffset;
static bool shouldUpdateStrip;
/*getpixel color to return color*/
static uint32_t getLogicWireColor(wire_state state){
  switch(state){
    case NOTHING:
      return use_strip->Color(LOGIC_WIRE_NOTHING_COLOR);
    case FALSE:
      return use_strip->Color(LOGIC_WIRE_FALSE_COLOR);
    case TRUE:
      return use_strip->Color(LOGIC_WIRE_TRUE_COLOR);
    case UNDEFINED:
      return use_strip->Color(LOGIC_WIRE_UNDEFINED_COLOR);
    case ERROR:
      return use_strip->Color(LOGIC_WIRE_ERROR_COLOR);
    case UNFILLED_INPUT:
      return use_strip->Color(LOGIC_WIRE_UNFILLED_INPUT_COLOR);
    default:
      return use_strip->Color(LOGIC_WIRE_DEFAULT_COLOR);
  }
}

void init_wire_lights(uint16_t offset, Adafruit_NeoPixel* strip, bool updateStrip, bool setupStrip) {
    use_strip = strip;
    stripOffset = offset;
    shouldUpdateStrip = updateStrip;

    if(setupStrip){
        use_strip->updateLength(LOGIC_WIRE_NUM_LEDS);
        use_strip->begin();
        use_strip->setBrightness(LOGIC_WIRE_BRIGHTNESS);
        use_strip->fill(use_strip->Color(255,0,0));
        use_strip->show();
        delay(250);
        use_strip->fill(use_strip->Color(0,255,0));
        use_strip->show();
        delay(250);
        use_strip->fill(use_strip->Color(0,0,255));
        use_strip->show();
        delay(250);
        use_strip->clear();
        use_strip->show();
    }
}

void update_wire_lights(logic_grid_wires* wires) {
  for(int col = 0; col < 5; col++){
    for(int row = 0; row < 4; row++){
      use_strip->setPixelColor(row + 4 * col, getLogicWireColor(wires->wires[col][row]));
    }
  }
  if(shouldUpdateStrip){
    use_strip->show();
  }
}