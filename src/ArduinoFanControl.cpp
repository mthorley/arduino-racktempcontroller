#include "ArduinoFanControl.h"

// interrupt callbacks
extern void ArduinoFanControl_tach1Count();
extern void ArduinoFanControl_tach2Count();
extern void ArduinoFanControl_tach3Count();
extern void ArduinoFanControl_tach4Count();

volatile uint16_t ArduinoFanControl_tach1 = 0;
volatile uint16_t ArduinoFanControl_tach2 = 0;
volatile uint16_t ArduinoFanControl_tach3 = 0;
volatile uint16_t ArduinoFanControl_tach4 = 0;

// externs
void ArduinoFanControl_tach1Count()
{
    ArduinoFanControl_tach1++;
}

void ArduinoFanControl_tach2Count()
{
    ArduinoFanControl_tach2++;
}

void ArduinoFanControl_tach3Count()
{
    ArduinoFanControl_tach3++;
}

void ArduinoFanControl_tach4Count()
{
    ArduinoFanControl_tach4++;
}

ArduinoFanControl::ArduinoFanControl(const uint8_t fans) : 
    FanControl(fans), 
    _pwmPeriod(40) {}  // 40us == 25kHz

/**
 * Initialise as PWM timers, tach inputs and
 * attach interrupts to measure.
 */
RESULT ArduinoFanControl::initialise()
{
    // Use two timers to drive 4 PWM pins - 2 each
    Timer1.initialize(_pwmPeriod);
    Timer3.initialize(_pwmPeriod);

    // For reading TACHs
    // Pullup - since TACH output is open collector and pullup rc reduces noise.
    pinMode(PIN_TACH1, INPUT_PULLUP);
    pinMode(PIN_TACH2, INPUT_PULLUP);
    pinMode(PIN_TACH3, INPUT_PULLUP);
    pinMode(PIN_TACH4, INPUT_PULLUP);

    // See https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
    attachInterrupt(digitalPinToInterrupt(PIN_TACH1), ArduinoFanControl_tach1Count, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_TACH2), ArduinoFanControl_tach2Count, CHANGE); 
    attachInterrupt(digitalPinToInterrupt(PIN_TACH3), ArduinoFanControl_tach3Count, CHANGE); 
    attachInterrupt(digitalPinToInterrupt(PIN_TACH4), ArduinoFanControl_tach4Count, CHANGE); 
    
    ArduinoFanControl_tach1 = 0;
    ArduinoFanControl_tach2 = 0;
    ArduinoFanControl_tach3 = 0;
    ArduinoFanControl_tach4 = 0;

    return RES_OK;
}

/**
 * Set PWM for fanid.
 * dutyCycle is ranged 0 to 100
 */
RESULT ArduinoFanControl::setPWM(const uint8_t fanid, const uint16_t dutyCycle)
{
    ASSERT_RANGE_DUTY_CYCLE(dutyCycle);
    ASSERT_RANGE_FAN_ID(fanid, getFanCount());

    // duty is from 0 to 1023
    switch (fanid)
    {
    case 1:
        Timer1.pwm(PIN_FAN1_T1, ((float)dutyCycle / 100) * 1023);
        break;

    case 2:
        Timer1.pwm(PIN_FAN2_T1, ((float)dutyCycle / 100) * 1023);
        break;

    case 3:
        Timer3.pwm(PIN_FAN3_T3, ((float)dutyCycle / 100) * 1023);
        break;

    case 4:
        Timer3.pwm(PIN_FAN4_T3, ((float)dutyCycle / 100) * 1023);
        break;

    default:
        break;
    }

    return RES_OK;
}

/**
 * Set PWM for all fans to duty
 */
RESULT ArduinoFanControl::setPWMForAll(const uint16_t dutyCycle)
{
    ASSERT_RANGE_DUTY_CYCLE(dutyCycle);
    
    RESULT res = RES_OK;
    for(int i=1; i<=getFanCount(); i++) {
        res = res & setPWM(i, dutyCycle);        
    }
    return res;
}

RESULT ArduinoFanControl::getTachHz(const uint8_t fanid, uint16_t& tachHz) 
{
    ASSERT_RANGE_FAN_ID(fanid, getFanCount());
    
    unsigned long tachTime = 750; //ms
    measureTach(fanid, tachTime);
    float tHz = readTach(fanid, tachTime);
    tachHz = round(tHz);
    return RES_OK;
}

RESULT ArduinoFanControl::getRPM(const uint8_t fanid, uint16_t& rpm) 
{
    uint16_t tachHz;
    getTachHz(fanid, tachHz);
    // Div 2 since TACH returns 2 pulses per revolution:
    // https://noctua.at/media/wysiwyg/Noctua_PWM_specifications_white_paper.pdf 
    rpm = (tachHz * 60.0) / 2;
    return RES_OK;
}

// private

void ArduinoFanControl::measureTach(const uint8_t fanid, unsigned long msWait)
{
    // reset interrupt count
    switch (fanid) {
        case 1:
            ArduinoFanControl_tach1 = 0;
            break;
        case 2:
            ArduinoFanControl_tach2 = 0;
            break;
        case 3:
            ArduinoFanControl_tach3 = 0;
            break;
        case 4:
            ArduinoFanControl_tach4 = 0;
            break;
    }

    // force wait to accumulate interrupts
    unsigned long t1 = millis();
    do {
        // nop
    } while ((millis() - t1) < msWait);
}

float ArduinoFanControl::readTach(const uint8_t fanid, unsigned long ms)
{
    // See https://www.pjrc.com/teensy/td_libs_TimerOne.html Interrupt Context Issues
    // noInterrupts();

    uint16_t tach = 0;
    switch (fanid) {
        case 1:
            tach = ArduinoFanControl_tach1;
            break;
        case 2:
            tach = ArduinoFanControl_tach2;
            break;
        case 3:
            tach = ArduinoFanControl_tach3;
            break;
        case 4:
            tach = ArduinoFanControl_tach4;
            break;
    }
    // interrupts();
    
    // Divide by 2: since interrupt is CHANGE: H-L and L-H for accuracy.
    float tachHz = (tach * 1000.0 ) / (2 * ms);
    return tachHz;
}
