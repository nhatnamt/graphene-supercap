/*
   Skyride payload OBC emulator

   This file provides an emulator of Skykraft's payload bay on-board computer (OBC). The OBC will handle all
   communications with Skyride payloads. This emulator provides a serial pass-through with representative 
   packet structure and CRC checks. 

   This emulator is intended to be left installed on the provided Teensy and used as a template for your custom
   payload. Feel free to modify the code but please do not distribute it.

   This file has been tested under expected use cases but problems may still arise. Please perform your own debugging
   prior to contacting Skykraft for troubleshooting assistance. If such assistance is still required, contact
   skyride@skykraft.com.au with a detailed description of your problem including any warning/error messages.

   Change Log:
   
   AG 13/10/21
   AG 29/10/21 - modified according to Skyride ICD issue 0.2

   AG 30/11/21

   Changelog:
    * 30/11/21 - CRC calculation no longer incorrectly includes the undefined CRC bytes at the end of the packet.
*/
#include "Arduino.h"
#include "CRC16.h"
#include "CRC.h"
// standard buffer size for Skykraft packet
#define BUFFER_SIZE 128

CRC16 crc;

// default packet fields
typedef struct packet_fields {
  // start flags
  uint8_t start_flags[2];
  // packet length
  uint8_t packet_length[2];
  // protocol (reserved)
  uint8_t protocol;
  // device ID
  uint8_t device_id[2];
  // message ID (reserved)
  uint8_t message_id[3];
  // transfer/response code
  uint8_t trans_resp_code;
  // payload and crc
  uint8_t payload[BUFFER_SIZE - 11];
} packet_fields;

// creating this union makes it easier to copy tx/rx data into the buffer
typedef union uart_buffer {
  packet_fields fields;
  uint8_t data[BUFFER_SIZE];
} uart_buffer;

// initialise tx and rx buffers for UART communications
uart_buffer tx_buffer;
uart_buffer rx_buffer;
// initialise array for contents of rx buffer payload
uint8_t rx_payload[BUFFER_SIZE - 11];
// initialise pin number for controlling the Teensy's on-board LED
int led = 13;

void setup() {
  // start serial interface to USB port
  Serial.begin(9600);
  // start serial interface to hardware UART (pins 0-1)
  Serial1.begin(9600);
  // set pin mode for LED
  pinMode(led, OUTPUT);
  // set polynomial for CRC
  crc.setPolynome(0x1021);
}

void send_tx_packet(uint8_t * data_for_payload, int data_len) {
  // start by clearing the payload section of the buffer
  memset(tx_buffer.fields.payload, 0, sizeof(tx_buffer.fields.payload));
  // put in the start flags
  tx_buffer.fields.start_flags[0] = 0x19;
  tx_buffer.fields.start_flags[1] = 0x94;
  // calculate total packet length (data + 7 after packet length in preamble + 2 for CRC)
  int packet_len = data_len + 9;
  // put in the length, which is Most Significant Byte (MSB) first
  tx_buffer.fields.packet_length[0] = (packet_len >> 8) & 0xFF;
  tx_buffer.fields.packet_length[1] = packet_len & 0xFF;
  // transfer/respond code
  tx_buffer.fields.trans_resp_code = 2;
  // now put in the data
  for (int i = 0; i < data_len; i++) {
    tx_buffer.fields.payload[i] = data_for_payload[i];
  }
  // calculate the crc over all fields but the start flag
  crc.reset();
  crc.setPolynome(0x1021);
  for (int i = 0; i < (packet_len - 2); i++) {
    crc.add(tx_buffer.data[4 + i]);
  }
  tx_buffer.fields.payload[data_len] = (crc.getCRC() >> 8) & 0xFF;
  tx_buffer.fields.payload[data_len + 1] = crc.getCRC() & 0xFF;
  // and send the whole buffer
  Serial.println("Sending...");
  for (int i = 0; i < (packet_len + 4); i++) {
    Serial1.write(tx_buffer.data[i]);
    Serial.print(tx_buffer.data[i],HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
}

uint8_t payload_data[BUFFER_SIZE - 11];
uint8_t rx_state;
uint16_t packet_length;
uint16_t rx_num;

void loop() {
  char c;
  uint16_t calc_crc;
  uint16_t rx_crc;
  int j = 0;
  // if something comes over USB, read it all then send it over UART
  if (Serial.available()) {
    // clear the payload_data buffer to make sure no previous messages are remaining
    memset(payload_data, 0, sizeof(payload_data));
    // start a counter and read all
    while (Serial.available()) {
      payload_data[j] = Serial.read();
      j++;
    }
    send_tx_packet(payload_data, j);
  }
  // if something comes over the hardware UART
  if (Serial1.available()) {
    c = Serial1.read();
    switch (rx_state) {
      // looking for first start flag
      case 0:
        if (c == 0x19) {
          rx_state++;
        }
        break;
      // looking for second start flag
      case 1:
        if (c == 0x94) {
          rx_state++;
          // since a new packet is found,
          // clear the rx buffer to make sure no previous messages are remaining
          memset(rx_buffer.data, 0, sizeof(rx_buffer.data));
        }
        break;
      // looking for msb of packet length
      case 2:
        packet_length |= (c << 8);
        rx_buffer.data[2] = c;
        rx_state++;
        break;
      // looking for lsb of packet length
      case 3:
        packet_length |= c;
        rx_buffer.data[3] = c;
        rx_state++;
        break;
      // reading the rest of the packet
      case 4:
        rx_num++;
        if (rx_num < packet_length) {
          rx_buffer.data[3 + rx_num] = c;
          break;
        }
        else {
          rx_buffer.data[3 + rx_num] = c;
          rx_state++;
        }
        // checking the crc
        crc.reset();
        crc.setPolynome(0x1021);
        for (int i = 0; i < (packet_length - 2); i++) {
          crc.add(rx_buffer.data[4 + i]);
        }

        calc_crc = crc.getCRC();
        rx_crc = 0;
        rx_crc |= rx_buffer.data[packet_length + 2] << 8;
        rx_crc |= rx_buffer.data[packet_length + 3];
        if (rx_crc != calc_crc) {
          Serial.print("CRCs do not match: ");
          Serial.print(rx_crc, HEX);
          Serial.print(" v.s. ");
          Serial.println(calc_crc, HEX);
          rx_state = 0;
          rx_num = 0;
          packet_length = 0;
          break;
        }
        // if it passes, print the received data
        Serial.print("Received: ");
        for (int i = 0; i < (packet_length + 4); i++) {
          Serial.print(rx_buffer.data[i],HEX);
        }
        Serial.println("");
        // reset variables
        rx_state = 0;
        rx_num = 0;
        packet_length = 0;
        break;
      default:
        rx_state = 0;
        rx_num = 0;
        packet_length = 0;
        break;
    }
  }
}