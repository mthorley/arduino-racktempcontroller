#include "RackTempController.h"
#include <ArduinoLog.h>

/**
 * Process temperatures, modify fan speed, check for errors, update trends
 */
void RackTempController::process(RackState_t& rs) {
    
    // read temperatures
    readTempStates(rs.thermos);

    // adjust fan speeds based on temps
    adjustFanSpeeds(rs);
    
    // read fan tach/rpms - delay of 750ms per fan
    readFanSpeeds(rs.fans);

    // verify fan PWMs matches RPMs
    verifyFanStates(rs.fans);

    // analyse trends
    analyseTrends(rs);
};

void RackTempController::analyseTrends(RackState_t& rs) /* const */ {

    // cache RackState for trend analysis
    cache(rs);
    
    float acc = 0.0;
    uint16_t samples = 0;

    // iterate through history to accumulate temps, count samples
    for(auto it = _rsHistory.begin(); it != _rsHistory.end(); it++) {
        for(auto tt = it->thermos.begin(); tt != it->thermos.end(); tt++) {
            acc += tt->second.tempCelsuis;
            samples++;
        }
    }
    float movingAve = (float)acc/samples;
    Log.notice(F("Moving average temp - %F"), movingAve);

    rs.aveTempCelsius = movingAve;
    //rs.trend.movingAveTempCelsuis = movingAve;
    //rs.trend.accFanError =   
}

/**
 * Maintains FIFO queue. Not using queue STL 
 * as it doesn't provide iterator.
 */
void RackTempController::cache(const RackState_t& rs) {

    _rsHistory.push_back(rs);
    if (_rsHistory.size() > _historyDepth) {
        _rsHistory.pop_front();
    }
    Log.notice(F("history queue size %d"), _rsHistory.size());
}

/**
 * Confirm for the fan, that 
 * - RPMs are above minimum (if minRpm is not zero), i.e. the fan is detected as spinning
 * - PWM setting and RPM reading is within expectation, i.e. +/- 10%
 */
RESULT RackTempController::checkRpm(FanState_t& fs) const {

    // is fan spinning at all?
    if (fs.minRpm > 0 && fs.rpm < fs.minRpm) {
        fs.result = ERR_FAN_NOT_OPERATIONAL;
        Log.error(F("Fan %s is not operational"), fs.position.c_str());
        return ERR_FAN_NOT_OPERATIONAL;
    }

    // is the fan within expected RPM variance given dutyCycle?
    uint16_t expectedRpm = round( fs.maxRpm * (float)fs.pwm/100 );
   
    uint16_t minExpectedRpm = round(expectedRpm - expectedRpm * _rpmVariance);
    uint16_t maxExpectedRpm = round(expectedRpm + expectedRpm * _rpmVariance);
    if (fs.rpm < minExpectedRpm || fs.rpm > maxExpectedRpm) {
        fs.result = ERR_FAN_TACH;
        Log.error(F("Rpm of fan %s is out of range: %d is not between %d to %d"), 
            fs.position.c_str(),
            fs.rpm,
            minExpectedRpm,
            maxExpectedRpm);
        return ERR_FAN_TACH;
    }

    return RES_OK;
}

/**
 * Verify fan state: speed
 */
void RackTempController::verifyFanStates(Fans_t& fans) const {
    for (auto it = fans.begin(); it != fans.end(); it++) {
        checkRpm(it->second);
    }
}

void RackTempController::adjustFanSpeeds(RackState_t& rs) {

    // Basic rule: any thermo above 20C, set it at full spin
    // else run it at half speed.
    uint8_t dutyCycle = 50;
    for (auto it = rs.thermos.begin(); it != rs.thermos.end(); it++) {
        Temperature_t& thermo = it->second;
        if (thermo.tempCelsuis > 22) {
            dutyCycle = 100;
        }
    }

    // update fan speed and state
    _fanControl.setPWMForAll(dutyCycle);
    for (auto it = rs.fans.begin(); it != rs.fans.end(); it++) {
        it->second.pwm = dutyCycle;
    }

    Log.notice(F("Fan's pwm set at - %d"), dutyCycle);
}

/**
 * Get tach/rpm for all fans
 * Delay of 750ms per fan within getRPM()
 */
