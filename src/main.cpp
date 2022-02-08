#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <CircularBuffer.h>
#include "TeensyTimerTool.h"
#include "communication.h"
#include "memory.h"
#include "supercap.h"

using namespace TeensyTimerTool;

#define DATA_COLLECT_INTERTVAL 62 //ms

typedef enum CAP_STATE {
  CAP_STATE_IDLE,
  CAP_STATE_CHARGE,
  CAP_STATE_DISCHARGE,
} cap_state_t;


//global variables
byte dataPointCounter = 0;
CircularBuffer<ExprimentBuffer,4> transmitBuffer; // contain 4 payload, each for a supercap waiting to be transmitted
CircularBuffer<ExprimentBuffer,80> transmitQueue; // contain 2 experiment data packet, each has 4 payload waiting to copied to the transmit buffer

SuperCap superCaps[] = {SuperCap(4,18,A0),SuperCap(5,17,A1),SuperCap(6,16,A2),SuperCap(7,14,A3)};
uint8_t superCapAddrs[] = {0x40,0x41,0x44,0x45}; //addresses or INA260s
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
					expBuffers[i].fields.superCapNo = i+1;
					expBuffers[i].fields.exprimentNo[0] = (experimentCounter >> 8) & 0xFF;
					expBuffers[i].fields.exprimentNo[1] = experimentCounter & 0xFF;
					int temp = 10; //random(65000);
					expBuffers[i].fields.startTemp[0] = (temp >> 8) & 0xFF; // record start temperature
					expBuffers[i].fields.startTemp[1] = temp & 0xFF;
				}
			}
			
			// record time, voltage and current
			for (byte i=0; i<4; i++) {
				int voltage,current;
				voltage = 4;//random(65000);
				current = 5;//random(65000);
				unsigned int deltaTime = abs(millis() - startTime);
				expBuffers[i].fields.chargeData[dataPointCounter-1].time[0] = (deltaTime >> 8) & 0xFF; // remember to handle this
				expBuffers[i].fields.chargeData[dataPointCounter-1].time[1] = deltaTime & 0xFF;
				expBuffers[i].fields.chargeData[dataPointCounter-1].voltage[0] = (voltage >> 8) & 0xFF; // remember to handle this
				expBuffers[i].fields.chargeData[dataPointCounter-1].voltage[1] = voltage & 0xFF;
				expBuffers[i].fields.chargeData[dataPointCounter-1].current[0] = (current >> 8) & 0xFF; // remember to handle this
				expBuffers[i].fields.chargeData[dataPointCounter-1].current[1] = current & 0xFF;
			}

			// reset counter and change state once have more than 8 points
        	if (dataPointCounter > 7) 
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
				for (byte i=0; i<4; i++) 
				{
					expBuffers[i].fields.midTemp = 22; // record start temperature
				}
			}
			
			// record time, voltage and current
			for (byte i=0; i<4; i++) {
				int voltage,current;
				voltage = 0;//random(65000);
				current = 0;//random(65000);
				unsigned int deltaTime = abs(millis() - startTime);
				expBuffers[i].fields.dischargeData[dataPointCounter-1].time[0] = (deltaTime >> 8) & 0xFF; // remember to handle this
				expBuffers[i].fields.dischargeData[dataPointCounter-1].time[1] = deltaTime & 0xFF;
				expBuffers[i].fields.dischargeData[dataPointCounter-1].voltage[0] = (voltage >> 8) & 0xFF; // remember to handle this
				expBuffers[i].fields.dischargeData[dataPointCounter-1].voltage[1] = voltage & 0xFF;
				expBuffers[i].fields.dischargeData[dataPointCounter-1].current[0] = (current >> 8) & 0xFF; // remember to handle this
				expBuffers[i].fields.dischargeData[dataPointCounter-1].current[1] = current & 0xFF;
			}

			// reset counter and change state once have more than 8 points
        	if (dataPointCounter > 7) 
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
        	}	
			break;
		}
  }
}

void setup() {
  	obcUART.begin();

	//setup super caps
  	for (uint8_t i=0; i<4; i++) {
	  	superCaps[i].init();
	  	superCaps[i].begin(superCapAddrs[i],&Wire1);
  	}
  	randomSeed(analogRead(0));
}

void loop() {
	static unsigned long previousBeat = millis();
	unsigned long currentMillis = millis();
	unsigned int deltaTime = abs(currentMillis - previousBeat);
	if (deltaTime >= DATA_COLLECT_INTERTVAL) {
		previousBeat = currentMillis;
    	stateDaemon();
	}

	//check if new message has been received
	UartBuffer rxData;
	memset(rxData.data, 0, sizeof(rxData.data));
	int16_t len = obcUART.receive(rxData.data);
	if (len > -1) 
	{
		if (rxData.fields.transferResponseCode == MASTER_REQUEST_DATA) //obc is requesting data
		{
			ExprimentBuffer payloadBuffer;
			memset(payloadBuffer.data, 0, sizeof(payloadBuffer.data));
			// check if the transmit buffer is empty and if there is data 
			// waiting to be loaded from the queue
			if (transmitBuffer.isEmpty() && (transmitQueue.size() >= 3)) 
			{
				// load data for 4 capacitors
				Serial.println("Loading more...");
				for (uint8_t i=0; i<4; i++) 
				{
					transmitBuffer.push(transmitQueue.shift());
				}	
				payloadBuffer = transmitBuffer.shift();
			} 
			// if not empty the pop a payload
			else 
			{
				payloadBuffer = transmitBuffer.shift();
			}
			
			// finally send it away, if no data is available, send all 0
			obcUART.transmit(payloadBuffer.data,EXP_BUFFER_LEN);
		}
	}
}

