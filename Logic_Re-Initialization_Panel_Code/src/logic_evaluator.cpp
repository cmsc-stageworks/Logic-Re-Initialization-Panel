#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "logic_board.h"
#include "cardParser.h"


static block_t
readSensorNibble(int sensorNum) {
    return getGate(sensorNum);
}



// TODO: Change to accept array of analog pin numbers?
void
readLogicGrid(logic_grid *output) {
    int sensorIndex = 0;
    for (int col = 0; col < LOGIC_BOARD_COLS; col++) {
        for (int row = 0; row < LOGIC_BOARD_ROWS; row++) {
            block_t blockNum = readSensorNibble(sensorIndex++);
            
            // If it's a double-wide
            if (blockNum & DW_BIT) {
                blockNum <<= 4;
                blockNum |= readSensorNibble(sensorIndex++);
                
                output->blocks[col][row] = (logic_block_type) blockNum;
                output->blocks[col][row+1] = EMPTY;
                row++;
            }
            else {
                output->blocks[col][row] = (logic_block_type) blockNum;
            }
        }
    }
}

#define REQUIRED_INPUT(x)  do { if ((x) == NOTHING) {x = UNFILLED_INPUT;} } while(0)
// #define ALL_DEFINED(x) (((x) & ~(TRUE|FALSE)) == 0)
// #define ANY_UNDEFINED(x) ((x) & (NOTHING|UNDEFINED))

// TODO: May or may not want ERROR in there. Would probably cause it to be overwritten.
//       If it's not, the case is unhandled, however.
#define UNDEFINED_MASK (NOTHING|UNDEFINED)

// TODO: Make these functions
// #define IS_UNDEFINED(x) ((x) & UNDEFINED_MASK)
// #define ANY_UNDEFINED(x,y) (((x)|(y)) & UNDEFINED_MASK)

static bool8 isUndefined(wire_state state) {
    return state & UNDEFINED_MASK;
}

static bool8 eitherUndefined(wire_state a, wire_state b) {
    return (a | b) & UNDEFINED_MASK;
}

void
populateLogicGridWireState(logic_grid *gridSetup, logic_grid_wires *gridWireState) {
    logic_block_type *logicBlockType = &gridSetup->blocks[0][0];
    wire_state *inputs = &gridWireState->wires[0][0];
    wire_state *outputs = &gridWireState->wires[1][0];
    
    for (int i = 0; i < LOGIC_BOARD_COLS*LOGIC_BOARD_ROWS;) {
        int slots;
        // TODO: Check validity of inputs.
        // TODO: Check validity of state every loop, for debugging purporses.
        switch (*logicBlockType) {
            //// Single-width stuffs
            case EMPTY: {
                slots = 1;
                outputs[0] = NOTHING;
            } break;
            case NOT: {
                slots = 1;
                outputs[1] = NOTHING;
                if (isUndefined(inputs[0])) {
                    outputs[0] = UNDEFINED;
                    REQUIRED_INPUT(inputs[0]);
                }
                else {
                    outputs[0] = (inputs[0] == TRUE) ? FALSE : TRUE;
                }
            } break;
            case STRAIGHT_WIRE: {
                slots = 1;
                outputs[0] = inputs[0];
            } break;
            
            
            //// Double-width wires
            case SWAPPER: {
                slots = 2;
                outputs[0] = inputs[1];
                outputs[1] = inputs[0];
            } break;
            case SPLITTER_TOP: {
                slots = 2;
                // If input is undefined or nothing
                if (inputs[0] & (NOTHING|UNDEFINED)) {
                    outputs[0] = UNDEFINED;
                    outputs[1] = UNDEFINED;
                    
                    REQUIRED_INPUT(inputs[0]);
                }
                else {
                    outputs[0] = inputs[0];
                    outputs[1] = inputs[0];
                }
            } break;
            case SPLITTER_BOTTOM: {
                slots = 2;
                // If input is undefined or nothing
                if (inputs[0] & (NOTHING|UNDEFINED)) {
                    outputs[0] = UNDEFINED;
                    outputs[1] = UNDEFINED;
                    
                    REQUIRED_INPUT(inputs[1]);
                }
                else {
                    outputs[0] = inputs[1];
                    outputs[1] = inputs[1];
                }
            } break;
            
            //// Double-width gates

// TODO: Consider passing `a` and `b` as variable names, for explicitness
// Double-wide gate operation.
#define DEFINE_DW_GATE_OP(defined_case)  \
    do {                                \
        slots = 2;                      \
        outputs[1] = NOTHING;           \
        if (eitherUndefined(inputs[0], inputs[1])) {    \
            outputs[0] = UNDEFINED;     \
            REQUIRED_INPUT(inputs[0]);  \
            REQUIRED_INPUT(inputs[1]);  \
        }                               \
        else {                          \
            bool8 a = inputs[0] == TRUE;\
            bool8 b = inputs[1] == TRUE;\
            outputs[0] = (defined_case) ? TRUE : FALSE;  \
        }                               \
    } while(0)                          \

            // TODO: Now that we have more of these, figure out how to condense them.
            case AND: {
                DEFINE_DW_GATE_OP(a && b);
                
                /* The above is equivalent to this, given the bitflag-like values for inputs
                if (inputs[0] == NOTHING || inputs[1] == NOTHING || inputs[1] == UNDEFINED || inputs[1] == UNDEFINED) {
                    outputs[1] = UNDEFINED;
                    
                    if (inputs[0] == NOTHING)
                        inputs[0] = UNFILLED_INPUT;
                    
                    if (inputs[1] == NOTHING)
                        inputs[1] = UNFILLED_INPUT;
                }
                else {
                    outputs[0] = ((inputs[0] == TRUE) && (inputs[1] == TRUE)) ? TRUE : FALSE;
                }
                */
            } break;
            
            case OR: {
                DEFINE_DW_GATE_OP(a || b);
            } break;
            
            case NAND: {
                DEFINE_DW_GATE_OP(!(a && b));
            } break;
            
            case NOR: {
                DEFINE_DW_GATE_OP(!(a || b));
            } break;
            
            case XOR: {
                DEFINE_DW_GATE_OP(a ^ b);
            } break;
            
            case XNOR: {
                DEFINE_DW_GATE_OP(!(a ^ b));
            } break;
            
            default: {
                outputs[0] = ERROR;
                slots = 1; // TODO: ERROR THIS
            }
        }
        
        i += slots;
        logicBlockType += slots;
        inputs += slots;
        outputs += slots;
    }
}