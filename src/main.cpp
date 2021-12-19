#include <Arduino.h>
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

uint8_t readRXBuffer(uint8_t *data_ptr, uint8_t len) {
  uint8_t data_counter = 0;
  while (Serial.available() || data_counter < 0) {
    *data_ptr = Serial.read();
    data_ptr++;
    data_counter++;
  }

  return data_counter;
}

// send data here
void serial_TX() {
  char tx_data[128] = {0};
  tx_data[0] = 0x19;
  tx_data[1] = 0x94;

  tx_data[3] = 0x64; //packet len
  tx_data[5] = 0x08;
  tx_data[6] = 0x86;
  tx_data[10] = 0x02;
}
void setup() {
  pinMode(ledPin, OUTPUT);

  Serial.begin(4800); //Baudrate set by Skykraft
  Serial.setTimeout(1);
  // put your setup code here, to run once:
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
        break;
      case MASTER_ISSUING_DATA:
        // receive data
        // then send ack
        byte TX_data[128] = {1}; 
        Serial.write(TX_data,5);
        break;
    }

    Serial.print("Packet received: bytes ");
    Serial.println(len);
    for(int i = 0; i < len; i++) {
      Serial.print(rx_bytes[i],HEX);
      Serial.println("");
    }
  }
}