#include <Arduino.h>
#include <SPI.h>
#include <SPISlave.h>

#define MISO 15
#define MOSI 12
#define SCK 14
#define MASTER_CS 13
#define MISO1 0
#define MOSI1 3
#define SCK1 2

#define SPI_FREQUENCIES (80 * 1000 * 1000)
#define RX_BUFFER_SIZE 256
#define TX_BUFFER_SIZE 256


char msg[RX_BUFFER_SIZE];
char backtalk[TX_BUFFER_SIZE];

SPISettings spisettings(SPI_FREQUENCIES, MSBFIRST, SPI_MODE0);

void setup() {
  SPI1.setRX(MISO);
  SPI1.setTX(MOSI);
  SPI1.setSCK(SCK);
  SPI1.begin();
  
}

void loop() {
  delay(5000);
  SPI1.beginTransaction(spisettings);
  sprintf(msg, "Current Millis: %ld\r\n", millis());
  SPI.transferAsync(msg, backtalk, sizeof(msg));
  SPI1.endTransaction();
}





volatile bool newDataReceived = false;
int8_t rxBuffer[RX_BUFFER_SIZE];
void recvCallback(uint8_t *data, size_t len){
  for(int i = 0; i < len; i++){
    rxBuffer[i] = data[i];
  }
  newDataReceived = true;
}

int8_t txBuffer[TX_BUFFER_SIZE];
size_t txSize = 1;
void sentCallback(){
  SPISlave1.setData((uint8_t*)txBuffer, txSize);
}


void setup1() {
  SPISlave.setRX(MISO1);
  SPISlave.setTX(MOSI1);
  SPISlave.setSCK(SCK1);
  SPISlave.setCS(MASTER_CS);
  SPISlave.onDataRecv(recvCallback);
  SPISlave.onDataSent(sentCallback);
  SPISlave.begin(spisettings);
  
  delay(3000);
  Serial.println("S-INFO: SPISlave started");
}

void loop1() {
  if(newDataReceived){
    Serial.print("Master Sent:");
    Serial.println((char*) rxBuffer);
    newDataReceived = false;
  }
}
