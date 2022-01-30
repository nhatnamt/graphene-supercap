// based on Skykaft example
#include <Arduino.h>

#define BUFFER_SIZE 128

enum TransferResponseCode{
  MASTER_REQUEST_DATA,
  MASTER_ISSUING_DATA,
  SLAVE_ACK,
};

enum RxState {
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

class UART {
private:
    char rxByte;
    uint8_t rxState;
    uint16_t rxPacketLen, rxPacketCount;

    uint16_t crc16(uint8_t const *arg_pData, uint8_t arg_size);
public:
    UART();
    void begin();

    void transmit(uint8_t *arg_pPayload, uint8_t arg_payloadLen);
    int8_t receive(uint8_t *arg_pPacket);
};