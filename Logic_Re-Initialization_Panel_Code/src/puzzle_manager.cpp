#include "puzzle_manager.h"

// Private Variables
static const uint8_t portsUsedForGates[LOGIC_GATES_PORTS_USED] = {MAIN_BOARD_ANALOG_PORT_1, 
    MAIN_BOARD_ANALOG_PORT_2, MAIN_BOARD_ANALOG_PORT_3, MAIN_BOARD_ANALOG_PORT_4};

static Adafruit_NeoPixel* stripToUse;
static uint16_t pixelOffset;
static Adafruit_ST7796S* logicScreen;

static uint8_t logicTest = 0;

static uint16_t parsed_W, parsed_H;
static LOGIC_CARD* readValues;

static logic_grid_wires gridWires;
static logic_grid grid;

static bool isDemoMode = false;

// Static function prototypes
static void setupScreenDebug();
static void showScreenDebug(LOGIC_CARD* readValues,uint16_t parsed_W,uint16_t parsed_H, logic_grid_wires* logic);
static uint16_t getLogicColor(wire_state state);

// function definitions
void initPuzzleManager(Adafruit_NeoPixel* strip, uint16_t offset, Adafruit_ST7796S* screen){
    stripToUse = strip;
    logicScreen = screen;
    pixelOffset = offset;
    initISOCards(NUM_ISO_CARDS_LOGIC_PANEL);
    #if DEBUG_PUZZLE_MANAGER
    Serial.println("Started ISO card backend");
    #endif
    int8_t isoCardInit = addISOCard(LOGIC_GATE_SLOT_NUMBER, LOGIC_GATE_MUX_NUMBER);
    #if DEBUG_PUZZLE_MANAGER
    Serial.print("Started ISO card with code ");
    Serial.println(isoCardInit);
    #endif

    setupParseLogicCards(LOGIC_GATES_W, LOGIC_GATES_H, portsUsedForGates ,LOGIC_GATES_PORTS_USED);
    readValues = getParsedGates(&parsed_W, &parsed_H);

    init_wire_lights(0, stripToUse, true, true);

    #if DEBUG_PUZZLE_MANAGER
    Serial.print("Puzzle Started");
    #endif
}

void setPuzzleDemoMode(bool isSet){
    isDemoMode = isSet;
    if(isSet){
        setupScreenDebug();
    }
}

void tickPuzzleManager(){
    tickParseLogicCards();
    #if DEBUG_SHOW_GATE_STATUS
    Serial.println("Current Gate status:");
    for(int h = 0; h < 4; h++){
        for(int w = 0; w < 4; w++){
            Serial.printf("%02x ", readValues[h + w * parsed_W]);
        }
        Serial.println();
    }
    #endif

    if(isDemoMode){
        gridWires.wires[0][0] = ((logicTest & 0b00010000) != 0) ? TRUE : FALSE;
        gridWires.wires[0][1] = ((logicTest & 0b00100000) != 0) ? TRUE : FALSE;
        gridWires.wires[0][2] = ((logicTest & 0b01000000) != 0) ? TRUE : FALSE;
        gridWires.wires[0][3] = ((logicTest & 0b10000000) != 0) ? TRUE : FALSE;
        logicTest += 16;
    } else {
      static bool cardInsertedLastTime = false;
      tickISOCards();
      ISO_CARD readCard = getISOCardBySlotID(LOGIC_GATE_SLOT_NUMBER);

      if(readCard.isPopulated){
        static uint8_t curUUID[NTAG2XX_VALID_UID_LEN];
        // check to see if this is a new card, i.e. we should change the screen
        if(!cardInsertedLastTime || 0 != memcmp(curUUID, readCard.UUID, NTAG2XX_VALID_UID_LEN)){
          memcpy(curUUID, readCard.UUID, NTAG2XX_VALID_UID_LEN);
          char puzzleName[ISO_CARD_MAX_PAYLOAD_LEN];
          memset(puzzleName, 0, ISO_CARD_MAX_PAYLOAD_LEN);
          memcpy(puzzleName, readCard.payload, readCard.payloadLen);
          Serial.print("New Puzzle to load, Card Name: ");
          Serial.println(puzzleName);
        }
      }
      cardInsertedLastTime = readCard.isPopulated;
    }
    readLogicGrid(&grid);
    populateLogicGridWireState(&grid,&gridWires);

    update_wire_lights(&gridWires);

    #if DEBUG_SHOW_GATE_STATUS
    Serial.println("Grid State:");
    for(int h = 0; h < 4; h++){
        for(int w = 0; w < 4; w++){
            Serial.printf("%02x ", grid.blocks[w][h]);
            // Serial.print(grid.blocks[h][w],BIN);
            // Serial.print(' ');
        }
        Serial.println();
    }
    Serial.println();
    #endif
    if(isDemoMode){
      showScreenDebug(readValues, parsed_W, parsed_H, &gridWires);
    }
}


