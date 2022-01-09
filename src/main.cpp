#include <Arduino.h>
#include <Wire.h>
#include "communication.h"
#include "memory.h"
#include "supercap.h"

#define BUFFER_SIZE 128
#define START_FLAG_MSB  0x19
#define START_FLAG_LSB  0x94

typedef enum TRANSFER_RESPONSE_CODE{
  MASTER_REQUEST_DATA,
  MASTER_ISSUING_DATA,
  SLAVE_ACK,
} tranfer_resposnse_code_t;

const int ledPin = 13;

void heartbeat() {
    static byte heartState = 1;
	static unsigned long previousBeat = millis();
	unsigned long currentMillis = millis();
	if (currentMillis - previousBeat >= 1000) {
		previousBeat = currentMillis;
		heartState ^= 1;
		digitalWrite(ledPin, heartState);
	}
}

//copy one array to another
void copyArr(uint8_t *array_from, uint8_t *array_to, size_t size) {
    while (size--) {
      *array_to++ = *array_from++;
    }
}

// crc16 ccitt 0x1021 checksum
uint16_t crc16(uint8_t const *data, size_t size) {
    uint16_t crc = 0;     
    while (size--) {         
        crc ^= *data++;         
        for (uint8_t k = 0; k < 8; k++)             
        crc = crc & 1 ? (crc >> 1) ^ 0x8408 : crc >> 1;     
    }     
    return crc; 
}

uint8_t readRXBuffer(uint8_t *data_ptr, uint8_t len) {
  uint8_t data_counter = 0;
  while (Serial.available() || data_counter < len) {
    *data_ptr = Serial.read();
    data_ptr++;
    data_counter++;
  }

  return data_counter;
}

// send data here
void serial_TX() {
  char tx_data[128] = {0};
  randomSeed(analogRead(0));
  char rand_len = random(10,124);

  tx_data[0] = 0x19;
  tx_data[1] = 0x94;

  tx_data[3] = rand_len; //packet len
  tx_data[5] = 0x08;
  tx_data[6] = 0x86;
  tx_data[10] = 0x02;

  //generate random data
  for (unsigned i=0; i<rand_len-10; i++) {
      tx_data[11+i] = random(0,255);
  }

  //check sum
  uint16_t checksum = crc16(&tx_data[2],rand_len+1);
  tx_data[rand_len+2] = checksum >> 8;
  tx_data[rand_len+3] = checksum & 0xFF;

  Serial.write("Data transmitting: ");
  Serial.write(tx_data,rand_len+1);
}
void setup() {
  pinMode(ledPin, OUTPUT);

  Serial.begin(4800); //Baudrate set by Skykraft
  Serial.setTimeout(1);
  
}

void loop() {
  heartbeat();
  
  // receive package from OBC, decide what to do next based on this
  if (Serial.available() > 0) {
    char rx_bytes[128] = {0};
    char rx_packet[128] = {0};
    byte start_idx = 0;
    byte packet_len = 0;
    delay(20);

    // check start flag
    int len = Serial.readBytes(&rx_bytes[0],BUFFER_SIZE);
    for (int i=0;i<len-1;i++){
      if (rx_bytes[i] == START_FLAG_MSB && rx_bytes[i+1] == START_FLAG_LSB) {
        start_idx = i;
        packet_len = rx_bytes[i+3];
        Serial.println("Hooray");
        Serial.println(start_idx);
        break;
      }
    }

    // act accordiningly to the transfer/response code
    switch (rx_bytes[start_idx+10]) {
      case MASTER_REQUEST_DATA:
        // send data here
        serial_TX();
        break;
      case MASTER_ISSUING_DATA:
        // receive data
        // then send ack
        byte TX_data[128] = {1}; 
        Serial.write(TX_data,5);
        break;
    }

    // will need to implement a state machine
    Serial.print("Packet received: bytes ");
    Serial.println(len);
    for(int i = 0; i < len; i++) {
      Serial.print(rx_bytes[i],HEX);
      Serial.println("");
    }
  }
}