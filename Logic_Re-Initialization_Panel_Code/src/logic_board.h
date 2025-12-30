#ifndef LOGIC_BOARD_H
#define LOGIC_BOARD_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#define LOGIC_BOARD_ROWS 4
#define LOGIC_BOARD_COLS 4
#define LOGIC_BOARD_SENSORS_PER_BLOCK 4

#define DW(first, second) (((second) << 4) | (first))
// This defines the bit that marks a double-wide gate.
#define DW_BIT 0b0100

typedef uint8_t bool8;

// This defines what size number holds the blocks and wire states.
typedef uint8_t block_t;
typedef uint8_t wire_t;

enum logic_block_type : block_t {
    // Single-width stuffs
    EMPTY = 0,
    NOT = 0b1000,
    STRAIGHT_WIRE = 0b1001,
    // Bit 0b0100 being set defines a double-wide gate.
    // The available double-wide gates 0b1100 and 0b1101
    DOUBLE_WIDE_WIRE = 0b1110,
    DOUBLE_WIDE_GATE = 0b1111,
    
    // Double-wide wires
    SWAPPER = DW(DOUBLE_WIDE_WIRE, 0b1000),
    SPLITTER_BOTTOM = DW(DOUBLE_WIDE_WIRE, 0b1001), 
    SPLITTER_TOP = DW(DOUBLE_WIDE_WIRE, 0b1010),
    
    
    // Double-wide gates
    AND = DW(DOUBLE_WIDE_GATE, 0b1000),
    OR = DW(DOUBLE_WIDE_GATE, 0b1001),
    NAND = DW(DOUBLE_WIDE_GATE, 0b1010),
    NOR = DW(DOUBLE_WIDE_GATE, 0b1011),
    XOR = DW(DOUBLE_WIDE_GATE, 0b1100),
    XNOR = DW(DOUBLE_WIDE_GATE, 0b1101),
    
};

// TODO: Logic code will become much simpler if FALSE = 0 and TRUE = 1
//       Then you can use use normal boolean logic.
//       This should be possible, since there aren't any flag-like logic with them right now.
//       One problem this might cause is that it wouldn't be as easy to identify undefined things.
enum wire_state : wire_t {
    NOTHING = 0x1,
    FALSE = 0x2,
    TRUE = 0x4,
    UNDEFINED = 0x8,
    ERROR = 0x10,
    UNFILLED_INPUT = 0x20,
};

struct logic_grid {
    logic_block_type blocks[LOGIC_BOARD_COLS][LOGIC_BOARD_ROWS];
};

struct logic_grid_wires {
    wire_state wires[LOGIC_BOARD_COLS+1][LOGIC_BOARD_ROWS];
};

void readLogicGrid(logic_grid *output);

void populateLogicGridWireState(logic_grid *gridSetup, logic_grid_wires *gridWireState);

#endif