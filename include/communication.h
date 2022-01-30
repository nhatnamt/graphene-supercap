#include <Arduino.h>

#define BUFFER_SIZE 128
#define START_FLAG_MSB  0x19
#define START_FLAG_LSB  0x94

typedef enum TRANSFER_RESPONSE_CODE{
  MASTER_REQUEST_DATA,
  MASTER_ISSUING_DATA,
  SLAVE_ACK,
} tranfer_resposnse_code_t;

class SerialHandler {
private:
    char m_payload[115] = {0};
    char m_payloadLen   = 0;

    uint16_t crc16(char const *arg_pData, char arg_size);
public:
    SerialHandler();
    void begin();

    void serialTx();
    void handler();
    void setPayload(char arg_payload[115], char arg_len);
};