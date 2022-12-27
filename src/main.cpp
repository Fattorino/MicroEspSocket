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

	if (millis() > auxMillis + 5000) // Sending demo messages every five seconds
	{
		auxMillis = millis();
		tesServer.broadcastMsg("demoEvent", "Demo message to all clients");
	}
}

// =============================================

// #include <Arduino.h>
// #include <ToroEspSocketClient.h>

// TES_Client tesClient("clientGroup");

// void onDemoEvent(String msg)
// {
// 	Serial.print("[DEMO] msg ==> ");
// 	Serial.println(msg);

// 	tesClient.sendMsg("demoEvent", "Client response!");
// }

// void setup()
// {
// 	Serial.begin(9600);

// 	tesClient.connect_to_wifi("apName", "apPassword");
// 	tesClient.start_ws("192.168.4.1", 81);

// 	tesClient.addEventListener("demoEvent", onDemoEvent);

// 	Serial.println("\nStarted!!");
// }

// void loop()
// {
// 	tesClient.loop();
// }