static void setupScreenDebug(){
  logicScreen->fillScreen(ILI9341_BLACK);
  for(int row = 0; row < 5; row++){
    logicScreen->drawLine(LCD_W/2 - LCD_H/2, ((LCD_H-1) * row)/4 , LCD_W/2 + LCD_H/2,  ((LCD_H-1) * row)/4, ILI9341_LIGHTGREY);
  }
  for(int col = 0; col < 5; col++){
    logicScreen->drawLine(LCD_W/2 - LCD_H/2 + LCD_H/4 * col, 0, LCD_W/2 - LCD_H/2  + LCD_H/4 * col,  LCD_H, ILI9341_LIGHTGREY);
  }
}

static void showScreenDebug(LOGIC_CARD* readValues,uint16_t parsed_W,uint16_t parsed_H, logic_grid_wires* logic){
  for(int row = 0; row < 4; row++){
    for(int col = 0; col < 4; col++){
      LOGIC_CARD currCard = readValues[col * 4 + row];
      bool populated = (currCard & 0x08) != 0;
      if(populated){
        Serial.printf("Row: %d Col: %d populated with value: %d\n", row, col, currCard);
      }
      logicScreen->drawChar(((LCD_H-1) * (col * 4 + 1))/16 + LCD_W/2 - LCD_H/2, ((LCD_H-1) * (row * 2 + 1))/8, 
        (populated ? ((currCard & 0x04) != 0) ? '1' : '0' : ' '), ILI9341_WHITE, ILI9341_BLACK, 1);
      logicScreen->drawChar(((LCD_H-1) * (col * 4 + 2))/16 + LCD_W/2 - LCD_H/2, ((LCD_H-1) * (row * 2 + 1))/8, 
        (populated ? ((currCard & 0x02) != 0) ? '1' : '0' : ' '), ILI9341_WHITE, ILI9341_BLACK, 1);
      logicScreen->drawChar(((LCD_H-1) * (col * 4 + 3))/16 + LCD_W/2 - LCD_H/2, ((LCD_H-1) * (row * 2 + 1))/8, 
        (populated ? ((currCard & 0x01) != 0) ? '1' : '0' : ' '), ILI9341_WHITE, ILI9341_BLACK, 1);
      int16_t rectX1 = ((LCD_H-1) * (col * 4))/16 + LCD_W/2 - LCD_H/2 + 4;
      int16_t rectY1 = ((LCD_H-1) * (row * 2 + 1))/8;
      logicScreen->fillRect(rectX1, rectY1, 10, 10, getLogicColor(logic->wires[col][row]));
      rectX1 = ((LCD_H-1) * (col * 4 + 4))/16 + LCD_W/2 - LCD_H/2 - 1;
      logicScreen->fillRect(rectX1, rectY1, -10, 10, getLogicColor(logic->wires[col + 1][row]));
    }
  }
}

static uint16_t getLogicColor(wire_state state){
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
