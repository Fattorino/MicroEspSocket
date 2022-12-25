#include <Arduino.h>
#include <ToroEspSocketServer.h>

TES_Server test;

// TODO: In WebSocketServer.h and .cpp ==> Make MaxClientConnections form constant to variable

void testEvent(String msg)
{
  Serial.printf("Got ==> %s\n", msg);
}

void whenGreeted(String msg)
{
  Serial.printf("Client sayed: \"%s\" to me!!\n", msg);
}

void setup()
{
  Serial.begin(9600);

  test.start_wifi("TEST", "00000000", 8);
  test.start_ws(81, 7000, 3000, 1);

  test.addUniversalListener(testEvent);
  test.addEventListener("grt", whenGreeted);
}

void loop()
{
  test.loop();
  if (millis() % 1000 == 0)
    test.sendMsg("tag", 0, "color", "red");
}