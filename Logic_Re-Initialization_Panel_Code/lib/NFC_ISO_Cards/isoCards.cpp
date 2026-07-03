#include "isoCards.h"

#include <Adafruit_PN532.h>
#include "MainBoard.h"
#include <Wire.h>
#include <string.h>
#include <Adafruit_Neopixel.h>
#include <esp_random.h>

#define DEBUG_NFC_ISO_CARDS true


static Adafruit_PN532* cardReaders;
static Adafruit_NeoPixel* pixelDriver;
static uint8_t* cardReadersMuxNums;
static ISO_CARD* cardsInserted;
static uint8_t numCardReaders;
static uint8_t initializedCardReaders; // pointer to what card has not been initialized
static const char* hexMap = "0123456789ABCDEF";


static void cardReadingAnimation(int16_t offset);
static void cardUnpopulatedAnimation(int16_t offset);


int32_t initISOCards(uint8_t numCards, Adafruit_NeoPixel* driver){
    if(numCards > ISO_CARD_MAX_CARD_READERS || numCards == 0){
        return 1;
    }

    pixelDriver = driver;

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


int8_t addISOCard(uint16_t slotID, int8_t muxNum, int16_t offset){
    if(initializedCardReaders == numCardReaders){
        return -1;
    }

    mainBoardSetI2CBus(muxNum);

    cardReadersMuxNums[initializedCardReaders] = muxNum;
    cardReaders[initializedCardReaders] = Adafruit_PN532(-1,-1, &Wire);
    bool started = cardReaders[initializedCardReaders].begin();

    if(!started){
        return -2;
    }

    cardsInserted[initializedCardReaders].slotID = slotID;

    cardsInserted[initializedCardReaders].pixelOffset = offset;

    uint32_t versiondata = cardReaders[initializedCardReaders].getFirmwareVersion();
    if (! versiondata) {
        return -3;
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
    uint8_t success;
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
    uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    for(uint8_t index = 0; index < initializedCardReaders; index++){
        //attempt to read card
        mainBoardSetI2CBus(cardReadersMuxNums[index]);
        success = cardReaders[index].readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, ISO_CARD_TIMEOUT_LEN_MS);
        //Check to see if a card was read, only do the first time payload copy if it is a new card or was re-inserted
        if(success && (!cardsInserted[index].isPopulated || 0 != memcmp(uid, cardsInserted[index].UUID, NTAG2XX_VALID_UID_LEN))){
            Serial.println("Read Card!");
            Serial.print("UID Len: ");
            Serial.println(uidLength);
            // NTAG 2XX cards have UID length == 7
            if (uidLength == NTAG2XX_VALID_UID_LEN){
                //save card info
                cardsInserted[index].isPopulated = true;
                cardsInserted[index].UUID_LEN = uidLength;
                memcpy(cardsInserted[index].UUID, uid, NTAG2XX_VALID_UID_LEN);

                //retrieve payload
                uint8_t data[NTAG2XX_BYTES_PER_PAGE];
                success = cardReaders[index].ntag2xx_ReadPage(ISO_CARD_PAYLOAD_HEADER_PAGE, data);
                cardsInserted[index].payloadLen = data[0] > ISO_CARD_MAX_PAYLOAD_LEN ? ISO_CARD_MAX_PAYLOAD_LEN : data[0];

                
                Serial.print(success ? "Read from card: ": "Error reading from card: ");
                Serial.print(cardsInserted[index].payloadLen);
                Serial.print(" ");
                Serial.println(data[0]);

                // we copy the whole payload to just be safe (invalid length programmed on card), but don't count on that data meaning anything!
                for(uint32_t payloadPage = 0; payloadPage * NTAG2XX_BYTES_PER_PAGE < cardsInserted[index].payloadLen; payloadPage++){
                    success = cardReaders[index].ntag2xx_ReadPage(ISO_CARD_PAYLOAD_PAGE(payloadPage), data);
                    if(!success){ // retry if failed to read page, for resiliance
                        #if DEBUG_NFC_ISO_CARDS
                        Serial.print("Error reading payload of ISO card! Read error occured on Page: ");
                        Serial.print(payloadPage);
                        Serial.println(" Retrying...");
                        #endif
                        success = cardReaders[index].ntag2xx_ReadPage(ISO_CARD_PAYLOAD_PAGE(payloadPage), data);
                    }
                    if(success){
                        memcpy(cardsInserted[index].payload + payloadPage * NTAG2XX_BYTES_PER_PAGE, data, NTAG2XX_BYTES_PER_PAGE);
                    } else { // couldn't read entire payload
                        #if DEBUG_NFC_ISO_CARDS
                        Serial.print("ABORTING CARD READ: Error reading payload of ISO card! Read error occured on Page: ");
                        Serial.println(payloadPage);
                        #endif
                        cardsInserted[index].isPopulated = false;
                        break;
                    }
                }
                Serial.println((char*) cardsInserted[index].payload);
            }
        }
        else if(!success){ // only eject the card once it has not been detected, not if we didn't read its payload
            cardsInserted[index].isPopulated = false;
        }

        if(cardsInserted[index].isPopulated){
            cardReadingAnimation(cardsInserted[index].pixelOffset);
        } else {
            cardUnpopulatedAnimation(cardsInserted[index].pixelOffset);
        }
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


bool attemptToWriteToCard(ISO_CARD& cardConfig){
    bool success = true;
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
    uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    cardConfig.payload;

    int32_t slotIndex = -1;

    for(int32_t i = 0; i < initializedCardReaders; i++){
        if(cardsInserted[i].slotID == cardConfig.slotID){
            slotIndex = i;
            break;
        }
    }

    
    Serial.print("Card Slot ID:");
    Serial.println(slotIndex);

    //attempt write
    mainBoardSetI2CBus(cardReadersMuxNums[slotIndex]);
    success = (slotIndex >= 0) && cardReaders[slotIndex].readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, ISO_CARD_TIMEOUT_LEN_MS);
    


    if(success){
        Serial.println("Card detected, beginning write");
        uint8_t headerContents[NTAG2XX_BYTES_PER_PAGE];
        headerContents[0] = cardConfig.payloadLen;
        success &= cardReaders[slotIndex].ntag2xx_WritePage(ISO_CARD_PAYLOAD_HEADER_PAGE, headerContents);
        if(!success){
            success = cardReaders[slotIndex].ntag2xx_WritePage(ISO_CARD_PAYLOAD_HEADER_PAGE, headerContents);
        }
        if(success){
            for(uint32_t page = 0; page * NTAG2XX_BYTES_PER_PAGE < cardConfig.payloadLen; page++){
                Serial.print("Preparing payload for page: ");
                Serial.println(ISO_CARD_PAYLOAD_PAGE(page));
                uint8_t buffer[NTAG2XX_BYTES_PER_PAGE];
                memcpy(buffer, &cardConfig.payload[page * NTAG2XX_BYTES_PER_PAGE], NTAG2XX_BYTES_PER_PAGE);
                Serial.print("Writing payload to page: ");
                Serial.println(ISO_CARD_PAYLOAD_PAGE(page));
                success &= cardReaders[slotIndex].ntag2xx_WritePage(ISO_CARD_PAYLOAD_PAGE(page), buffer);
                if(!success){ // retry if failed to read page, for resiliance
                    #if DEBUG_NFC_ISO_CARDS
                    Serial.print("Error writing to payload of ISO card! Write error occured on Page: ");
                    Serial.print(page);
                    Serial.println(" Retrying...");
                    #endif
                    success = cardReaders[slotIndex].ntag2xx_WritePage(ISO_CARD_PAYLOAD_PAGE(page), buffer);
                }
                if(!success){
                    #if DEBUG_NFC_ISO_CARDS
                    Serial.print("Error writing to payload of ISO card! Write error occured on Page: ");
                    Serial.print(page);
                    Serial.println(" Aborting!");
                    #endif
                    break;
                }
            }
        }
    }

    return success;
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


#define ISO_CARD_RED_TO_GREEN(intensity) pixelDriver->Color(intensity, (255 - intensity), 0)


static void cardReadingAnimation(int16_t offset) {
    static int32_t randVal = random(0, ISO_CARDS_NUM_LEDS_PER_CARD * 255);
    randVal += random(-255, 255);
    if(randVal > ISO_CARDS_NUM_LEDS_PER_CARD * 255){
        randVal = ISO_CARDS_NUM_LEDS_PER_CARD * 255;
    } else if(randVal < 0){
        randVal = 0;
    }
    for(int8_t i = 0; i < ISO_CARDS_NUM_LEDS_PER_CARD; i++){
        int16_t intensity = randVal > 255 * (1 + i) ? 255 : (randVal <  255 * i ? 0 : randVal - 255 * i);
        pixelDriver->setPixelColor(offset + i, ISO_CARD_RED_TO_GREEN(intensity));
    }
}


static void cardUnpopulatedAnimation(int16_t offset) {
    pixelDriver->fill(0, offset, ISO_CARDS_NUM_LEDS_PER_CARD);
}