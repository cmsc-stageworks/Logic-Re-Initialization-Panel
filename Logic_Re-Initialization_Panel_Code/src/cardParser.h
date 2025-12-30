#ifndef CARD_PARSER_H
#include <Arduino.h>

// bit 7 denotes if the card is populated or not
typedef enum LOGIC_CARD : uint8_t {
    NOT_POPULATED_GATE = 0x00,
    WIRE_GATE = 0b1001,
    NOT_GATE = 0b1000,
    AND_GATE_TOP = 0b1111,
    AND_GATE_BOTTOM = 0b1000,
    OR_GATE_TOP = 0b1111,
    OR_GATE_BOTTOM = 0b1001,
    NAND_GATE_TOP = 0b1111,
    NAND_GATE_BOTTOM = 0b1010,
    NOR_GATE_TOP = 0b1111,
    NOR_GATE_BOTTOM = 0b1011,
    XOR_GATE_TOP = 0b1111,
    XOR_GATE_BOTTOM = 0b1100,
    NXOR_GATE_TOP = 0b1111,
    NXOR_GATE_BOTTOM = 0b1101,
    CROSSING_WIRES_GATE_TOP = 0b1110,
    CROSSING_WIRES_GATE_BOTTOM = 0b1000,
    SPLITTER_TOP_GATE_TOP = 0b1110,
    SPLITTER_TOP_GATE_BOTTOM = 0b1001,
    SPLITTER_BOTTOM_GATE_TOP = 0b1110,
    SPLITTER_BOTTOM_GATE_BOTTOM = 0b1010,
    POPULATED_READ_ERROR_GATE = 0b0111
} LOGIC_CARD;

#define LOGIC_GATES_TICK_PERIOD_MS 250

#define LOGIC_GATES_HALL_SENSOR_THRESHOLD 500

#define LOGIC_GATES_SENSORS_PER_GATE 3

/*
Setup card parser, defining pinout and hardware setup
@param W,H Dimensions of the port holder
@param List of ports used (Deep copied) It is assumed that you are using the PCB for the boards (therefore H is in multiples of 4) Order goes from Top to bottom, left to right
@param numPorts number of ports used
*/
void setupParseLogicCards(uint16_t W, uint16_t H, const uint8_t* ports, uint8_t numPorts);

/*
Read the current status of the cards, call regularly so updates are timely
@returns nothing
*/
void tickParseLogicCards();

/*
@returns Pointer to the array that holds the parsed gates
@param W Number of ports wide the gates read is
@param H Number of ports high the gates read is
*/
LOGIC_CARD* getParsedGates(uint16_t* W, uint16_t* H);

/*
@returns the gate that is at the index (Col-row wrap around)
@param index the index of the desired gate
*/
LOGIC_CARD getGate(uint16_t index);

#endif