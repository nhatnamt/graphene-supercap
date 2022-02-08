/*!
 *  @file supercap.h
 *
 * 	Super capacitor driver based on I2C Driver
 *  for INA260 Current and Power sensor
 *
 */
#include <Adafruit_INA260.h>

/**
 * @brief Class that is responsible for controlling the super capacitor circuit
 * and taking measurement, inherit from the Adafruit_INA260 library
 * 
 */
class SuperCap : public Adafruit_INA260
{
    private:
        const int m_pinCharge;
        const int m_pinDischarge;
        const int m_pinTemp;

        const int R1 = 1000;
        const int NTC = 10000; // assumed only

    public:
        /**
         * @brief Contrusctor: pin location to control 
         *  charge, discharge and read temperature operation
         */
        SuperCap(int pinCharge, int pinDischarge, int pinTemp);

        /**
         * @brief Intialize: set pin mode
         */
        void init();

        /**
         * @brief Set the mode to Charge
         */
        void setCharge();

        /**
         * @brief Set the mode to Discharge
         */
        void setDischarge();

        /**
         * @brief Set the mode to Idle, 
         * i.e. detach both charge and discharge
         */
        void setIdle();

        /**
         * @brief Read the temperature of the capacitors through NTC thermistor
         * 
         * @return (int) the temperature in C?
         */
        int readTemperature();
};