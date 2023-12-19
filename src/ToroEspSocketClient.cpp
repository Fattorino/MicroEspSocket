#include "MicroEspSocketClient.h"

String TES_Client::_decodeTag(uint8_t *payload, size_t length)
{
    String msg(payload, length);
    uint index = msg.indexOf("=");
    if (index > 0)
        return msg.substring(0, index);
    else
        return msg;
}

String TES_Client::_decodeMsg(uint8_t *payload, size_t length)
{
    String msg(payload, length);
    uint index = msg.indexOf("=");
    if (index > 0)
        return msg.substring(index + 1);
    else
        return msg;
}

TES_Client::TES_Client(String group)
{
    _group = group;
    _isConnected = false;
}

void TES_Client::connect_to_wifi(String ssid, String pw)
{
    WiFi.begin(ssid.c_str(), pw.c_str());
    // while (WiFi.status() != WL_CONNECTED)
    // {
    //     delay(100);
    // }
}

void TES_Client::start_ws(String ip, uint16_t port)
{
    _ws = new WebSocketsClient;

    _ws->begin(ip, port);
    _ws->onEvent(this, _eventHandler);
}

void TES_Client::start_ws(String ip, uint16_t port, uint rconnectDelta)
{
    _ws = new WebSocketsClient;
    _rconnectDelta = rconnectDelta;

    _ws->begin(ip, port);
    _ws->onEvent(this, _eventHandler);
    _ws->setReconnectInterval(rconnectDelta);
}

void TES_Client::loop()
{
    _ws->loop();
}

void TES_Client::addUniversalListener(TES_CEvent event)
{
    _universalEvent = event;
    _universalEventToggle = true;
}

void TES_Client::addEventListener(String tag, TES_CEvent event)
{
    _eventList.insert(std::pair<String, TES_CEvent>(tag, event));
}

void TES_Client::sendMsg(String tag, std::vector<String> msg)
{
    String payload = String(_index) + ">" + tag + "=";
    for (String &d : msg)
    {
        payload += d + "!";
    }
    // MAYBE TODO: delete last "!"

    _ws->sendTXT(payload);
}

void TES_Client::sendMsg(String tag, String msg)
{
    String payload = String(_index) + ">" + tag + "=" + msg;
    _ws->sendTXT(payload);
}

void TES_Client::rconnectDelta(uint rconnectDelta)
{
    _rconnectDelta = rconnectDelta;
    _ws->setReconnectInterval(rconnectDelta);
}

uint TES_Client::rconnectDelta() { return _rconnectDelta; }

bool TES_Client::connected() { return _isConnected; }
