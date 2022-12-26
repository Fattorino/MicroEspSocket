#ifndef TOROESPSOCKETCLIENT_H
#define TOROESPSOCKETCLIENT_H

#include <Arduino.h>
#include <vector>
#include <map>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "WebSocketBase/WebSocketsClient.h"

typedef std::function<void(String msg)> TES_Event;

class TES_Client
{
private:
    String _ip;
    uint16_t _port;
    String _group;
    uint _index;
    WebSocketsClient *_ws;
    uint _rconnectDelta;

    std::map<String, TES_Event> _eventList;
    TES_Event _universalEvent;
    bool _universalEventToggle = false;

    String _decodeTag(uint8_t *payload, size_t length);
    String _decodeMsg(uint8_t *payload, size_t length);

public:
    friend void _eventHandler(TES_Client *th, WStype_t type, uint8_t *payload, size_t length);

    TES_Client(String group);
    ~TES_Client() {}

    void connect_to_wifi(String ssid, String pw);
    void start_ws(String ip, uint16_t port);
    void start_ws(String ip, uint16_t port, uint rconnectDelta);
    void loop();

    void addUniversalListener(TES_Event event);
    void addEventListener(String tag, TES_Event event);

    void sendMsg(String tag, std::vector<String> msg);
    void sendMsg(String tag, String msg);

    void rconnectDelta(uint rconnectDelta);
};

void _eventHandler(TES_Client *th, WStype_t type, uint8_t *payload, size_t length)
{

    switch (type)
    {
    case WStype_CONNECTED:
    {
        th->_ws->sendTXT("IdEnTiFiEr" + th->_group);
    }
    break;

    case WStype_TEXT:
    {
        String tag = th->_decodeTag(payload, length);
        String msg = th->_decodeMsg(payload, length);

        if (th->_universalEventToggle)
        {
            th->_universalEvent(msg);
        }

        if (tag == "InDeX")
        {
            th->_index = msg.toInt();
            break;
        }

        if (tag == "ReGrOuP")
        {
            // TODO: Set new stuff
            break;
        }

        auto it = th->_eventList.find(tag);
        if (it != th->_eventList.end())
            it->second(msg);
    }

    default:
        break;
    }
}

#endif // !TOROESPSOCKETCLIENT_H