// AVR use little endian so we will need to create array of uint8_t to manually push for big endian
#define EXP_FIELDS_SIZE 6

struct DataPointFields { // 6 bytes
    uint8_t time[2];
    uint8_t voltage[2];
    uint8_t current[2];
};

// 115 max
struct ExprimentFields {  // 104 bytes
    uint8_t exprimentNo[2]; // 2 bytes
    uint8_t startTemp[2]; // 2 bytes
    DataPointFields chargeData[8]; //6*8 = 48 bytes  
    uint16_t midTemp; // 2 bytes
    DataPointFields dischargeData[8]; //6*8 = 48 bytes
    uint16_t endTemp;   // 2 bytes
};

union ExprimentBuffer
{
    ExprimentFields fields;
    uint8_t data[104];
};
