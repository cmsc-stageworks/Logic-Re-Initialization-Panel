#include "cardParser.h"
#include "MainBoard.h"

/*
Yeah, I'm sorry for this one, I made this PCB a long time ago and I don't know why I didn't wire it straight through, 
it made sense at the time but now eludes me. Please forgive me :)
*/
static const uint8_t portToPinDemux[4][LOGIC_GATES_SENSORS_PER_GATE] = {{0x2,0x4,0x0}, {0x3,0x1,0x5}, {0x9,0x7,0xB}, {0xA, 0x8, 0x6}};

bool errorFlag = true;

static uint8_t* portsToRead = NULL;
static uint8_t numPortsToRead = 0;

static LOGIC_CARD* cards;
static uint16_t H_ports, W_ports;

static uint32_t calibVal = 2000; // this was found using experimentation TODO: allow user calibration and storing of it

static uint32_t avgPortCalib(uint8_t port){
    uint64_t avgVal = 0;
    for(int gateSlot = 0; gateSlot < 4; gateSlot++) {
        for(int sensor = 0; sensor < 3; sensor++){
            avgVal += mainBoardGetAnalogMux(portToPinDemux[gateSlot][sensor] + 
                (port - MAIN_BOARD_ANALOG_PORT_1) * MAIN_BOARD_ANALOG_PINS_PER_PORT);
        }
    }
    avgVal = avgVal / 12;
    return avgVal;
}

static void calibrateCards(){
    uint64_t avgVal = 0;
    for(uint8_t port = 0; port < numPortsToRead; port++){
        avgVal += avgPortCalib(portsToRead[port]);
    }
    avgVal = avgVal / numPortsToRead;
    calibVal = avgVal;
}

void setupParseLogicCards(uint16_t W, uint16_t H, const uint8_t* ports, uint8_t numPorts){
    if(numPorts == 0 || H % 4 != 0){
        errorFlag = true;
        return;
    }
    numPortsToRead = numPorts;
    if(portsToRead != NULL){
        free(portsToRead);
    }
    if(cards != NULL){
        free(cards);
    }
    portsToRead = (uint8_t*) malloc(numPorts);
    for(int i = 0; i < numPorts; i++){
        portsToRead[i] = ports[i];
        if(portsToRead[i] < MAIN_BOARD_ANALOG_PORT_1 || portsToRead[i] > MAIN_BOARD_ANALOG_PORT_8){
            errorFlag = true;
            return;
        }
    }
    H_ports = H;
    W_ports = W;
    cards = (LOGIC_CARD*) malloc(W * H * sizeof(LOGIC_CARD));
    errorFlag = false;
}

static LOGIC_CARD sensorReadingsToGate(int32_t* sensors){
    LOGIC_CARD result = (LOGIC_CARD) 0x80; // assume it is populated
    for(int sensor = 0; sensor < LOGIC_GATES_SENSORS_PER_GATE; sensor++){
        if(sensors[sensor] >= calibVal + LOGIC_GATES_HALL_SENSOR_THRESHOLD){
            result = (LOGIC_CARD) (result | (0x1 << sensor));
        } else if(sensors[sensor] <= calibVal - LOGIC_GATES_HALL_SENSOR_THRESHOLD){
            // the bit is by default zero, so do nothing. I left this statement here just in case it is needed in the future/for clarity.
        } else{
            result = NOT_POPULATED_GATE; //clear the populated bit
            break;
        }
    }

    return result;
}

static LOGIC_CARD* parsePort(uint8_t portNum){
    static LOGIC_CARD result[4];
    for(int gateSlot = 0; gateSlot < 4; gateSlot++) {
        static int32_t sensorLine[3];
        for(int sensor = 0; sensor < 3; sensor++){
            sensorLine[sensor] = mainBoardGetAnalogMux(portToPinDemux[gateSlot][sensor] + 
                (portNum - MAIN_BOARD_ANALOG_PORT_1) * MAIN_BOARD_ANALOG_PINS_PER_PORT);
        }
        result[gateSlot] = sensorReadingsToGate(sensorLine);
    }
}

static void updateAndParseLogicCards(){
    for(uint16_t port = 0; port < numPortsToRead; port++){
        LOGIC_CARD* portResult = parsePort(portsToRead[port]);
    }
}

void tickParseLogicCards(){
    if(errorFlag){
        return;
    }
    static unsigned long lastTicked = 0;
    if(millis() - lastTicked >= LOGIC_GATES_TICK_PERIOD_MS || lastTicked > millis()){
        lastTicked = millis();
        updateAndParseLogicCards();
    }
}

LOGIC_CARD* getParsedGates(uint16_t* W, uint16_t* H);