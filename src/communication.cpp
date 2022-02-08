#include "communication.h"

//------------------------------------------------------------------------------------------------
UART::UART(): m_rxState(FIRST_FLAG_FIND), m_rxPacketLen(0), m_rxPacketCount(0){}

//------------------------------------------------------------------------------------------------
void UART::begin()
{
    // PC Communication
    Serial.begin(9600);

    // OBC Communication
    Serial1.begin(9600); //Baudrate set by Skykraft
    Serial1.setTimeout(1);
}

//------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------
int16_t UART::receive(uint8_t *arg_pPacket)
{
    int16_t len = -1;

    if(Serial1.available()) // check if new byte is available/waiting to be read
    {
        // read in byte an advance state until the whole
        // packaged is found
        rxByte = Serial1.read();
        switch (m_rxState)
        {
        case FIRST_FLAG_FIND:
            if (rxByte == 0x19) {
                m_rxState++;
            }
            break;

        case SECOND_FLAG_FIND:
            if (rxByte == 0x94) {
                m_rxState++;
                memset(rxBuffer.data, 0, sizeof(rxBuffer.data));
            }
            break;

        case MSB_PACKET_LEN_FIND:
            m_rxPacketLen |= (rxByte << 8);
            rxBuffer.data[2] = rxByte;
            m_rxState++;
            break;

        case LSB_PACKET_LEN_FIND:
            m_rxPacketLen |= rxByte;
            rxBuffer.data[3] = rxByte;
            m_rxState++;
            break;
        
        case PACKET_PROCESS:
        {
            rxBuffer.data[0] = 0x19;
            rxBuffer.data[1] = 0x94;
            m_rxPacketCount++;
            
            if (m_rxPacketCount < m_rxPacketLen) 
            {
                rxBuffer.data[3+m_rxPacketCount] = rxByte;
                break;
            }
            else
            {
                rxBuffer.data[3+m_rxPacketCount] = rxByte;
                m_rxState++;

            } 

            // check for crc
            uint16_t rxChecksum = 0;
            uint16_t calculatedChecksum = crc16(&rxBuffer.data[4],m_rxPacketLen-2);
            rxChecksum = rxBuffer.data[m_rxPacketLen + 2] << 8;
            rxChecksum |= rxBuffer.data[m_rxPacketLen + 3];

            // compare checksum, if pass => indicating that a completed package has been received
            if (rxChecksum == calculatedChecksum) 
            {
                arg_pPacket[0] = 0x19;
                arg_pPacket[1] = 0x94;
                Serial.print("Receive: ");
                for (int i = 0; i < (m_rxPacketLen - 9); i++) 
                {
                    Serial.print(rxBuffer.data[i + 11],HEX);
                }
                Serial.println("");
                len = m_rxPacketLen; 
            }
            else 
            {
                Serial.print("CRCs do not match: ");
                Serial.print(rxChecksum, HEX);
                Serial.print(" v.s. ");
                Serial.println(calculatedChecksum, HEX);
            }

            m_rxState = 0;
            m_rxPacketCount = 0;
            m_rxPacketLen = 0;
            break;
        }
        default:
            m_rxState = 0;
            m_rxPacketCount = 0;
            m_rxPacketLen = 0;
            break;
        }
    }

    return len;
}