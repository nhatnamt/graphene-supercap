#include "communication.h"


UART::UART(): rxState(FIRST_FLAG_FIND), rxPacketLength(0), rxPacketIndex(0){

    //set polynomial
    //crc.setPolynome(0x1021);
}

/**
 * @brief Begin Serial Communication
 * 
 */
void UART::begin()
{
    // PC Communication
    Serial.begin(9600);

    // OBC Communication
    Serial1.begin(9600); //Baudrate set by Skykraft
    Serial1.setTimeout(1);
}
    
/**
 * @brief Transmit Serial data in the correct format
 * 
 */
void UART::transmit(uint8_t *arg_pPayload, uint8_t arg_payloadLen)
{
    UartBuffer txBuffer;
    uint8_t packetLen = arg_payloadLen + 9;

    memset(txBuffer.data, 0, sizeof(txBuffer.data));            // clearing the txBuffer

    txBuffer.fields.startFlags[0] = 0x19;                       // start flag MSB & LSB
    txBuffer.fields.startFlags[1] = 0x94;
    txBuffer.fields.packetLength[0] = (packetLen >> 8) & 0xFF;  // packet length MSB & LSB
    txBuffer.fields.packetLength[1] = packetLen & 0xFF;
    txBuffer.fields.deviceId[0] = 0x08;                         // device ID 
    txBuffer.fields.deviceId[1] = 0x86;                         // subsys ID
    txBuffer.fields.transferResponseCode = SLAVE_ACK;

    // copying the payload to txBuffer
    for (int i = 0; i < arg_payloadLen; i++) {
        txBuffer.fields.payload[i] = arg_pPayload[i];
    }

    // calculate checksum
    uint16_t checksum = crc16(&txBuffer.data[4],packetLen-2);
    txBuffer.fields.payload[arg_payloadLen] = (checksum >> 8) & 0xFF;
    txBuffer.fields.payload[arg_payloadLen+1] = checksum & 0xFF;

    // transmitting data
    Serial.write("Data transmitting: ");
    for (int i=0; i<packetLen+4; i++) {
        //send to UARt Device
        Serial1.write(txBuffer.data[i]);
        //send to PC 
        Serial.print(txBuffer.data[i],HEX);
        Serial.print(" ");
    }
    Serial.println(" ");
}

/**
 * @brief Calculate CRC16 checksum
 * 
 * @param arg_pData (char)      Array to calculate checksum
 * @param arg_size  (char)      Length of the array
 * @return          (uint16_t)  CRC16 Checksum
 */
uint16_t UART::crc16(uint8_t const *arg_pData, uint8_t arg_size)
{
    int i, crc = 0;
    for (; arg_size>0; arg_size--)         /* Step through bytes in memory */
    {
        crc = crc ^ (*arg_pData++ << 8);   /* Fetch byte from memory, XOR into CRC top byte*/
        for (i=0; i<8; i++)                /* Prepare to rotate 8 bits */
        {
            crc = crc << 1;                /* rotate */
            if (crc & 0x10000)             /* bit 15 was set (now bit 16)... */
            crc = (crc ^ 0x1021) & 0xFFFF; /* XOR with XMODEM polynomic */
                                           /* and ensure CRC remains 16-bit value */
        }                                  /* Loop for 8 bits */
    }                                      /* Loop until num=0 */
    return(crc);                           /* Return updated CRC */
}

/**
 * @brief Handle Serial receive data and act accordingly
 * 
 */
int8_t UART::receive(uint8_t *arg_pPacket)
{
    UartBuffer rxBuffer;
    if(Serial1.available())
    {
        rxByte = Serial1.read();
        switch (rxState)
        {
        case FIRST_FLAG_FIND:
            if (rxByte == 0x19) rxState++;
            break;

        case SECOND_FLAG_FIND:
            if (rxByte == 0x94) rxState++;
            memset(rxBuffer.data, 0, sizeof(rxBuffer.data));
            break;

        case MSB_PACKET_LEN_FIND:
            rxPacketLen = rxByte << 8;
            rxBuffer.data[2] = rxByte;
            rxState++;
            break;

        case LSB_PACKET_LEN_FIND:
            rxPacketLen |= rxByte &0xFF;
            rxBuffer.data[3] = rxByte;
            rxState++;
            break;
        
        case PACKET_PROCESS:
            rxPacketCount++;
            if (rxPacketCount < rxPacketLen) 
            {
                rxBuffer.data[3+rxPacketCount] = rxByte;
                break;
            }
            else
            {
                rxBuffer.data[3+rxPacketCount] = rxByte;
                rxState++;

            } 
            // check for crc
            uint16_t rxChecksum = 0;
            uint16_t calculatedChecksum = crc16(&rxBuffer.data[4],rxPacketLen-2);
            rxChecksum = rxBuffer.data[rxPacketLen + 2] << 8;
            rxChecksum |= rxBuffer.data[rxPacketLen + 3];
            if (rxChecksum != calculatedChecksum) 
            {
                Serial.println("CRC Failed");
                rxState = 0;
                rxPacketCount = 0;
                rxPacketLen = 0;
                return -1;
            }
            
            Serial.print("Received: ");
            for (int i = 0; i < (rxPacketLen - 9); i++) 
            {
                Serial.print((char)rxBuffer.data[i + 11]);
            }
            Serial.println("");

            rxState = 0;
            rxPacketCount = 0;
            rxPacketLen = 0;
            break;
        default:
            rxState = 0;
            rxPacketCount = 0;
            rxPacketLen = 0;
            break;
        }
    }
}