#ifndef ISO_CARDS_H
#define ISO_CARDS_H
#include <Adafruit_PN532.h>
#include <Arduino.h>

#include <String>

//max payload length must be divisible by NTAG2XX_BYTES_PER_PAGE with no remainder
#define ISO_CARD_MAX_PAYLOAD_LEN 32
#define ISO_CARD_TIMEOUT_LEN_MS 50
#define NTAG2XX_VALID_UID_LEN 7
#define NTAG2XX_BYTES_PER_PAGE 4
#define NTAG2XX_START_USER_PAGE 4
#define ISO_CARD_PAYLOAD_HEADER_PAGE NTAG2XX_START_USER_PAGE
#define ISO_CARD_PAYLOAD_PAGE(num) (ISO_CARD_PAYLOAD_HEADER_PAGE + 1 + (num))


#define ISO_CARD_MAX_CARD_READERS 8

typedef struct ISO_CARD {
    uint16_t slotID;
    uint8_t payload[ISO_CARD_MAX_PAYLOAD_LEN];
    uint8_t UUID[NTAG2XX_VALID_UID_LEN];
    uint8_t UUID_LEN;
    uint8_t payloadLen;
    bool isPopulated;
} ISO_CARD;

/*
Initializes the iso-card interface, pre-allocating space needed for interface
@param numCards the number of cards in the buffer
@returns 0 if successfull or !0 if an error occured
*/
int initISOCards(uint8_t numCards);

/*
Updates all the card slots.
*/
void tickISOCards();

/*
Adds an iso card to the buffer
@param slotID the thorium slot ID
@param muxNum the I2C bus on the main board we are connected to -1 if not using the mux
@returns the index of the card, or a negative number if an error was encountered
*/
int8_t addISOCard(uint16_t slotID, int8_t muxNum = -1);

/*
@param slotID the slot that you wish to read (By thorium ID)
@returns the ISO card in the slot
*/
ISO_CARD getISOCardBySlotID(uint16_t slotID);

/*
@param index the slot by index that you wish to read
@returns the ISO card in the slot
*/
ISO_CARD getISOCardByIndex(int8_t index);

/*
@param card info that you want written
@returns operation success (True) or failure (False)
*/
bool attemptToWriteToCard(ISO_CARD& cardConfig);

/*
@param card the Card you want to serialize
@param includeSlotID include the Slot ID in the JSON (MQTT Server defines topic by Slot ID, so it would be redundant to include it)
@returns the JSON string of the card
*/
String cardToString(ISO_CARD card, bool includeSlotID = false);


#endif