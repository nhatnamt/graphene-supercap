#include <Adafruit_INA260.h>

class SuperCap : public Adafruit_INA260
{
    private:
        const int m_pinCharge;
        const int m_pinDischarge;
        const int m_pinTemp;
    public:
        SuperCap(int pinCharge, int pinDischarge, int pinTemp);

        void setCharge();
        void setDischarge();
        void setIdle();

        int readTemperature();
};