#ifndef _MAX31790_FANCONTROL_H
#define _MAX31790_FANCONTROL_H

#include "FanControl.h"

// registers
#define GLOBAL_CONFIG_REG     0x00
#define PWM_FREQ 			  0x01
#define FAN_CONFIG(ch)		  (0x02+(ch-1))
#define FAN_DYNAMICS(ch) 	  (0x08+(ch-1))

#define TACH_COUNT(ch)		         (0x18+(ch-1)*2)
#define PWM_DUTY_CYCLE(ch)	         (0x30+(ch-1)*2)
#define PWMOUT_TARGET_DUTY_CYCLE(ch) (0x40+(ch-1)*2)
#define TACH_TARGET(ch)		         (0x50+(ch-1)*2)

#define MAX_DUTY_CYCLE_SCALE 511    // range is 0-511

/**
 * Global Configuration
 * I2C watchdog setting 
 */
enum I2CWatchDogEnum 
{
    Disabled  = 0, 
    FiveSec   = 1, 
    TenSec    = 2, 
    ThirtySec = 3
};

enum OscillatorEnum 
{
    Internal         = 0, 
    External_Crystal = 1
};
  
struct GlobalConfig
{
    GlobalConfig(
        bool standBy_Not_Run       = 0,
        bool not_Bus_Timeout       = 0,
        OscillatorEnum oscillator  = OscillatorEnum::Internal,
        I2CWatchDogEnum watchDog   = I2CWatchDogEnum::Disabled)
    : 
        standBy_Not_Run(standBy_Not_Run), 
        normal_Not_Reset(0),
        not_Bus_Timeout(not_Bus_Timeout),
        oscillator(oscillator),
        watchDog(watchDog) {};
 
    bool            standBy_Not_Run:1;    // 0 = Run - D7
    bool            normal_Not_Reset:1;   // 0 = Normal
    bool            not_Bus_Timeout:1;    // 0 = Enabled
    bool            reserved:1;
    OscillatorEnum  oscillator:1;
    I2CWatchDogEnum watchDog:2;       
    bool            watchDogStatus:1;     // 1 = watchdog fault detected
};

// @TODO:
typedef struct {

} FanConfigStruct;

class MAX31790 : public FanControl
{
public:

    // constructors
    MAX31790(const uint8_t i2cAddress, const uint8_t fans);

    virtual RESULT initialise();    
    virtual RESULT setPWMForAll(const uint16_t dutyCycle);
    virtual RESULT setPWM(const uint8_t fanid, const uint16_t dutyCycle);
    virtual RESULT getTachCount(const uint8_t fanid, uint16_t& tachCount);

    virtual RESULT getTachHz(const uint8_t fanid, uint16_t& tachHz);
    virtual RESULT getRPM(const uint8_t fanid, uint16_t& rpm);

    RESULT getGlobalConfiguration(GlobalConfig& config);
    //RESULT setGlobalConfiguration(const GlobalConfig& config);
    //RESULT setFanConfigForAll(const FanConfigStruct& config);

    // utility to scan for all i2c devices 
    void scanForI2C();
    
    // accessors
    uint8_t getDeviceAddress() {
        return _deviceAddress;
    };

private:
    RESULT readByte(const uint8_t address, uint8_t& result);
    RESULT readBytes(const uint8_t address, const uint8_t n, uint8_t* result);
    RESULT writeByte(const uint8_t address, const uint8_t byte);
    RESULT writeBytes(const uint8_t address, const uint8_t* bytes, const uint8_t nBytes);
    
    uint16_t scaleDutyCycle(const uint16_t dutyCycle) const;

    uint8_t _deviceAddress; // device address - note this is 7bit
};

#endif 
