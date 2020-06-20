#include <SPI.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFiMulti.h>   // Include the Wi-Fi-Multi library
#include <ESP8266mDNS.h>        // Include the mDNS library

ESP8266WiFiMulti wifiMulti;

#include "clock_generic.h"

#include <ESP8266mDNS.h>

const char* nodename = "clock1";

bool setupNetwork()
{
  wifiMulti.addAP("wifi", "ffffffffee");
  wifiMulti.addAP("foxland01", "ffffffffee");
  wifiMulti.addAP("foxland02", "ffffffffee");

  int i = 0;
  // FIXME: Make this a state machine so we don't hang while starting.
  // Will need to move other network service startups into here
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer

  if (!MDNS.begin(nodename)) {             // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");

  return true;
}
