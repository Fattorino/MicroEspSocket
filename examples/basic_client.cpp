#include <Arduino.h>
#include <MicroEspSocketClient.h>

TES_Client tesClient("clientGroup");

void onDemoEvent(String msg)
{
    Serial.print("[DEMO] msg ==> ");
    Serial.println(msg);

    tesClient.sendMsg("demoEvent", "Client response!");
}

void setup()
{
    Serial.begin(9600);

    tesClient.connect_to_wifi("apName", "apPassword");
    tesClient.start_ws("192.168.4.1", 81);

    tesClient.addEventListener("demoEvent", onDemoEvent);

    Serial.println("\nStarted!!");
}

void loop()
{
    tesClient.loop();
}
