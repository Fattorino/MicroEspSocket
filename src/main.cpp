// #include <Arduino.h>
// #include <ToroEspSocketServer.h>

// TES_Server tesServer;

// void onMsg(uint index, String msg)
// {
//   Serial.printf("From client identifying with index: %d, got ==> %s\n", index, msg);
// }

// void onDemoEvent(uint index, String msg)
// {
//   Serial.printf("Client %d sayed: \"%s\"\n", index, msg);
//   tesServer.sendMsg("clientGroup", index, "msgTag", "demo server response");
// }

// void setup()
// {
//   Serial.begin(9600);

//   uint maxDeviceConnected = 8;
//   tesServer.start_wifi("apName", "apPassword", maxDeviceConnected);
//   tesServer.start_ws(81);

//   tesServer.addUniversalListener(onMsg);
//   tesServer.addEventListener("eventTag", onDemoEvent);
// }

// void loop()
// {
//   tesServer.loop();
// }

// =============================================

#include <Arduino.h>
#include <ToroEspSocketClient.h>

TES_Client tesClient("deviceGroup");

void onEvent(String msg)
{
	Serial.printf("Got from server ==> %s\n", msg);
}

void onDemoEvent(String msg)
{
	Serial.printf("Demo event got activated, msg ==> %s\n", msg);
}

void setup()
{
	Serial.begin(9600);

	tesClient.connect_to_wifi("apName", "apPassword");
	tesClient.start_ws("192.168.4.1", 81);

	tesClient.addUniversalListener(onEvent);
	tesClient.addEventListener("msgTag", onDemoEvent);
}

ulong auxMillis = 0;
void loop()
{
	tesClient.loop();

	if (millis() > auxMillis + 2000) // Sending demo messages every two seconds
	{
		auxMillis = millis();
		tesClient.sendMsg("eventTag", "Demo client message");
	}
}