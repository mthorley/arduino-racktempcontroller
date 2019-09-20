#ifndef _FAN_CONTROL_H
#define _FAN_CONTROL_H

#include <Arduino.h>
#include <ArduinoLog.h>
#include "ErrCodes.h"

#define MAX_DUTY_CYCLE 100  // %
#define MIN_DUTY_CYCLE 0

// assertions
#define ASSERT_RANGE(var, min, max, msg) if(var<min || var>max) { \
    Log.error(F(msg)); \
    return ERR_BAD_PARAM; \
}

#define ASSERT_RANGE_DUTY_CYCLE(var) ASSERT_RANGE( \
    var, MIN_DUTY_CYCLE, MAX_DUTY_CYCLE, "Duty cycle is out of range")

#define ASSERT_RANGE_FAN_ID(var, fans) ASSERT_RANGE(\
    var, 1, fans, "Fanid out of range")

/**
 * Abstract base class for all Fan Control. 
 * Sub types include Arduino and MAX31790 classes. 
 */
class FanControl
{
public:
    FanControl(const uint8_t fans) : 
        _fans(fans) {};

    virtual RESULT initialise() = 0;
    virtual RESULT setPWMForAll(const uint16_t dutyCycle) = 0;
    virtual RESULT setPWM(const uint8_t fanid, const uint16_t dutyCycle) = 0;
    virtual RESULT getTachCount(const uint8_t fanid, uint16_t& tachCount) = 0;

    const uint8_t getFanCount() const {
        return _fans;
    };

private:
    uint8_t _fans; // total number of fans to control
};

#endif