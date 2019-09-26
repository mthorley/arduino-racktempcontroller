#ifndef UNIT_TEST

#include <ArduinoLog.h>
#include "RackTempController.h"
#include "OLEDDisplay.h"
#include "MqttPublisher.h"
//#include "MAX31790FanControl.h"
#include "ArduinoFanControl.h"

/*
All pin usage
2, 4, 7         - OLED
8               - Onewire DS
11, 12, 5, 3    - Fan PWM
18, 19, 20, 21  - Fan Tach for RPM
6               - IR sensor
*/

// Pins
#define PIN_DC     2        // OLED
#define PIN_RESET  4        // OLED
#define PIN_CS     7        // OLED
#define PIN_ONE_WIRE_BUS 8  // Onewire for DS18B20s
#define PIN_IR     6        // IR Sensor

byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// MQTT config
const char* CLIENT_ID      = "RackTempController.Arduino.Client";
const char* MQTT_SERVER_IP = "172.16.0.143";
const uint16_t MQTT_PORT   = 1883;

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(PIN_ONE_WIRE_BUS);

//MAX31790           fanControl(0xC0, 4);
ArduinoFanControl  fanControl(4);
RackTempController rtc(oneWire, fanControl);
OLEDDisplay        oled(PIN_CS, PIN_DC, PIN_RESET, PIN_IR);
EthernetClient     ethClient;
MqttPublisher      mqttPublish(ethClient, CLIENT_ID, MQTT_SERVER_IP, MQTT_PORT);

bool ethernetPresent = false;

RESULT ethernetSetup() {
    RESULT res = RES_OK;

    if (Ethernet.linkStatus() == LinkOFF) {
        Log.error(F("Ethernet cable is not connected."));
        res = ERR_NO_CABLE_DETECTED;
        return res;
    }

    // Start the Ethernet connection and the server
    if (Ethernet.begin(mac)==0) {
        Log.error(F("Failed to get IP from DHCP"));
        res = ERR_FAILED_TO_GET_IP_FROM_DHCP;
        return res;
    }

    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Log.error(F("Ethernet hardware not found"));
        res = ERR_NO_ETHERNET_HW;
        return res;
    }

    ethernetPresent = true;    
    return res;
}

// log support
void printTimestamp(Print* logOutput) {
    char c[12];
    sprintf(c, "%10lu ", millis());
    logOutput->print(c);
}

void printNewline(Print* logOutput) {
    logOutput->print('\n');
}

void setup(void)
{
    Serial.begin(9600);

    // setup logging
//    Log.begin(LOG_LEVEL_WARNING, &mqttPublish); // use &Serial only to avoid MQTT events
    Log.begin(LOG_LEVEL_VERBOSE, &mqttPublish);   // use &Serial only to avoid log output being published as events
    Log.setPrefix(printTimestamp);
    Log.setSuffix(printNewline);

    fanControl.initialise();

    oled.initialise();

    RESULT res = ethernetSetup();
    if (res == RES_OK) {
        oled.render("Ethernet initialised");
        Log.notice(F("Ethernet initialised"));
        
        mqttPublish.initialise();
        
        oled.render("Mqtt initialised");
        Log.notice(F("Mqtt initialised"));
    }
    else {
        oled.render("Ethernet failed to initialised");
        Log.error(F("Ethernet failed to initalise %d"), res);        
    }
}
  
void loop(void) {

    NetworkState_t ns;
    RackState_t rs = rtc.build();
    
    if (ethernetPresent) {
        ns.ethernetIP = Ethernet.localIP();
        ns.mqttServerIP = MQTT_SERVER_IP;
        ns.mqttPort = MQTT_PORT;

        mqttPublish.poll();
    }

    // read temperatures, modify fan speed
    rtc.process(rs);

    // render rack state, network state
    oled.render(rs, ns);

    if (ethernetPresent) {
        // emit rackstate data, will reconnect if required
        mqttPublish.publish(rs);

        // maintain IP via DHCP
        int res = Ethernet.maintain();
        if (res!=0)
            Log.notice(F("Ethernet maintain - %d"), res);
    }    
}

#endif