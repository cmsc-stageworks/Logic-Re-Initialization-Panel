#include "puzzle_manager.h"

static const uint8_t inputMuxPinsLogic[NUM_LOGIC_INPUTS] = {73, 72, 70, 71};


static const uint8_t inputUIButtonInputs[NUM_UI_BTNS] = {16,18,20,22};
static const uint8_t inputUIButtonIndicators[NUM_UI_BTNS] = {17,19,21,23};

// Private Variables
static const uint8_t portsUsedForGates[LOGIC_GATES_PORTS_USED] = {MAIN_BOARD_ANALOG_PORT_1, 
    MAIN_BOARD_ANALOG_PORT_2, MAIN_BOARD_ANALOG_PORT_3, MAIN_BOARD_ANALOG_PORT_4};

static Adafruit_NeoPixel* stripToUse;
static uint16_t pixelOffset;
static Adafruit_ST7796S* logicScreen;
extern SdFat mainBoardSD;
static Adafruit_ImageReader imgReader(mainBoardSD);

static uint8_t logicTest = 0;
PUZZLE_STATES currPuzzleStatus = NOT_IN_PUZZLE;

static uint16_t parsed_W, parsed_H;
static LOGIC_CARD* readValues;
static ProblemConfig currProblemConfig;

static logic_grid_wires gridWires;
static logic_grid grid;

static bool isDemoMode = false;

// Static function prototypes
static void setupScreenDebug();
static void showScreenDebug(LOGIC_CARD* readValues,uint16_t parsed_W,uint16_t parsed_H, logic_grid_wires* logic);
static uint16_t getLogicColor(wire_state state);
static bool loadNewPuzzle(char* puzzleName);

// function definitions
void initPuzzleManager(Adafruit_NeoPixel* strip, uint16_t offset, Adafruit_ST7796S* screen){
    stripToUse = strip;
    logicScreen = screen;
    pixelOffset = offset;

    for(uint8_t uiBtn = 0; uiBtn < NUM_UI_BTNS; uiBtn++){
      mainBoardDigitalPinMode(inputUIButtonInputs[uiBtn], INPUT);
      mainBoardDigitalPinMode(inputUIButtonIndicators[uiBtn], OUTPUT);
      mainBoardWriteDigitalOutput(inputUIButtonIndicators[uiBtn], HIGH);
    }

    //Do a quick flash so users can verify if the button lights are operational
    delay(100);
    for(uint8_t uiBtn = 0; uiBtn < NUM_UI_BTNS; uiBtn++){
      mainBoardWriteDigitalOutput(inputUIButtonIndicators[uiBtn], LOW);
    }
    // delay(100);
    // for(uint8_t uiBtn = 0; uiBtn < NUM_UI_BTNS; uiBtn++){
    //   mainBoardWriteDigitalOutput(inputUIButtonIndicators[uiBtn], HIGH);
    // }

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
    // Parse Logic Cards
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

    // Update Test Logic
    #if DEBUG_COUNT_LOGIC_INPUTS
    gridWires.wires[0][0] = ((logicTest & 0b00010000) != 0) ? TRUE : FALSE;
    gridWires.wires[0][1] = ((logicTest & 0b00100000) != 0) ? TRUE : FALSE;
    gridWires.wires[0][2] = ((logicTest & 0b01000000) != 0) ? TRUE : FALSE;
    gridWires.wires[0][3] = ((logicTest & 0b10000000) != 0) ? TRUE : FALSE;
    logicTest += 16;
    #else
    // TODO: read user input
    #endif

    // Check ISO card, ensure that we have proper puzzle loaded
    if(!isDemoMode) {
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
          #if DEBUG_PUZZLE_MANAGER
          Serial.print("New Puzzle to load, Card Name: ");
          Serial.println(puzzleName);
          #endif
          if(!loadNewPuzzle(puzzleName)){
            #if DEBUG_PUZZLE_MANAGER
            Serial.println("Unable to load puzzle associated with ISO Card!");
            #endif
            currPuzzleStatus = NOT_IN_PUZZLE;
          } else{
            currPuzzleStatus = IN_PUZZLE;
          }
        }
      } else {
        if(currPuzzleStatus != NOT_IN_PUZZLE){
          currPuzzleStatus = NOT_IN_PUZZLE;
          Adafruit_Image image;
          ImageReturnCode retVal = imgReader.loadBMP(LOGIC_BOARD_LOGO_PATH, image);
          if(retVal == IMAGE_SUCCESS){
            image.draw(*logicScreen, 0, 0);
          } else {
            Serial.println("Error Printing Logo!");
          }
        }
      }
      cardInsertedLastTime = readCard.isPopulated;
    }
    

    // Perform Logic Gate Evaluations
    readLogicGrid(&grid);
    populateLogicGridWireState(&grid,&gridWires);

    if(currPuzzleStatus == IN_PUZZLE && mainBoardGetDigitalInput(LOGIC_UI_BTN_SELECT) == 0){
      if(validate_board(&grid, &currProblemConfig)){
        Serial.println("Solution was correct!");
        currPuzzleStatus = FINISHED_PUZZLE;
      } else {
        Serial.println("Solution was incorrect!");
        currPuzzleStatus = INCORRECT_SOLUTION;
      }
    }

    // Update user output
    if(currPuzzleStatus == IN_PUZZLE || isDemoMode){
      update_wire_lights(&gridWires);
    } else if (currPuzzleStatus == FINISHED_PUZZLE){
      wire_lights_correct();
    } else if(currPuzzleStatus == INCORRECT_SOLUTION){
      wire_lights_incorrect();
    } else if (currPuzzleStatus == NOT_IN_PUZZLE){
      wire_lights_inactive();
    }

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

    // show debug screen if applicable
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

