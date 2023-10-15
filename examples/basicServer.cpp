#include <Arduino.h>
#include <ToroEspSocketServer.h>

TES_Server tesServer;

void onDemoEvent(int index, String msg)
{
    Serial.printf("[DEMO] From client %d, got ==> ", index);
    Serial.println(msg);
}

void setup()
{
    Serial.begin(9600);

    uint maxDeviceConnected = 8;
    tesServer.start_wifi("apName", "apPassword", maxDeviceConnected);
    tesServer.start_ws(81);

    tesServer.addEventListener("demoEvent", onDemoEvent);

    Serial.println("\nStarted!!");
}

ulong auxMillis = 0;
void loop()
{
    tesServer.loop();

    if (millis() > auxMillis + 2000)
    {
        auxMillis = millis();
        tesServer.sendMsg("clientGroup", 0, "demoEvent", "You are number ZERO");
        tesServer.printList();
    }
}
