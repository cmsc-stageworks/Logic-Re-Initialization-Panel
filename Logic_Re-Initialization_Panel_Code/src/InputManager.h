#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H
#include <stdint.h>

#define NUM_RESERVED_INPUTS 64
#define INPUT_FLAG_OUTPUT_ENABLE 0x0
#define INPUT_FLAG_OUTPUT_ACTIVE_HIGH 0x1
#define INPUT_FLAG_ACTUATED 0x7

#define INPUT_SET_FLAG(var, toSet) (var) |= (0x1 << (toSet))
#define INPUT_RESET_FLAG(var, toSet) (var) &= ~(0x1 << (toSet))

#define INPUT_GET_FLAG(var, flag) (((var) & 0x1 << (flag)) != 0)


enum PinInputMode: uint8_t {
    INPUT_IDLE = 0x00, // Do Not Update
    INPUT_EVENT, // Set actuated if set, reset manually in code
    INPUT_TOGGLE // Turn On and Off when pressed
};

typedef uint8_t PinInputFlags8_t;

struct PinInputStruct {
    PinInputMode mode;
    PinInputFlags8_t flags;
    uint8_t indicatorPinNumber;
};

void initInputs();
void addInput(uint8_t pinNumber, PinInputMode pinMode, int16_t outputIndicatorPinNumber = -1, bool outputActiveLow = true);
void tickInputs();
bool getInputState(uint8_t pin);
bool resetInputState(uint8_t pin);


#endif