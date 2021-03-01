#include "RackTempController.h"
#include <Ethernet.h>
#include <ArduinoMqttClient.h>
#include <Print.h>

/**
 * Wrapper class for MQTT and Logging framework
 */ 
class MqttManager : public Print {

public:
    MqttManager(MqttClient* p_mqttClient, const char* clientID, const char* serverIP, const uint16_t port) :
        _p_mqttClient(p_mqttClient),
        _clientID(clientID),
        _serverIP(serverIP),
        _port(port) {};

    int initialise();

    void publish(RackState_t& rs);

    void poll();

    /**
     * From Print.h
     */
    size_t write(uint8_t c);
    
private:
    void sendMessage(const String& topic, const String& msg);

    MqttClient* _p_mqttClient;

    const String   _clientID;   // client identifier
    const String   _serverIP;   // mqtt server IP endpoint
    const uint16_t _port;       // mqtt server port
    String         _buf;        // log buffer

    const String topicTempRackTop  = "device/temp/rack/top";
    const String topicTempRackBase = "device/temp/rack/base";
    const String topicTempRackAve  = "device/temp/rack/average";
    
    const String topicFanTopLeft   = "device/rack/fan/topleft";
    const String topicFanTopRight  = "device/rack/fan/topright";
    const String topicFanBaseLeft  = "device/rack/fan/baseleft";
    const String topicFanBaseRight = "device/rack/fan/baseright";

    const String topicConfig      = "device/rack/config";  //display on/off subscriber topic 

    const String topicRackLog     = "device/rack/log";
    const String subtopicFanError = "/error";
    const String subtopicFanRPM   = "/rpm";
};
