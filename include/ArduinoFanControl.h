#ifndef _ARDUINO_FANCONTROL_H
#define _ARDUINO_FANCONTROL_H

#include <TimerOne.h>
#include <TimerThree.h>
#include "FanControl.h"

class ArduinoFanControl : public FanControl
{
public:
    // constructors
    ArduinoFanControl(const uint8_t fans);

    virtual RESULT initialise();
    virtual RESULT setPWMForAll(const uint16_t dutyCycle);
    virtual RESULT setPWM(const uint8_t fanid, const uint16_t dutyCycle);
    virtual RESULT getTachCount(const uint8_t fanid, uint16_t& tachCount);

private:
    void measureTach(const uint8_t fanid, unsigned long ms);
    float getTach(const uint8_t fanid, unsigned long ms);

    uint16_t _pwmPeriod;
  
    // drives PWM PINs
    // based on https://github.com/PaulStoffregen/TimerOne/blob/master/config/known_16bit_timers.h 
    const char PIN_FAN1_T1 = 11; // Timer1
    const char PIN_FAN2_T1 = 12; // Timer1
    const char PIN_FAN3_T3 = 5;  // Timer3
    const char PIN_FAN4_T3 = 3;  // Timer3

    // interrupts for TACH input
    // see https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
    const char PIN_TACH1 = 21;
    const char PIN_TACH2 = 20;
    const char PIN_TACH3 = 19;
    const char PIN_TACH4 = 18;

};

#endif
