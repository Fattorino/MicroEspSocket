#include "ToroEspSocketServer.h"

int TES_Server::_decodeIndex(uint8_t *payload, size_t length)
{
    String msg(payload, length);
    uint iEnd = msg.indexOf(">") + 1;
    if (iEnd > 0)
        return msg.substring(0, iEnd).toInt();
    else
        return -1;
}

String TES_Server::_decodeTag(uint8_t *payload, size_t length)
{
    String msg(payload, length);
    uint iEnd = msg.indexOf("=");
    uint iStart = msg.indexOf(">") + 1;
    if (iEnd > 0)
    {
        if (iStart > 0)
            return msg.substring(iStart, iEnd);
        else
            return msg.substring(0, iEnd);
    }
    else
        return msg;
}

String TES_Server::_decodeMsg(uint8_t *payload, size_t length)
{
    String msg(payload, length);
    uint index = msg.indexOf("=");
    if (index > 0)
        return msg.substring(index + 1);
    else
        return msg;
}

void TES_Server::start_wifi(String ssid, String pw, uint maxc)
{
    _maxConnections = maxc;

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid.c_str(), pw.c_str(), 4, 0, maxc);
    delay(500);

    // ANDROID 10 WORKAROUND==================================================
    WiFi.disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();
    wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
    my_config.ampdu_rx_enable = 0;
    esp_wifi_init(&my_config);
    esp_wifi_start();
    delay(500);
    // ANDROID 10 WORKAROUND==================================================

    _myIP = WiFi.softAPIP();
}

void TES_Server::start_ws(uint16_t port)
{
    _port = port;
    _ws = new WebSocketsServer(port, _maxConnections);
    _ws->begin();
    _ws->onEvent(this, _eventHandler);
}

void TES_Server::start_ws(uint16_t port, uint pingDelta, uint pingWait, uint failsToDisc)
{
    _port = port;
    _pingDelta = pingDelta;
    _pingWait = pingWait;
    _failsToDisc = failsToDisc;

    _ws = new WebSocketsServer(port, _maxConnections);
    _ws->begin();
    _ws->onEvent(this, _eventHandler);
    if (_pingDelta > 0 && _pingWait > 0 && _failsToDisc > 0)
        _ws->enableHeartbeat(_pingDelta, _pingWait, _failsToDisc);
}

void TES_Server::loop()
{
    _ws->loop();
}

void TES_Server::addUniversalListener(TES_SEvent event)
{
    _universalEvent = event;
    _universalEventToggle = true;
}

void TES_Server::addEventListener(String tag, TES_SEvent event)
{
    _eventList.insert(std::pair<String, TES_SEvent>(tag, event));
}

void TES_Server::sendMsg(String group, uint index, String tag, std::vector<String> msg)
{
    auto it = _cDevices.find(DeviceUID{group, index});
    if (it == _cDevices.end())
        return;

    uint8_t num = it->second;
    String payload = tag + "=";
    for (String &d : msg)
    {
        payload += d + "!";
    }
    // MAYBE TODO: delete last "!"

    _ws->sendTXT(num, payload);
}

void TES_Server::sendMsg(String group, uint index, String tag, String msg)
{
    auto it = _cDevices.find(DeviceUID{group, index});
    if (it == _cDevices.end())
        return;

    uint8_t num = it->second;
    String payload = tag + "=" + msg;

    _ws->sendTXT(num, payload);
}

void TES_Server::broadcastMsg(String group, String tag, std::vector<String> msg)
{
    String payload = tag + "=";
    for (String &d : msg)
    {
        payload += d + "!";
    }
    // MAYBE TODO: delete last "!"

    for (auto it = _cDevices.find(DeviceUID{tag, 0}); it != _cDevices.end(); it++)
    {
        uint8_t num = it->second;
        _ws->sendTXT(num, payload);
    }
}

void TES_Server::broadcastMsg(String group, String tag, String msg)
{
    String payload = tag + "=" + msg;
    for (auto it = _cDevices.find(DeviceUID{tag, 0}); it != _cDevices.end(); it++)
    {
        uint8_t num = it->second;
        _ws->sendTXT(num, payload);
    }
}

void TES_Server::broadcastMsg(String tag, std::vector<String> msg)
{
    String payload = tag + "=";
    for (String &d : msg)
    {
        payload += d + "!";
    }
    // MAYBE TODO: delete last "!"

    for (auto it = _cDevices.begin(); it != _cDevices.end(); it++)
    {
        uint8_t num = it->second;
        _ws->sendTXT(num, payload);
    }
}

void TES_Server::broadcastMsg(String tag, String msg)
{
    String payload = tag + "=" + msg;

    for (auto it = _cDevices.begin(); it != _cDevices.end(); it++)
    {
        uint8_t num = it->second;
        _ws->sendTXT(num, payload);
    }
}

void TES_Server::regroupDevice(DeviceUID device, String newGroup)
{
    auto deviceIt = _cDevices.find(device);
    uint8_t num = deviceIt->second;
    uint index = connectedDevices(newGroup);

    _cDevices.erase(deviceIt);
    _cDevices.insert(std::pair<DeviceUID, uint8_t>(DeviceUID{newGroup, index}, num));

    _ws->sendTXT(num, "ReGrOuP=" + newGroup);
    _ws->sendTXT(num, "InDeX=" + String(index));
}

IPAddress TES_Server::getIP() { return _myIP; }

uint TES_Server::connectedDevices(String group)
{
    uint count = 0;
    for (auto it = _cDevices.begin(); it != _cDevices.end(); it++)
    {
        if (it->first.group == group)
            count++;
    }
    return count;
}

uint TES_Server::connectedDevices() { return _cDevices.size(); }

void TES_Server::pingDelta(uint pingDelta)
{
    _pingDelta = pingDelta;
    if (_pingDelta > 0 && _pingWait > 0 && _failsToDisc > 0)
        _ws->enableHeartbeat(_pingDelta, _pingWait, _failsToDisc);
}

uint TES_Server::pingDelta() { return _pingDelta; }

void TES_Server::pingWait(uint pingWait)
{
    _pingWait = pingWait;
    if (_pingDelta > 0 && _pingWait > 0 && _failsToDisc > 0)
        _ws->enableHeartbeat(_pingDelta, _pingWait, _failsToDisc);
}

uint TES_Server::pingWait() { return _pingWait; }

void TES_Server::failsToDisc(uint failsToDisc)
{
    _failsToDisc = failsToDisc;
    if (_pingDelta > 0 && _pingWait > 0 && _failsToDisc > 0)
        _ws->enableHeartbeat(_pingDelta, _pingWait, _failsToDisc);
}

uint TES_Server::failsToDisc() { return _failsToDisc; }

void TES_Server::printList()
{
    Serial.println("=========");
    for (auto it = _cDevices.begin(); it != _cDevices.end(); it++)
    {
        Serial.print("Device: ");
        Serial.print(it->first.group);
        Serial.print(", ");
        Serial.print(it->first.index);
        Serial.print(" ==> ");
        Serial.println(it->second);
    }
    Serial.println("=========\n");
}
