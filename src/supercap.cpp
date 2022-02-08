#include "supercap.h"

//------------------------------------------------------------------------
SuperCap::SuperCap(int pinCharge, int pinDischarge, int pinTemp): 
    m_pinCharge(pinCharge), 
    m_pinDischarge(pinDischarge),
    m_pinTemp(pinTemp) {}

//------------------------------------------------------------------------
void SuperCap::init() 
{
    pinMode(m_pinCharge,OUTPUT);
    pinMode(m_pinDischarge,OUTPUT);
    pinMode(m_pinTemp,INPUT);
}

//------------------------------------------------------------------------
void SuperCap::setIdle()
{
    digitalWrite(m_pinCharge,LOW);
    digitalWrite(m_pinDischarge,LOW);
}

//------------------------------------------------------------------------
void SuperCap::setCharge()
{
    digitalWrite(m_pinCharge,HIGH);
    digitalWrite(m_pinDischarge,LOW);
}

//------------------------------------------------------------------------
void SuperCap::setDischarge()
{
    digitalWrite(m_pinCharge,LOW);
    digitalWrite(m_pinDischarge,HIGH);
}

//------------------------------------------------------------------------
int SuperCap::readTemperature()
{

}