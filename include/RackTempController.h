#ifndef __RACK_TEMP_CONTROLLER_H
#define __RACK_TEMP_CONTROLLER_H

#include <Arduino.h>
#include <Wire.h>
#include <Ethernet.h>
#include <DallasTemperature.h>
#include <ArduinoSTL.h>
#include <list>
#include <map>
#include "FanControl.h"

typedef struct {
    DeviceAddress addr;        // address for DS*
    float         tempCelsuis; // temp in C
    RESULT        result;      // reading outcome
} Temperature_t;

typedef struct {
    String   position;  // TL, TR etc
    uint8_t  pwm;       // % of dutycycle
    uint16_t rpm;       // tach count
    uint16_t minRpm;    // minRpm may be >0
    uint16_t maxRpm;
    RESULT   result;    // state: OK, not operational, etc
} FanState_t;

typedef std::map <String, Temperature_t> Thermos_t;
typedef std::map <uint8_t, FanState_t>   Fans_t;

typedef struct {
    Thermos_t thermos;
    float     aveTempCelsius; 
    Fans_t    fans;
} RackState_t;

typedef struct {
    IPAddress ethernetIP;    // may change via DHCP
    String    mqttServerIP;
    uint16_t  mqttPort;
} NetworkState_t;

/**
 * Controller class for thermometers (using OneWire)
 * and fans under the interface FanControl.
 * Any number of thermos or fans can be supported as 
 * long as these are configured by factory methods:
 * build() and build_debug();
 */
class RackTempController
{    
public:
    RackTempController(OneWire& oneWire, FanControl& fanControl) :
        _tempSensors(&oneWire), 
        _fanControl(fanControl) {};

    // process temps, update PWMs, read fan tach ...
    void process(RackState_t& rackState);

    // factory methods
    RackState_t build() const;
    RackState_t build_debug() const;    // for testing support

    // search for DS18* devices and print addresses
    void searchAndPrintAddresses();
  
protected:
    void readFanSpeeds(Fans_t& fs);
    void readTempStates(Thermos_t& ts);
    void adjustFanSpeeds(RackState_t& rs);
    void verifyFanStates(Fans_t& fs) const;
    void analyseTrends(RackState_t& rs) /* const */;

    void printAddress(const DeviceAddress deviceAddress) const;

private:
    DallasTemperature  _tempSensors;
    FanControl&        _fanControl;   // fan control implementation

    std::list <RackState_t> _rsHistory;    // for trend analysis
    const uint8_t _historyDepth = 10;
    const float   _rpmVariance  = 0.1;     // 10%

    RESULT checkRpm(FanState_t& fs) const;
    void cache(const RackState_t& rs);
};

#endif