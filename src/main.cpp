#include <Arduino.h>
#include <Wire.h>
#include "TeensyTimerTool.h"
#include "communication.h"
#include "memory.h"
#include "supercap.h"

using namespace TeensyTimerTool;

typedef enum CAP_STATE {
  CAP_STATE_IDLE,
  CAP_STATE_CHARGE,
  CAP_STATE_DISCHARGE,
} cap_state_t;


//global variables
static volatile int cap_state;
static int ticker_attached = 0;
static int current_state = CAP_STATE_IDLE;
static int prev_state = CAP_STATE_DISCHARGE;


SuperCap SuperCap1(&Wire, 0x55);
OneShotTimer StateTicker(TCK);

const int ledPin = 13;
SerialHandler serialHandler;

// void heartbeat() {
//     static byte heartState = 1;
// 	static unsigned long previousBeat = millis();
// 	unsigned long currentMillis = millis();
// 	if (currentMillis - previousBeat >= 500) {
// 		previousBeat = currentMillis;
// 		heartState ^= 1;
// 		digitalWrite(ledPin, heartState);
// 	}
// }

//copy one array to another
void copyArr(uint8_t *array_from, uint8_t *array_to, size_t size) {
    while (size--) {
      *array_to++ = *array_from++;
    }
}


// uint8_t readRXBuffer(uint8_t *data_ptr, uint8_t len) {
//   uint8_t data_counter = 0;
//   while (Serial.available() || data_counter < len) {
//     *data_ptr = Serial.read();
//     data_ptr++;
//     data_counter++;
//   }

//   return data_counter;
// }

void changeState(int next_state) {
  ticker_attached = 0;
  current_state = next_state;
}

// state daemon of the capacitor
void stateDaemon(){
    static unsigned long previous_state_time = millis();
    switch (cap_state) {
        case CAP_STATE_IDLE:
            SuperCap1.setIdle();
            SuperCap1.setCurrent(0);
            unsigned long current_time = millis();
            if (abs(current_time - previous_state_time) >= 5) {
                prev_state = CAP_STATE_IDLE;
                if (prev_state == CAP_STATE_DISCHARGE) {
                    Serial.println("Changing to Charge");
                    changeState(CAP_STATE_CHARGE);
                }
                else {
                    Serial.println("Changing to Discharge");
                    changeState(CAP_STATE_DISCHARGE);
                }
            }
            break;

        case CAP_STATE_CHARGE:
            SuperCap1.setIdle();
            SuperCap1.setCurrent(3);
            unsigned long current_time = millis();
            if (abs(current_time - previous_state_time) >= 2) {
                Serial.println("Changing to IDLE from CHARGE");
                prev_state = CAP_STATE_CHARGE;
                changeState(CAP_STATE_IDLE);
            }
            break;

        case CAP_STATE_DISCHARGE:
            SuperCap1.setIdle();
            SuperCap1.setCurrent(-3);
            unsigned long current_time = millis();
            if (abs(current_time - previous_state_time) >= 2) {
                Serial.println("Changing to IDLE form DIS");
                prev_state = CAP_STATE_DISCHARGE;
                changeState(CAP_STATE_IDLE);
            }
            break;
        
  }
}
void setup() {
  pinMode(ledPin, OUTPUT);
  serialHandler.begin();
}

void loop() {
  //heartbeat();
  serialHandler.handler();
}

//
//    FILE: CRC16_test.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo
//    DATE: 2021-01-20
//    (c) : MIT


// #include "CRC16.h"
// #include "CRC.h"

// char str[9] = {0x12,0x43,0x12,0x67,0xFF,0x14,0x46,0x78,0x44};

// CRC16 crc;

// crc161(char const *arg_pData, char arg_size)
// {
//     int i, crc = 0;
//     for (; arg_size>0; arg_size--)         /* Step through bytes in memory */
//     {
//         crc = crc ^ (*arg_pData++ << 8);   /* Fetch byte from memory, XOR into CRC top byte*/
//         for (i=0; i<8; i++)                /* Prepare to rotate 8 bits */
//         {
//             crc = crc << 1;                /* rotate */
//             if (crc & 0x10000)             /* bit 15 was set (now bit 16)... */
//             crc = (crc ^ 0x1021) & 0xFFFF; /* XOR with XMODEM polynomic */
//                                            /* and ensure CRC remains 16-bit value */
//         }                                  /* Loop for 8 bits */
//     }                                      /* Loop until num=0 */
//     return(crc);                           /* Return updated CRC */
// }

// void test()
// {
//    Serial.println(crc161((uint8_t *) str, 9), HEX);
//   Serial.println(crc16((uint8_t *) str, 9, 0x1021, 0, 0, false, false), HEX);
  
//   crc.setPolynome(0x1021);
//   crc.add((uint8_t*)str, 9);
//   Serial.println(crc.getCRC(), HEX);

//   crc.reset();
//   crc.setPolynome(0x1021);
//   for (int i = 0; i < 9; i++)
//   {
//     crc.add(str[i]);
//     Serial.print(i);
//     Serial.print("\t");
//     Serial.println(crc.getCRC(), HEX);
//   }

//   crc.restart();
//   for (int i = 0; i < 9; i++)
//   {
//     crc.add(str[i]);
//   }
//   Serial.println(crc.getCRC(), HEX);
//   for (int i = 0; i < 9; i++)
//   {
//     crc.add(str[i]);
//   }
//   Serial.println(crc.getCRC(), HEX);
//   Serial.println(crc.count());
// }


// // -- END OF FILE --

// void setup()
// {
//   Serial.begin(115200);
//   Serial.println(__FILE__);

//   // Serial.println("Verified with - https://crccalc.com/\n");

//   test();
// }


// void loop()
// {
// }
