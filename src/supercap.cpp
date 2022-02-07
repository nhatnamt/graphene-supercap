#include "supercap.h"

SuperCap::SuperCap(int pinCharge, int pinDischarge, int pinTemp): 
    m_pinCharge(pinCharge), 
    m_pinDischarge(pinDischarge),
    m_pinTemp(pinTemp) {}