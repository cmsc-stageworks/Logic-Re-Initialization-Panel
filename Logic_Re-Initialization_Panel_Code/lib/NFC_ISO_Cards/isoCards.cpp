#include "isoCards.h"

#include <Adafruit_PN532.h>
#include "MainBoard.h"
#include <Wire.h>

static Adafruit_PN532* cardReaders;
static uint8_t* cardReadersMuxNums;
static ISO_CARD* cardsInserted;
static uint8_t numCardReaders;
static uint8_t initializedCardReaders; // pointer to what card has not been initialized
static const char* hexMap = "0123456789ABCDEF";

int initISOCards(uint8_t numCards){
    if(numCards > ISO_CARD_MAX_CARD_READERS || numCards == 0){
        return 1;
    }

    numCardReaders = numCards;

    if(cardsInserted != NULL){
        free(cardsInserted);
    }
    cardsInserted = (ISO_CARD*) calloc(numCardReaders, sizeof(ISO_CARD));

    if(cardReadersMuxNums != NULL){
        free(cardReadersMuxNums);
    }
    cardReadersMuxNums = (uint8_t*) calloc(numCardReaders, 1);

    if(cardReaders != NULL){
        free(cardReaders);
    }
    cardReaders = (Adafruit_PN532*) calloc(numCardReaders, sizeof(Adafruit_PN532));

    initializedCardReaders = 0;

    if(cardReaders == NULL || cardReadersMuxNums == NULL || cardsInserted == NULL){
        return 2;
    }

    return 0;
}


int8_t addISOCard(uint16_t slotID, int8_t muxNum = -1, int8_t i2cAddr = -1){
    if(initializedCardReaders == numCardReaders){
        return -1;
    }
    cardReadersMuxNums[initializedCardReaders] = muxNum;
    cardReaders[initializedCardReaders] = Adafruit_PN532(-1,-1, &Wire);
    cardReaders[initializedCardReaders].begin();

    uint32_t versiondata = cardReaders[initializedCardReaders].getFirmwareVersion();
    if (! versiondata) {
        return -2;
    }

    #if DEBUG_NFC_ISO_CARDS
    Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
    Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
    Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
    #endif

    initializedCardReaders++;
    return initializedCardReaders - 1;
}


void tickISOCards(){
    for(uint8_t index = 0; index < initializedCardReaders; index++){
        //TODO: READ CARDS
    }
}


ISO_CARD getISOCardBySlotID(uint16_t slotID){
    for(uint8_t index = 0; index < initializedCardReaders; index++){
        if(cardsInserted[index].slotID == slotID){
            return cardsInserted[index];
        }
    }
    ISO_CARD errVal;
    errVal.slotID = 0;
    return errVal;
}


ISO_CARD getISOCardByIndex(int8_t index){
    return cardsInserted[index];
}


String cardToString(ISO_CARD card, bool includeSlotID){
    String payload = "";
    for(int i = 0; i < card.payloadLen; i++){
        payload = payload + String(card.payload[i]);
    }
    String UUID = "";
    for(int i = 0; i < card.UUID_LEN; i++){
        UUID = UUID + String(hexMap[(card.UUID[i] >> 4) & 0x0F]) + String(hexMap[card.UUID[i] & 0x0F]) + (i == card.UUID_LEN - 1 ? "" : ":");
    }
    String strVal = "{" + (includeSlotID ? "\"slotID\":" + String(card.slotID) + "," : "") +
                    "\"payload\":\"" + payload + "\",\"UUID\":\"" + UUID + "\"}";
    return strVal;
}