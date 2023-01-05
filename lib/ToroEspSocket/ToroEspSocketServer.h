#ifndef TOROESPSOCKETSERVER_H
#define TOROESPSOCKETSERVER_H

#include <Arduino.h>
#include <vector>
#include <map>

#include <WiFi.h>
#include <esp_wifi.h>
#include "WebSocketBase/WebSocketsServer.h"

typedef std::function<void(int index, String msg)> TES_SEvent;

struct DeviceUID
{
    String group;
    uint index;

    bool operator<(const DeviceUID &cmp) const
    {
        return (group.charAt(0) * 100 + group.charAt(1) * 10 + index) < (cmp.group.charAt(0) * 100 + cmp.group.charAt(1) * 10 + cmp.index);
    }

    void setIndex(uint nIndex) { index = nIndex; }
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

    std::map<String, TES_SEvent> _eventList;
    TES_SEvent _universalEvent;
    bool _universalEventToggle = false;

    std::map<DeviceUID, uint8_t> _cDevices;

    int _decodeIndex(uint8_t *payload, size_t length);
    String _decodeTag(uint8_t *payload, size_t length);
    String _decodeMsg(uint8_t *payload, size_t length);

public:
    friend void _eventHandler(TES_Server *th, uint8_t num, WStype_t type, uint8_t *payload, size_t length);

    TES_Server() {}
    ~TES_Server() { delete _ws; }

    void start_wifi(String ssid, String pw, uint maxc);
    void start_ws(uint16_t port);
    void start_ws(uint16_t port, uint pingDelta, uint pingWait, uint failsToDisc);
    void loop();

    void addUniversalListener(TES_SEvent event);
    void addEventListener(String tag, TES_SEvent event);

    void sendMsg(String group, uint index, String tag, std::vector<String> msg);
    void sendMsg(String group, uint index, String tag, String msg);
    void broadcastMsg(String group, String tag, std::vector<String> msg);
    void broadcastMsg(String group, String tag, String msg);
    void broadcastMsg(String tag, std::vector<String> msg);
    void broadcastMsg(String tag, String msg);

    void regroupDevice(DeviceUID device, String newGroup);

    IPAddress getIP();
    uint connectedDevices(String group);
    uint connectedDevices();
    void pingDelta(uint pingDelta);
    uint pingDelta();
    void pingWait(uint pingWait);
    uint pingWait();
    void failsToDisc(uint failsToDisc);
    uint failsToDisc();

    void printList();
};

inline void _eventHandler(TES_Server *th, uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
    {
        auto it = th->_cDevices.begin();
        for (; it != th->_cDevices.end() && it->second != num; it++)
        {
        }
        if (it == th->_cDevices.end())
            break;

        String dGroup = it->first.group;
        uint dIndex = it->first.index;
        std::vector<std::map<DeviceUID, uint8_t>::iterator> toDelete;
        std::vector<uint8_t> toReAddNums;

        toDelete.push_back(it);
        it = th->_cDevices.begin();
        for (; it != th->_cDevices.end(); it++)
        {
            if (it->first.group == dGroup && it->first.index > dIndex)
            {
                toDelete.push_back(it);
                toReAddNums.push_back(it->second);
            }
        }
        for (std::map<DeviceUID, uint8_t>::iterator &dIt : toDelete)
        {
            th->_cDevices.erase(dIt);
        }

        for (int i = 0; i < toReAddNums.size(); i++)
        {
            th->_cDevices.insert(std::pair<DeviceUID, uint8_t>(DeviceUID{dGroup, dIndex + i}, toReAddNums[i]));
            th->_ws->sendTXT(toReAddNums[i], "InDeX=" + String(dIndex + i));
        }
    }
    break;

    case WStype_TEXT:
    {
        String tag = th->_decodeTag(payload, length);
        String msg = th->_decodeMsg(payload, length);
        int index = th->_decodeIndex(payload, length);

        if (th->_universalEventToggle)
        {
            th->_universalEvent(index, msg);
        }

        if (tag == "IdEnTiFiEr")
        {
            uint index = th->connectedDevices(msg);
            th->_cDevices.insert(std::pair<DeviceUID, uint8_t>(DeviceUID{msg, index}, num));
            th->_ws->sendTXT(num, "InDeX=" + String(index));
            break;
        }

        auto it = th->_eventList.find(tag);
        if (it != th->_eventList.end())
            it->second(index, msg);
    }
    break;

    default:
        break;
    }
}

#endif // !TOROESPSOCKETSERVER_H