void RackTempController::readFanSpeeds(Fans_t& fans) {
    
    uint16_t rpm = 0;
    
    Log.notice(F("Reading fan rpms"));

    for (auto it = fans.begin(); it != fans.end(); it++) {
        _fanControl.getRPM(it->first, rpm);
        it->second.rpm = rpm;
        Log.notice(F("Fan %s rpm - %d"), it->second.position.c_str(), rpm);
    }
};

void RackTempController::readTempStates(Thermos_t& thermos) {

    // Initialise sensors each read incase new sensors are added/removed.
    _tempSensors.begin();

    // useful for new thermo's to get deviceAddress
    //  searchAndPrintAddresses();
    
    // Iterate through all devices ensuring they are still connected
    for (auto it = thermos.begin(); it != thermos.end(); it++) {        
        Temperature_t& thermo = it->second;
        if (!_tempSensors.isConnected(thermo.addr)) {
            Log.warning(F("Unable to find thermometer %s"), it->first.c_str());
            thermo.result = ERR_FAILED_TO_FIND_DEVICE;            
        }
    }
    
    // Send command to all DS18* for temperature conversion
    Log.notice(F("Requesting temperatures"));
    _tempSensors.requestTemperatures();

    // Get temperature for each thermometer
    for (auto it = thermos.begin(); it != thermos.end(); it++) {        
        Temperature_t& thermo = it->second;
        // if device found, then read temp
        if (thermo.result != ERR_FAILED_TO_FIND_DEVICE) {
            if ((thermo.tempCelsuis = _tempSensors.getTempC(thermo.addr)) == DEVICE_DISCONNECTED_C) {
                thermo.result = ERR_FAILED_TO_READ_TEMP;
                Log.warning(F("Failed to read %s temperature"), it->first.c_str());
            }
            else {
                Log.notice(F("%s.tempCelsuis - %F"), it->first.c_str(), thermo.tempCelsuis);         
            }
        }
    }
}
/**
 * Search for all devices and printout addresses
 */
void RackTempController::searchAndPrintAddresses() {

    DeviceAddress deviceAddr;
    // locate devices on the bus
    Serial.println("Locating devices...");
    Serial.print("Found ");
    int deviceCount = _tempSensors.getDeviceCount();
    Serial.print(deviceCount, DEC);
    Serial.println(" devices.");
    Serial.println("");

    Serial.println("Printing addresses...");
    for (int i = 0;  i < deviceCount;  i++)
    {
        Serial.print("Sensor ");
        Serial.print(i+1);
        Serial.print(" : ");
        _tempSensors.getAddress(deviceAddr, i);
        printAddress(deviceAddr);
    }
}

// function to print a device address for DS18*
void RackTempController::printAddress(const DeviceAddress deviceAddress) const
{
    for (uint8_t i = 0; i < 8; i++)
    {
        Serial.print("0x");
        if (deviceAddress[i] < 0x10) 
            Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
        if (i < 7) 
            Serial.print(", ");
    }
    Serial.println("");
}

RackState_t RackTempController::build() const {

    RackState_t rs;
    
    rs.thermos.insert({ 
        "topRack", {
            { 0x28, 0xAA, 0x48, 0x66, 0x53, 0x14, 0x01, 0xD5 },
            0.0,
            RES_OK
        }
    });

    rs.thermos.insert({ 
        "baseRack", {
            { 0x28, 0xAA, 0x51, 0x59, 0x53, 0x14, 0x01, 0x88 },
            0.0,
            RES_OK
        }
    });

    rs.aveTempCelsius = 0.0;

    rs.fans.insert({
        1, {
            "TL",
            0, 0,
            400, 1200,
            RES_OK
        }
    });

    rs.fans.insert({
        2, {
            "TR",
            0, 0,
            400, 1200,
            RES_OK
        }
    });

    rs.fans.insert({
        3, {
            "BL",
            0, 0,
            400, 1200,
            RES_OK
        }
    });

    rs.fans.insert({
        4, {
            "BR",
            0, 0,
            400, 1200,
            RES_OK
        }
    });

    return rs;
}

RackState_t RackTempController::build_debug() const {
    
    RackState_t rs;
    
    rs.thermos.insert({ 
        "topRack", {
            { 0x28, 0xFF, 0xBF, 0xDC, 0x51, 0x17, 0x04, 0x48 }, //keyes 
            0.0,
            RES_OK
        }
    });

    rs.aveTempCelsius = 0.0;
    
    rs.fans.insert({
        1,{
            "TL",
            0, 0,
            400, 1200,
            RES_OK
        }
    });

    return rs;
}