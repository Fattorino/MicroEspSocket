![github-header-image](https://user-images.githubusercontent.com/90210751/209707943-5e7ef05a-122e-4542-8ded-95f59bcb5711.png)


# ToroEspSocket
>**All-in-one web-socket library for the ESP32**.
>
>ToroEspSocket is based on a modified version of [arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets).
>The library manages both client ```TES_Client``` and server ```TES_Server```.

## Features
- Automatic management of connected clients
- Event-based logic
- WiFi and web-socket handling in a single place

## Implementation (In a PlatformIO project)
1. Download the latest .zip from the release page;
2. Copy the *ToroEspSockets* folder into the lib folder of your PlatformIO project
3. ``` #include <ToroEspSocketServer> ``` or/end ``` #include <ToroEspSocketClient> ```

## TES_Server example code
```c++
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
```

## TES_Client example code
```c++
#include <Arduino.h>
#include <ToroEspSocketClient.h>

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
```

## Example code interactions
1. Every five seconds, the server will broadcast a  message with the tag ``` demoEvent ``` to every TES_Client connected;
2. Clients receive the message, the tag ``` demoEvent ``` gets decoded so the related function ``` onDemoEvent ``` gets called;
3. The client sends a message with the tag ``` demoEvent ``` to the server;
4. The server receives the message, the tag ``` demoEvent ``` gets decoded so the related function ``` onDemoEvent ``` gets called;
5. Inside the function ``` onDemoEvent ``` (server side), the content of the message gets printed.

&nbsp;

> ## Recommendations
> To learn more about: the group system used to manage clients, the overall working idea of the library and for a detailed explanation of every function please refer to the wiki

***Wiki coming soon!***
