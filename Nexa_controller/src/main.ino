#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>
#include "NexaTransmitter.h"
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

#define rfPin D0

#define lightsOnTime 18 // 20, subtract two hours (summer time)
#define lightsOffTime 22 // 24, subtract two hours (summer time)

const char* ssid = "Heia Viking";
const char* password = "jazz346skjagg";
String msg = "";

IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
unsigned int localPort = 2390;      // local port to listen for UDP packets

WiFiUDP udp;


WiFiClient client;
ESP8266WebServer server(80);
NexaTransmitter remote(rfPin,  16619302); //sender 16619302 !group  on recipient 2 (0,1,2)


void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    ESP.restart();
  }


  // Port defaults to 8266
   ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
   ArduinoOTA.setHostname("iot-esp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Device is connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());

  setupWebServer();
}


bool lightson = false;
bool started = false;
long ms = -60000;

/********************************** LOOP *********/

void loop() {

  ArduinoOTA.handle();
  server.handleClient();

  // check time every minute
  if (millis() > ms + 60000) {
    unsigned long myTime = getTrueTime();
    int hour = (myTime  % 86400L) / 3600; //  (86400 equals secs per day)

      // check if lights should be turned on
      if (hour == lightsOnTime && !lightson || !started) {
        remote.setSwitch(true,0);
        remote.setSwitch(true,1);
        remote.setSwitch(true,2);
        lightson = true;
        started = true;
      }
      // check if lights should be turned off
      if (hour == lightsOffTime && lightson || !started) {
        remote.setSwitch(false,0);
        remote.setSwitch(false,1);
        remote.setSwitch(false,2);
        lightson = false;
        started = true;
      }
    ms = millis();
  }

}


/************** Web server routines **************/

void setupWebServer(void){
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);
  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/on", [](){
    server.send(200, "text/plain", "All lights on");
    remote.setSwitch(true,0);
    remote.setSwitch(true,1);
    remote.setSwitch(true,2);
  });
  server.on("/off", [](){
    server.send(200, "text/plain", "All lights off");
    remote.setSwitch(false,0);
    remote.setSwitch(false,1);
    remote.setSwitch(false,2);

  });
  server.on("/blink", [](){
    server.send(200, "text/plain", "All lights off");
    for (int i=0; i<5; i++) {
    remote.setSwitch(true,0);
    remote.setSwitch(true,1);
    remote.setSwitch(true,2);
    delay(200);
    remote.setSwitch(false,0);
    remote.setSwitch(false,1);
    remote.setSwitch(false,2);
    delay(200);
    }
  });
  server.on("/restart", [](){
    server.send(200, "text/plain", "Restarted");
    started = false;
    ms = -100000;
  });
  server.on("/demo", [](){
    server.send(200, "text/plain", "All lights off");
    for (int i=0; i<5; i++) {
    remote.setSwitch(false,1);
    remote.setSwitch(true,0);
    delay(100);
    remote.setSwitch(false,2);
    remote.setSwitch(true,1);
    delay(100);
    remote.setSwitch(false,0);
    remote.setSwitch(true,2);
    delay(100);

    }
  });
  server.on("/time", [](){
    handleGetTime();
  });

  server.on("/switch", [](){

    if (server.hasArg("light") && server.hasArg("value")){
        int light = server.arg("light").toInt();

        if (server.arg("value") == "on") remote.setSwitch(true,light);
        else if (server.arg("value") == "allon") {
          remote.setSwitch(true,0);
          remote.setSwitch(true,1);
          remote.setSwitch(true,2);
        }
        else if (server.arg("value") == "alloff") {
          remote.setSwitch(false,0);
          remote.setSwitch(false,1);
          remote.setSwitch(false,2);
        }
        else remote.setSwitch(false,light);
    }
    msg = "OK";
    handleRoot();

  });

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void handleRoot() {

 String content = "<html><body><form action='/switch' method='POST'>Control lights<br>";
 content += "Light number (0,1,2):<input type='text' name='light' placeholder='0'><br>";
 content += "On or off:<input type='text' name='value' placeholder='off'><br>";
 content += "<input type='submit' name='SUBMIT' value='Submit'></form><br>"+msg;
 content += "</body></html>";
 server.send(200, "text/html", content);
 msg = "";
}

void handleNotFound(){

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

unsigned long getTrueTime() {
  WiFi.hostByName(ntpServerName, timeServerIP);
  unsigned long epoch = 0;
  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  int cb = udp.parsePacket();
  unsigned long ms = millis();
  while (!cb && ms+10000>millis()) {
    Serial.println("no packet yet");
    delay(10);
    cb = udp.parsePacket();
  }
  if (cb) {

    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    epoch = secsSince1900 - seventyYears;
  }

  return epoch;
}

void handleGetTime() {
  unsigned long myTime = getTrueTime();
  int hour = (myTime  % 86400L) / 3600; //  (86400 equals secs per day)

  String message = "Hour = ";
  message += hour;
  server.send(200, "text/plain", message);
}

unsigned long handleGetTime2(){

    String message = "";
    WiFi.hostByName(ntpServerName, timeServerIP);
    unsigned long epoch = 0;

    sendNTPpacket(timeServerIP); // send an NTP packet to a time server
    // wait to see if a reply is available
    int cb = udp.parsePacket();
    unsigned long ms = millis();
    while (!cb && ms+1000>millis()) {
      Serial.println("no packet yet");
      delay(10);
      cb = udp.parsePacket();
    }
    if (!cb) {
        message = "Request to NTP server timed out";
      } else {
      Serial.print("packet received, length=");
      Serial.println(cb);
      // We've received a packet, read the data from it
      udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

      //the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, esxtract the two words:

      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      Serial.print("Seconds since Jan 1 1900 = " );
      Serial.println(secsSince1900);

      // now convert NTP time into everyday time:
      Serial.print("Unix time = ");
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;
      // subtract seventy years:
      epoch = secsSince1900 - seventyYears;
      // print Unix time:
      Serial.println(epoch);
      // print the hour, minute and second:
      message += "The UTC time is ";       // UTC is the time at Greenwich Meridian (GMT)
      message += (epoch  % 86400L) / 3600; // print the hour (86400 equals secs per day)
      message += ':';
      if ( ((epoch % 3600) / 60) < 10 ) {
        // In the first 10 minutes of each hour, we'll want a leading '0'
        message += '0';
      }
      message += (epoch  % 3600) / 60; // print the minute (3600 equals secs per minute)
      message += ':';
      if ( (epoch % 60) < 10 ) {
        // In the first 10 seconds of each minute, we'll want a leading '0'
        message += '0';
      }
      message += epoch % 60; // print the second
    }

    server.send(200, "text/plain", message);
    return epoch;
}


// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}
