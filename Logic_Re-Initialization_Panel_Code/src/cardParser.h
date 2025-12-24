#ifndef CARD_PARSER_H
#include <Arduino.h>

// bit 7 denotes if the card is populated or not
typedef enum LOGIC_CARD : uint8_t {
    NOT_POPULATED_GATE = 0x00,
    WIRE_GATE = 0x81,
    NOT_GATE = 0x80,
    AND_GATE_TOP = 0x87,
    AND_GATE_BOTTOM = 0x80,
    OR_GATE_TOP = 0x87,
    OR_GATE_BOTTOM = 0x81,
    NAND_GATE_TOP = 0x87,
    NAND_GATE_BOTTOM = 0x82,
    NOR_GATE_TOP = 0x87,
    NOR_GATE_BOTTOM = 0x83,
    XOR_GATE_TOP = 0x87,
    XOR_GATE_BOTTOM = 0x84,
    NXOR_GATE_TOP = 0x87,
    NXOR_GATE_BOTTOM = 0x85,
    CROSSING_WIRES_GATE_TOP = 0x86,
    CROSSING_WIRES_GATE_BOTTOM = 0x80,
    SPLITTER_TOP_GATE_TOP = 0x86,
    SPLITTER_TOP_GATE_BOTTOM = 0x81,
    SPLITTER_BOTTOM_GATE_TOP = 0x86,
    SPLITTER_BOTTOM_GATE_BOTTOM = 0x82,
    POPULATED_READ_ERROR_GATE = 0x80
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

#endif