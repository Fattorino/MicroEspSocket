#ifndef TOROESPSOCKETSERVER_H
#define TOROESPSOCKETSERVER_H

#include <Arduino.h>
#include <vector>
#include <map>

#include <WiFi.h>
#include <esp_wifi.h>
#include "WebSocketBase/WebSocketsServer.h"

typedef std::function<void(String msg)> TES_Event;

struct DeviceUID
{
    String group;
    uint index;

    bool operator<(const DeviceUID &cmp) const
    {
        return (group.charAt(0) * 100 + group.charAt(1) * 10 + index) < (cmp.group.charAt(0) * 100 + cmp.group.charAt(1) * 10 + cmp.index);
    }
};

class TES_Server
{
private:
    IPAddress _myIP;
    uint16_t _port;
    uint _maxConnections;
    WebSocketsServer *_ws;
    uint _pingDelta = 0;
    uint _pingWait = 0;
    uint _failsToDisc = 0;

    std::map<String, TES_Event> _eventList;
    TES_Event _universalEvent;
    bool _universalEventToggle = false;

    std::map<DeviceUID, uint8_t> _cDevices;

    String _decodeTag(uint8_t *payload, size_t length);
    String _decodeMsg(uint8_t *payload, size_t length);

public:
    friend void _eventHandler(TES_Server *th, uint8_t num, WStype_t type, uint8_t *payload, size_t length);

    TES_Server() {}
    ~TES_Server() { delete _ws; }

    void start_wifi(String name, String pw, uint maxc);

    IPAddress getIP();

    void start_ws(uint16_t port);
    void start_ws(uint16_t port, uint pingDelta, uint pingWait, uint failsToDisc);
    void loop();

    void addUniversalListener(TES_Event event);
    void addEventListener(String tag, TES_Event event);

    void sendMsg(String group, uint index, String tag, std::vector<String> msg);
    void sendMsg(String group, uint index, String tag, String msg);
    void broadcastMsg(String group, String tag, std::vector<String> msg);
    void broadcastMsg(String group, String tag, String msg);
    void broadcastMsg(String tag, std::vector<String> msg);
    void broadcastMsg(String tag, String msg);

    uint connectedDevices(String group);
    uint connectedDevices();

    void pingDelta(uint pingDelta);
    uint pingDelta();
    void pingWait(uint pingWait);
    uint pingWait();
    void failsToDisc(uint failsToDisc);
    uint failsToDisc();
};

inline void _eventHandler(TES_Server *th, uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
    {
        // TODO
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

        if (tag == "IdEnTiFiEr")
        {
            uint8_t i = th->_cDevices.size();
            th->_cDevices.insert(std::pair<DeviceUID, uint8_t>(DeviceUID{msg, i}, num));
            break;
        }

        auto it = th->_eventList.find(tag);
        if (it != th->_eventList.end())
            it->second(msg);
    }
    break;

    default:
        break;
    }
}

#endif // !TOROESPSOCKETSERVER_H
