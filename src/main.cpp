#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <CircularBuffer.h>
#include "TeensyTimerTool.h"
#include "communication.h"
#include "memory.h"
#include "supercap.h"

using namespace TeensyTimerTool;

#define DATA_COLLECT_INTERTVAL 10 //ms

typedef enum CAP_STATE {
  CAP_STATE_IDLE,
  CAP_STATE_CHARGE,
  CAP_STATE_DISCHARGE,
} cap_state_t;


//global variables
byte dataPointCounter = 0;
CircularBuffer<ExprimentBuffer,4> transmitBuffer; // contain 4 payload, each for a supercap waiting to be transmitted
CircularBuffer<ExprimentBuffer,8> transmitQueue; // contain 2 experiment data packet, each has 4 payload waiting to copied to the transmit buffer


//SuperCap superCaps[] = {SuperCap(1,2),SuperCap(1,2),SuperCap(1,2),SuperCap(1,2)};

const int ledPin = 13;
UART obcUART;

// state daemon of the capacitor
void stateDaemon()
{
	static int experimentCounter = 0;
	static unsigned long startTime = 0;
	static byte currentState = CAP_STATE_CHARGE;
	static byte previousState = CAP_STATE_IDLE;
	static ExprimentBuffer expBuffers[4];

    switch (currentState) {
        case CAP_STATE_IDLE: //rest for 62 ms
		{
			dataPointCounter++;
        	if (dataPointCounter > 1) // reset the counter and change state
			{
          		dataPointCounter = 0;
				if (previousState == CAP_STATE_DISCHARGE) //switch between discharge and charge
				{
					currentState = CAP_STATE_CHARGE;
					//Serial.println("Changing State... Next State: CHARGE");
				} 
				else 
				{
					currentState = CAP_STATE_DISCHARGE;
					//Serial.println("Changing State... Next State: DISCHARGE");
				}
				previousState = CAP_STATE_IDLE;
        	}	
			break;
		}
                    

        case CAP_STATE_CHARGE:
		{
			dataPointCounter++;
			if (dataPointCounter == 1) 
			{
				startTime = millis(); // record start time
				experimentCounter++;
				for (byte i=0; i<4; i++) {
					expBuffers[i].fields.exprimentNo[0] = (experimentCounter >> 8) & 0xFF;
					expBuffers[i].fields.exprimentNo[1] = experimentCounter & 0xFF;
					expBuffers[i].fields.startTemp[0] = (20 >> 8) & 0xFF; // record start temperature
					expBuffers[i].fields.startTemp[1] = 20 & 0xFF;
				}
			}
			
			// record time, voltage and current
			for (byte i=0; i<4; i++) {
				unsigned int deltaTime = abs(millis() - startTime);
				expBuffers[i].fields.chargeData[dataPointCounter-1].time[0] = (deltaTime >> 8) & 0xFF; // remember to handle this
				expBuffers[i].fields.chargeData[dataPointCounter-1].time[1] = deltaTime & 0xFF;
				expBuffers[i].fields.chargeData[dataPointCounter-1].voltage[0] = (12 >> 8) & 0xFF; // remember to handle this
				expBuffers[i].fields.chargeData[dataPointCounter-1].voltage[1] = 12 & 0xFF;
				expBuffers[i].fields.chargeData[dataPointCounter-1].current[0] = (43 >> 8) & 0xFF; // remember to handle this
				expBuffers[i].fields.chargeData[dataPointCounter-1].current[1] = 43 & 0xFF;
			}

			// reset counter and change state once have more than 8 points
        	if (dataPointCounter > 8) 
			{
          		dataPointCounter = 0;
				previousState = CAP_STATE_CHARGE;
				currentState = CAP_STATE_IDLE;
        	}	
			break;
		}

        case CAP_STATE_DISCHARGE:
 		{
			dataPointCounter++;
			if (dataPointCounter == 1) 
			{
				for (byte i=0; i<4; i++) {
					expBuffers[i].fields.midTemp = 22; // record start temperature
				}
			}
			
			// record time, voltage and current
			for (byte i=0; i<4; i++) {
				unsigned int deltaTime = abs(millis() - startTime);
				expBuffers[i].fields.dischargeData[dataPointCounter-1].time[0] = (deltaTime >> 8) & 0xFF; // remember to handle this
				expBuffers[i].fields.dischargeData[dataPointCounter-1].time[1] = deltaTime & 0xFF;
				expBuffers[i].fields.dischargeData[dataPointCounter-1].voltage[0] = (11 >> 8) & 0xFF; // remember to handle this
				expBuffers[i].fields.dischargeData[dataPointCounter-1].voltage[1] = 11 & 0xFF;
				expBuffers[i].fields.dischargeData[dataPointCounter-1].current[0] = (23 >> 8) & 0xFF; // remember to handle this
				expBuffers[i].fields.dischargeData[dataPointCounter-1].current[1] = 23 & 0xFF;
			}

			// reset counter and change state once have more than 8 points
        	if (dataPointCounter > 8) 
			{
				for (byte i=0; i<4; i++) {
					expBuffers[i].fields.endTemp = 22; // record start temperature
				}
          		dataPointCounter = 0;
				previousState = CAP_STATE_DISCHARGE;
				currentState = CAP_STATE_IDLE;
				// push into the transmit queue
				for (byte i=0; i<4; i++) {
					transmitQueue.push(expBuffers[i]);
				}
				// uint8_t test[104];
				// for (byte i=0; i<104; i++) {
				// 	test[i] = transmitQueue[0].data[i];
				// }
				// obcUART.transmit(test,104);
        	}	
			break;
		}
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  obcUART.begin();

  //SuperCap1.readCurrent();
  //SuperCap1.begin(0x88,&Wire1);
}

void loop() {
	static unsigned long previousBeat = millis();
	unsigned long currentMillis = millis();
	unsigned int deltaTime = abs(currentMillis - previousBeat);
	if (deltaTime >= DATA_COLLECT_INTERTVAL) {
		previousBeat = currentMillis;
    	stateDaemon();
	}
}

