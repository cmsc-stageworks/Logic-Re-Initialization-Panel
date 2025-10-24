#include <Arduino.h>
#include <SPI.h>
#include <SPISlave.h>

// spi 0 is master, spi 1 is slave
#define MISO 15
#define MOSI 12
#define SCK 14
#define MASTER_CS 5
#define MISO1 0
#define MOSI1 3
#define SCK1 2
#define SPI_SLAVE_CS 13

#define SPI_FREQUENCIES (1 * 1000 * 1000)
#define RX_BUFFER_SIZE 256
#define TX_BUFFER_SIZE 256

char msg[RX_BUFFER_SIZE];
char backtalk[TX_BUFFER_SIZE];

SPISettings spisettings(SPI_FREQUENCIES, MSBFIRST, SPI_MODE0);

void setup() {
  delay(4000);
  SPI.setRX(MISO1);
  SPI.setTX(MOSI1);
  SPI.setSCK(SCK1);
  SPI.setCS(MASTER_CS);
  SPI.begin(true);

  Serial.println("Master Started");
  
}

void loop() {
  delay(5000);
  SPI.beginTransaction(spisettings);
  sprintf(msg, "Current Millis: %ld\r\n", millis());
  Serial.print(msg);
  SPI.transferAsync(msg, backtalk, sizeof(msg));
  SPI.endTransaction();
}





volatile bool newDataReceived = false;
int8_t rxBuffer[RX_BUFFER_SIZE];
void recvCallback(uint8_t *data, size_t len){
  for(int i = 0; i < len; i++){
    rxBuffer[i] = data[i];
  }
  Serial.println("received SPI from Master");
  newDataReceived = true;
}

int8_t txBuffer[TX_BUFFER_SIZE];
size_t txSize = 1;
void sentCallback(){
  SPISlave1.setData((uint8_t*)txBuffer, txSize);
}


void setup1() {

  SPISlave1.setRX(MOSI);
  Serial.println("RX");
  SPISlave1.setTX(MISO);
  Serial.println("TX");
  SPISlave1.setCS(SPI_SLAVE_CS);
  Serial.println("CS");
  SPISlave1.setSCK(SCK);
  Serial.println("Set Pins");

  sentCallback();
  Serial.println("Called Back");

  SPISlave1.onDataRecv(recvCallback);
  SPISlave1.onDataSent(sentCallback);
  Serial.println("Assigned callbacks");
  SPISlave1.begin(spisettings);


  delay(3000);
  Serial.println("S-INFO: SPISlave1 started");
}

void loop1() {
  if(newDataReceived){
    Serial.print("Master Sent:");
    Serial.println((char*) rxBuffer);
    newDataReceived = false;
  }
}
