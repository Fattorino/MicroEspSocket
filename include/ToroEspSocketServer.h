#ifndef TOROESPSOCKETSERVER_H
#define TOROESPSOCKETSERVER_H

#include <Arduino.h>
#include <vector>
#include <map>

#include <WiFi.h>
#include <esp_wifi.h>
#include <WebSocketsServer.h>

typedef std::function<void(int index, String msg)> TES_SEvent;

struct DeviceUID
{
    String group;
    uint index;

    bool operator=(const DeviceUID &cmp) const
    {
        return group == cmp.group;
    }

    bool operator<(const DeviceUID &cmp) const
    {
        for (int i = 0; i < group.length(); i++)
        {
            if (group[i] != cmp.group[i])
                return group[i] < cmp.group[i];
        }
        return index < cmp.index;
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
    TES_SEvent _connectEvent;
    bool _connectEventToggle = false;
    TES_SEvent _disconnectEvent;
    bool _disconnectEventToggle = false;
    TES_SEvent _identifyEvent;
    bool _identifyEventToggle = false;

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
    void addOnConnectListener(TES_SEvent event);
    void addOnDisconnectListener(TES_SEvent event);
    void addOnIdentificationListener(TES_SEvent event);

    void sendMsg(String group, uint index, String tag, std::vector<String> msg);
    void sendMsg(String group, uint index, String tag, String msg);
    void broadcastMsg(String group, String tag, std::vector<String> msg);
    void broadcastMsg(String group, String tag, String msg);
    void broadcastMsg(String tag, std::vector<String> msg);
    void broadcastMsg(String tag, String msg);

    void regroupDevice(DeviceUID device, String newGroup);

    IPAddress getIP();
    uint getConnectedDevices(String group);
    uint getConnectedDevices();
    void setPingDelta(uint pingDelta);
    uint getPingDelta();
    void setPingWait(uint pingWait);
    uint getPingWait();
    void setFailsToDisc(uint failsToDisc);
    uint getFailsToDisc();

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
        for (it = th->_cDevices.begin(); it != th->_cDevices.end(); it++)
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

        if (th->_disconnectEventToggle)
        {
            th->_disconnectEvent((int)dIndex, dGroup);
        }
    }
    break;

    case WStype_CONNECTED:
    {
        if (th->_connectEventToggle)
        {
            th->_connectEvent((int)num, String(payload, length));
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
            uint nIndex = th->getConnectedDevices(msg);
            th->_cDevices.insert(std::pair<DeviceUID, uint8_t>(DeviceUID{msg, nIndex}, num));
            th->_ws->sendTXT(num, "InDeX=" + String(nIndex));

            if (th->_identifyEventToggle)
            {
                th->_identifyEvent(nIndex, msg);
            }

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
