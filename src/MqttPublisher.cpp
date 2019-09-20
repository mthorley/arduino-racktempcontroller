#include "MqttPublisher.h"
#include <ArduinoLog.h>

void MqttPublisher::initialise()
{
    if (!_mqttClient.connect(_serverIP.c_str(), _port)) {
        Log.error(F("Mqtt connection failed - error code %d"), _mqttClient.connectError());
    }
    _mqttClient.setId(_clientID);
}

void MqttPublisher::publish(RackState_t& rs) {
    
    if (!_mqttClient.connected()) {
        Log.error(F("Mqtt connection lost ... attempting reconnection"));
        initialise();        
    }

    Log.notice(F("Publishing temperature events"));
    sendMessage(topicTempRackTop, String(rs.thermos["topRack"].tempCelsuis));
    sendMessage(topicTempRackBase, String(rs.thermos["baseRack"].tempCelsuis));
    sendMessage(topicTempRackAve, String(rs.aveTempCelsius));
    
    //Log.notice(F("Publishing fan events"));
    /*
    sendMessage(topicFanBaseLeft + subtopicFanRPM, String(rs.fanTopLeft.rpm));
    sendMessage(topicFanTopRight + subtopicFanRPM, String(rs.fanTopRight.rpm));
    sendMessage(topicFanBaseLeft + subtopicFanRPM, String(rs.fanBaseLeft.rpm));
    sendMessage(topicFanTopRight + subtopicFanRPM, String(rs.fanBaseRight.rpm));
    */

    // if (rs.hasFanError()) {  
    //     for each fan in error
    //        sendMessage(topicRackFanError, rs.fanid)
    //}
    
}

void MqttPublisher::poll() {
    _mqttClient.poll();
}

void MqttPublisher::sendMessage(const String& topic, const String& msg)
{
    _mqttClient.beginMessage(topic);
    _mqttClient.print(msg);
    if (!_mqttClient.endMessage())
        Log.error(F("Failed to send message to topic %s"), topic.c_str());
}

/**
 * From Print.h
 * Enables writing all log events to the topicRackLog
 */
size_t MqttPublisher::write(uint8_t c) {
    Serial.print(char(c));
    _buf += (char)c;
    if (c == '\n') {
        if (_mqttClient.connected()) {
            _mqttClient.beginMessage(topicRackLog);
            _mqttClient.print(_buf);
            _mqttClient.endMessage();
        }
        _buf = "";
    }
    return 1;
};
