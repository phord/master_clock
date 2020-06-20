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

/* Telnet server machine */
#include "TelnetServer.h"

/* NTP server machine */
#include "NtpServer.h"

const char* nodename = "clock1";

bool setupNetwork()
{
  static enum {NET_INIT, NET_WAIT, NET_REPORT, NET_FINALIZE, NET_DONE} state = NET_INIT;
  static int i = 0;

  switch(state) {
    case NET_INIT:
      wifiMulti.addAP("wifi", "ffffffffee");
      wifiMulti.addAP("foxland01", "ffffffffee");
      wifiMulti.addAP("foxland02", "ffffffffee");
      state = NET_WAIT;
      break;

    case NET_WAIT:
      if (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
        showActivity(2);
        break;
      }
      state = NET_REPORT;
      break;

    case NET_REPORT:
      Serial.println('\n');
      Serial.print("Connected to ");
      Serial.println(WiFi.SSID());              // Tell us what network we're connected to
      Serial.print("IP address:\t");
      Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer

      if (!MDNS.begin(nodename)) {             // Start the mDNS responder for esp8266.local
        Serial.println("Error setting up MDNS responder!");
      }
      Serial.println("mDNS responder started");
      state = NET_FINALIZE;
      break;

    case NET_FINALIZE:
      NtpSetup() ;
      setupTelnetServer();
      state = NET_DONE;

    case NET_DONE:
    // TODO: Detect if wifi disconnects and start over
      break;
  }

  return state == NET_DONE;
}
