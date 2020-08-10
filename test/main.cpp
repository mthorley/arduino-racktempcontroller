#include <ArduinoLog.h>
#include "RackTempController.h"
#include "OLEDDisplay.h"
#include "SevenSegmentRender.h"
#include <vector>

// Pins
#define PIN_DC     2        // OLED
#define PIN_RESET  4        // OLED
#define PIN_CS     7        // OLED
#define PIN_ONE_WIRE_BUS 8  // Onewire for DS18B20s

byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

OLED oled(PIN_CS, PIN_DC, PIN_RESET);
OLEDDisplay oledDisplay(oled);
SevenSegmentRender ssr(oled);

std::vector<RackState_t> testRackStates;

// log support
void printTimestamp(Print* logOutput) {
    char c[12];
    sprintf(c, "%10lu ", millis());
    logOutput->print(c);
}

void printNewline(Print* logOutput) {
    logOutput->print('\n');
}
  
RackState_t testcase_normal_operation() {
    RackState_t rs;
    rs.thermos.insert({ "topRack",  {{ 0x28, 0xAA, 0x48, 0x66, 0x53, 0x14, 0x01, 0xD5 }, 21.3, RES_OK } });
    rs.thermos.insert({ "baseRack", {{ 0x28, 0xAA, 0x51, 0x59, 0x53, 0x14, 0x01, 0x88 }, 21.4, RES_OK } });
    rs.aveTempCelsius = 21.35;
    rs.fans.insert({1, { "TL", 100, 1200, 400, 1200, RES_OK } });
    rs.fans.insert({2, { "TR", 100, 1200, 400, 1200, RES_OK } });
    rs.fans.insert({3, { "BL", 100, 1150, 400, 1200, RES_OK } });
    rs.fans.insert({4, { "BR", 100, 1170, 400, 1200, RES_OK } });
    return rs;
}

RackState_t testcase_single_fan_fail() {
    RackState_t rs = testcase_normal_operation();
    rs.fans[2].result = ERR_FAN_NOT_OPERATIONAL;
    rs.fans[2].rpm = 0;
    return rs;
}

RackState_t testcase_total_fan_fail() {
    RackState_t rs = testcase_normal_operation();
    for(int i=1;i<5;i++) {
        rs.fans[i].result = ERR_FAN_NOT_OPERATIONAL;
        rs.fans[i].rpm = 0;
    }
    return rs;
}

RackState_t testcase_fan_out_of_range() {
    RackState_t rs = testcase_normal_operation();
    rs.fans[3].result = ERR_FAN_TACH;
    rs.fans[3].rpm = 500;
    return rs;
}

RackState_t testcase_temps_low() {
    RackState_t rs = testcase_normal_operation();
    rs.thermos["topRack"].tempCelsuis = 0.1;
    return rs;
}

RackState_t testcase_thermo_missing() {
    RackState_t rs = testcase_normal_operation();
    rs.thermos["topRack"].result = ERR_FAILED_TO_FIND_DEVICE;
    rs.thermos["topRack"].tempCelsuis = 0.0;
    rs.thermos["baseRack"].result = ERR_FAILED_TO_FIND_DEVICE;
    rs.thermos["baseRack"].tempCelsuis = 0.0;
    return rs;
}

RackState_t testcase_allerrs() {
    RackState_t rs = testcase_thermo_missing();
    for(int i=1;i<5;i++) {
        rs.fans[i].result = ERR_FAN_NOT_OPERATIONAL;
        rs.fans[i].rpm = 0;
    }
    return rs;    
}

void setup(void)
{
    Serial.begin(9600);

    // setup logging
    Log.begin(LOG_LEVEL_VERBOSE, &Serial);
    Log.setPrefix(printTimestamp);
    Log.setSuffix(printNewline);

    oledDisplay.initialise();

    testRackStates.push_back(testcase_normal_operation());
    testRackStates.push_back(testcase_single_fan_fail());
    testRackStates.push_back(testcase_total_fan_fail());
    testRackStates.push_back(testcase_fan_out_of_range());
    testRackStates.push_back(testcase_temps_low());
    testRackStates.push_back(testcase_thermo_missing());
    testRackStates.push_back(testcase_allerrs());
}

/**
 * This is not really testing, but visual inspection of 
 * the oled display based on rackstates.
 */
void loop(void) {

    Log.notice(F("FanController::Test Suite"));
    NetworkState_t ns;
    ns.ethernetIP = IPAddress(192, 168, 0, 11);

    RackState_t rs;

    for(int i=0;i<10000;i++) {
        oledDisplay.setOrientation(OLED_Orientation::ROTATE_0);
        oledDisplay.clearDisplay();
        for(auto it = testRackStates.begin(); it != testRackStates.end(); it++) {
            rs = *it;
            oledDisplay.render(rs, ns);
            delay(500);
        }
        oledDisplay.setOrientation(OLED_Orientation::ROTATE_90);
        oledDisplay.clearDisplay();
        for(auto it = testRackStates.begin(); it != testRackStates.end(); it++) {
            rs = *it;
            oledDisplay.render(rs, ns);
            delay(500);
        }
    }
}
