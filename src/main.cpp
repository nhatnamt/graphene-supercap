#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
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


SuperCap SuperCap1(4,15);
SuperCap SuperCap2();
SuperCap SuperCap3();
SuperCap SuperCap4();
OneShotTimer StateTicker(TCK);

const int ledPin = 13;
UART obcUART;

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


void changeState(int next_state) {
  ticker_attached = 0;
  current_state = next_state;
}

// state daemon of the capacitor
void stateDaemon(){
    static unsigned long previous_state_time = millis();
    unsigned long current_time;
    switch (cap_state) {
        case CAP_STATE_IDLE:
            //SuperCap1.setIdle();
            //SuperCap1.setCurrent(0);
            current_time = millis();
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
            //SuperCap1.setIdle();
            //SuperCap1.setCurrent(3);
            current_time = millis();
            if (abs(current_time - previous_state_time) >= 2) {
                Serial.println("Changing to IDLE from CHARGE");
                prev_state = CAP_STATE_CHARGE;
                changeState(CAP_STATE_IDLE);
            }
            break;

        case CAP_STATE_DISCHARGE:
            //SuperCap1.setIdle();
            //SuperCap1.setCurrent(-3);
            current_time = millis();
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
  obcUART.begin();

  SuperCap1.readCurrent();
  SuperCap1.begin(0x88,&Wire1);
}

void loop() {
  //heartbeat();
  //serialHandler.handler();
}

