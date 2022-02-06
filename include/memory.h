struct ExpFields { // 6 bytes
    uint8_t time[2];
    uint8_t voltage[2];
    uint8_t current[2];
};

// 115 max
struct PayloadFields {  // 104 bytes
    uint16_t exprimentNo; // 2 bytes
    uint16_t startTemp; // 2 bytes
    ExpFields chargeData[8]; //6*8 = 48 bytes  
    uint16_t midTemp; // 2 bytes
    ExpFields dischargeData[8]; //6*8 = 48 bytes
    uint16_t endTemp;   // 2 bytes
};

union PayloadBuffer
{
    PayloadFields fields;
    uint8_t data[104];
};