static bool loadNewPuzzle(char* puzzleName){
  bool success = true;
  // Prepare the filePath
  String filePath = String(PUZZLE_PATH) + String(puzzleName);
  // Make sure it is a valid directory
  if(mainBoardSD.exists(filePath)){
    #if DEBUG_PUZZLE_MANAGER
    Serial.println("Found Puzzle Folder Locally!");
    #endif

    // find and open the solution path
    String solutionPath = filePath + "/" + String(PUZZLE_SOLUTION_FILENAME);
    if(mainBoardSD.exists(solutionPath)){
      //Open solution for reading
      FatFile solutionFile = mainBoardSD.open(solutionPath, O_RDONLY);

      // Prepare the file buffer
      char solutionFileBuffer[SOLUTION_FILE_BUFFER_SIZE];
      uint32_t bytesRead = 0;
      memset(solutionFileBuffer, 0, SOLUTION_FILE_BUFFER_SIZE);
      solutionFile.rewind();
      // copy file to the buffer
      uint32_t lineBytesRead = 0;
      do {
        lineBytesRead = solutionFile.fgets(solutionFileBuffer + bytesRead, SOLUTION_FILE_BUFFER_SIZE - bytesRead, "");
        bytesRead += lineBytesRead;
      } while(lineBytesRead > 0);
      
      // Vestigal file reading (Slower)
      // while(byteRead != -1 && bytesRead < SOLUTION_FILE_BUFFER_SIZE){
      //   solutionFileBuffer[bytesRead] = byteRead;
      //   bytesRead++;
      //   byteRead = solutionFile.read();
      // }
      solutionFile.close();

      #if DEBUG_PUZZLE_MANAGER
      Serial.println("Solution File Read:");
      Serial.println(solutionFileBuffer);
      Serial.println("EOF");
      #endif

      if(bytesRead < MIN_SOL_FILESIZE){
        #if DEBUG_PUZZLE_MANAGER
        Serial.println("Solution File too short!");
        #endif
        success = false;
      } else {
        int32_t parseStatus = parse_config(solutionFileBuffer, bytesRead, &currProblemConfig);
        //if we parsed the config correctly
        if(parseStatus == -1){
          Adafruit_Image image;
          String promptPath = filePath + "/" + String(PUZZLE_PROMPT_FILENAME);
          ImageReturnCode retVal = imgReader.loadBMP(promptPath.c_str(), image);
          if(retVal == IMAGE_SUCCESS){
            image.draw(*logicScreen, 0, 0);
          } else {
            #if DEBUG_PUZZLE_MANAGER
            Serial.println("Error Printing Image!");
            #endif
            // Not a fatal error, maybe the puzzle is in an "emergancy guide" or something simillar, if needed this will change later
          }
        } else{
          #if DEBUG_PUZZLE_MANAGER
          Serial.print("Error Parsing Solution on character: ");
          Serial.println(parseStatus);
          #endif
          success = false;
        }
      }
    } else{
      #if DEBUG_PUZZLE_MANAGER
      Serial.print("Error: Solution File Does Not Exist: ");
      Serial.println(solutionPath);
      #endif
      success = false;
    }
  } else{
    #if DEBUG_PUZZLE_MANAGER
    Serial.println("Puzzle folder could not be found!");
    #endif
    success = false;
  }
  return success;
}
