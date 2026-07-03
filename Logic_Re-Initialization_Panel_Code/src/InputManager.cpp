#include "InputManager.h"
#include <unordered_map>
#include "MainBoard.h"

static std::unordered_map<uint8_t, PinInputStruct> inputs;

void initInputs(){
    inputs.reserve(NUM_RESERVED_INPUTS);
}

void addInput(uint8_t pinNumber, PinInputMode pinMode, int16_t outputIndicatorPinNumber, bool outputActiveLow){
    PinInputStruct newInput;
    newInput.flags = 0x0;
    newInput.mode = pinMode;
    if(outputIndicatorPinNumber < 0){
        newInput.indicatorPinNumber = 0x0;
    } else{
        newInput.indicatorPinNumber = outputIndicatorPinNumber;
        INPUT_SET_FLAG(newInput.flags, INPUT_FLAG_OUTPUT_ENABLE);
        mainBoardDigitalPinMode(outputIndicatorPinNumber, OUTPUT);
        if(outputActiveLow){
            mainBoardWriteDigitalOutput(outputIndicatorPinNumber, HIGH);
        } else {
            INPUT_SET_FLAG(newInput.flags, INPUT_FLAG_OUTPUT_ACTIVE_HIGH);
            mainBoardWriteDigitalOutput(outputIndicatorPinNumber, LOW);
        }
    }
    mainBoardDigitalPinMode(pinNumber, INPUT);
    inputs[pinNumber] = newInput;
}

void tickInputs(){
    for (auto input = inputs.begin(); input != inputs.end(); ++input) {
        //it->first, it->second
        bool state = !mainBoardGetDigitalInput(input->first);
        switch(input->second.mode){
            case INPUT_EVENT:
                if(!INPUT_GET_FLAG(input->second.flags, INPUT_FLAG_ACTUATED)){
                    if(state){
                        INPUT_SET_FLAG(input->second.flags, INPUT_FLAG_ACTUATED);
                    }
                }
            break;
            case INPUT_TOGGLE:
                if(state){
                    if(!INPUT_GET_FLAG(input->second.flags, INPUT_FLAG_ACTUATED)){
                        INPUT_SET_FLAG(input->second.flags, INPUT_FLAG_ACTUATED);
                    } else{
                        INPUT_RESET_FLAG(input->second.flags, INPUT_FLAG_ACTUATED);
                    }
                }
            break;
        }
    }
}

bool getInputState(uint8_t pin){
    return INPUT_GET_FLAG(inputs[pin].flags, INPUT_FLAG_ACTUATED);
}

bool resetInputState(uint8_t pin){
    return INPUT_RESET_FLAG(inputs[pin].flags, INPUT_FLAG_ACTUATED);
}