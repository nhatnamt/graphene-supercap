#include "communication.h"


SerialHandler::SerialHandler() {


    //set polynomial
    //crc.setPolynome(0x1021);
}

/**
 * @brief Begin Serial Communication
 * 
 */
void SerialHandler::begin()
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
void SerialHandler::serialTx()
{
    char txFrame[128] = {0};    
    txFrame[0]    = 0x19;
    txFrame[1]    = 0x94;

    txFrame[5]    = 0x08;
    txFrame[6]    = 0x86;
    txFrame[10]   = 0x02;

    uint8_t packetLen = m_payloadLen+9;
    txFrame[3] = packetLen;

    //generate random data for now
    for (unsigned i=0; i<m_payloadLen; i++) 
    {
        txFrame[11+i] = random(0,255);
    }

    //calculate checksum
    uint16_t checksum = crc16(&txFrame[4],packetLen-2);
    txFrame[m_payloadLen+11] = checksum >> 8;
    txFrame[m_payloadLen+12] = checksum & 0xFF;

    // Transmitting data
    Serial.write("Data transmitting: ");
    for (int i=0; i<m_payloadLen+13; i++) {
        Serial.print(txFrame[i],HEX);
        Serial.print(" ");
    }
    Serial.println("");
    //Serial.write(txFrame,m_payloadLen+13);
    Serial1.write(txFrame,m_payloadLen+13);

}

/**
 * @brief Calculate CRC16 checksum
 * 
 * @param arg_pData (char)      Array to calculate checksum
 * @param arg_size  (char)      Length of the array
 * @return          (uint16_t)  CRC16 Checksum
 */
uint16_t SerialHandler::crc16(char const *arg_pData, char arg_size)
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

void SerialHandler::setPayload(char arg_payload[115], char arg_len)
{
    for (int i=0; i<arg_len; i++)
    {
      m_payload[i] = arg_payload[i];
    }
    m_payloadLen = arg_len;
}
/**
 * @brief Handle Serial receive data and act accordingly
 * 
 */
void SerialHandler::handler()
{
    //local vars
    char rxFrame[128] = {0};
    byte startIdx = 0, packetLen = 0;
    randomSeed(analogRead(0));
    char rand_len = random(10,124);
    char payload[115] = {0};
    setPayload(payload,rand_len);
    delay(100);
    serialTx();
    digitalWrite(13, 0);
    delay(100);
    digitalWrite(13, 1);
    // if new data is available
    if(Serial1.available() > 0)
    {
        int frameLen = Serial1.readBytes(&rxFrame[0],BUFFER_SIZE);

        // find the start flag
        for (int i=0;i<frameLen-1;i++)
        {
            if (rxFrame[i] == START_FLAG_MSB && rxFrame[i+1] == START_FLAG_LSB)
            {
                startIdx = i;
                packetLen = rxFrame[i+3];
                Serial.print("New packet received: "); Serial.println(startIdx);
                break;
            }
        } 

        //decide what todo
        char transferCode = rxFrame[startIdx+10];
        switch (transferCode)
        {
        case MASTER_REQUEST_DATA:
        {
            randomSeed(analogRead(0));
            char rand_len = random(10,124);
            char payload[115] = {0};
            setPayload(payload,rand_len);
            serialTx();
            break;
        }
        
        case MASTER_ISSUING_DATA:
        {
            Serial.println("Master issued data");
            randomSeed(analogRead(0));
            char rand_len = random(10,124);
            char payload[115] = {0};
            setPayload(payload,rand_len);
            serialTx();
            break;
        }
        }

        //debug
        Serial.print("Packet received: ");
        Serial.println(packetLen);
        for(int i = 0; i < packetLen; i++) 
        {
            Serial.print(rxFrame[i],HEX);
            Serial.println("");
        }
    }

    delay(5); 
}