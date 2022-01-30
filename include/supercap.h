#include <Wire.h>

class SuperCap
{
public: 
    /**
     * @brief Construct a new Super Cap object
     * 
     * @param _i2c_pipe   Which wire(i2c) object to use
     * @param _addr  Address of the slave SuperCap IC
     */
    SuperCap(TwoWire *_i2c_pipe, uint8_t _addr){
        i2c_pipe = _i2c_pipe;
        addr = _addr;
    }

    void begin() {
        i2c_pipe->begin();
    }

    void setCharge() {
        i2c_pipe->beginTransmission(2);
        i2c_pipe->write(2);
        i2c_pipe->endTransmission();
    }

    void setDischarge() {
        i2c_pipe->beginTransmission(2);
        i2c_pipe->write(1);
        i2c_pipe->endTransmission();
    }

<<<<<<< HEAD
    void setCurrent() {
=======
    void setIdle() {
>>>>>>> 5eb5a1424c2effd36b17623b1c8813fbe6e459a7
        i2c_pipe->beginTransmission(2);
        i2c_pipe->write(1);
        i2c_pipe->endTransmission();
    }

<<<<<<< HEAD
    void setIdle() {
=======
    void setCurrent(int mA) {
>>>>>>> 5eb5a1424c2effd36b17623b1c8813fbe6e459a7
        i2c_pipe->beginTransmission(2);
        i2c_pipe->write(1);
        i2c_pipe->endTransmission();
    }
<<<<<<< HEAD
=======

>>>>>>> 5eb5a1424c2effd36b17623b1c8813fbe6e459a7

private:
    TwoWire *i2c_pipe;
    uint8_t addr;
};
