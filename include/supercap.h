#include <Adafruit_INA260.h>

class SuperCap : public Adafruit_INA260
{
    private:
        const int m_pinCharge;
        const int m_pinDischarge;
    public:
        SuperCap(int pinCharge, int pinDischarge);

        void setCharge();
        void setDischarge();
        void setIdle();

        int readTemperature();
};