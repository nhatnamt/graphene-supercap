/*!
 *  @file communication.h
 *
 *  Serial/UART handler, responible for transceiving, packing and unpacking serial data.
 *  Based on Skykraft protocal and example
 *
 */

#include <Arduino.h>

#define BUFFER_SIZE 128 //buffer size of the i/o package

enum TransferResponseCode{
	MASTER_REQUEST_DATA,
  	MASTER_ISSUING_DATA,
  	SLAVE_ACK,
};

enum m_rxState {
    FIRST_FLAG_FIND,
    SECOND_FLAG_FIND,
    MSB_PACKET_LEN_FIND,
    LSB_PACKET_LEN_FIND,
    PACKET_PROCESS,
};

// default packet fields
struct PacketFields {
    uint8_t startFlags[2];                      // start flags
    uint8_t packetLength[2];                    // packet length
    uint8_t protocol;                           // protocol (reserved)
    uint8_t deviceId[2];                        // device ID
    uint8_t messageId[3];                       // message ID (reserved)
    TransferResponseCode transferResponseCode;  // transfer/response code
    uint8_t payload[BUFFER_SIZE - 11];          // payload and crc
};

// creating this union makes it easier to copy tx/rx data into the buffer
union UartBuffer {
  	PacketFields fields;
  	uint8_t data[BUFFER_SIZE];
};

/**
 * @brief Class that handle UART i/o packages
 * 
 */
class UART {
private:
    char rxByte;
    uint8_t m_rxState;
    uint16_t m_rxPacketLen, m_rxPacketCount;

	UartBuffer rxBuffer;

	/**
	* @brief Calculate CRC16-XMODEM checksum with polynomial of 0x1021

	* @param arg_pData  Array to calculate checksum
 	* @param arg_size	Length of the array
 	* @return          	CRC16 Checksum
	 */
    uint16_t crc16(uint8_t const *arg_pData, uint8_t arg_size);

public:
	/**
 	* @brief  Constructor
 	*/
    UART();

	/**
 	* @brief Begin Serial Communication to PC and OBC
 	*/
    void begin();

	/**
	 * @brief Packing and sending the payload
	 * 
	 * @param arg_pPayload 		The actual payload
	 * @param arg_payloadLen 	The payload's length 
	 */
    void transmit(uint8_t *arg_pPayload, uint8_t arg_payloadLen);

	/**
	 * @brief Receiving and decoding the message
	 * 
	 * @param arg_pPacket received pacakge content
	 * @return (int16_t) the length of the received package 
	 */
    int16_t receive(uint8_t *arg_pPacket);

};