#include "MqttManager.h"
#include <ArduinoLog.h>

extern void onMqttMessage(int messageSize);

int MqttManager::initialise()
{
    if (!_p_mqttClient->connect(_serverIP.c_str(), _port)) {
        Log.error(F("Mqtt connection failed - error code %d"), _p_mqttClient->connectError());
        return _p_mqttClient->connectError();
    }
    _p_mqttClient->setId(_clientID);

    // set message handler for callback
    _p_mqttClient->onMessage(::onMqttMessage);

    // subscribe to config topic
    _p_mqttClient->subscribe(topicConfig);

    return 0;
}

void MqttManager::publish(RackState_t& rs) {
    
    if (!_p_mqttClient->connected()) {
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

void MqttManager::poll() {
    _p_mqttClient->poll();
}

void MqttManager::sendMessage(const String& topic, const String& msg)
{
    _p_mqttClient->beginMessage(topic);
    _p_mqttClient->print(msg);
    if (!_p_mqttClient->endMessage())
        Log.error(F("Failed to send message to topic %s"), topic.c_str());
}

/**
 * From Print.h
 * Enables writing all log events to the topicRackLog
 */
size_t MqttManager::write(uint8_t c) {
    Serial.print(char(c));
    _buf += (char)c;
    if (c == '\n') {
        if (_p_mqttClient->connected()) {
            _p_mqttClient->beginMessage(topicRackLog);
            _p_mqttClient->print(_buf);
            _p_mqttClient->endMessage();
        }
        _buf = "";
    }
    return 1;
};
