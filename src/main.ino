#include <ArduinoOTA.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>

#include "Config.h"

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(PORT);

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_BUILTIN_AUX, OUTPUT);
    strip.begin();
    strip.setBrightness(DEFAULT_BRIGHTNESS);
    Serial.begin(9600);
    delay(10);
    Serial.println('\n');
    wifiMulti.addAP(WIFI_SSID, WIFI_PW);
    Serial.println("Connecting ...");

    while (wifiMulti.run() != WL_CONNECTED)
    {
        delay(250);
        Serial.print('.');
    }
    Serial.println('\n');
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("ledy"))
    {
        Serial.println("mDNS responder started");
    }
    else
    {
        Serial.println("Error setting up MDNS responder!");
    }

    server.on("/", handleRoot);
    server.on("/off", handleOff);
    server.on("/warmwhite", handleWarmWhite);
    server.onNotFound(handleNotFound);
    server.begin();
    ArduinoOTA.onStart([]()
                       {
                           String type;
                           if (ArduinoOTA.getCommand() == U_FLASH)
                           {
                               type = "sketch";
                           }
                           else
                           { // U_FS
                               type = "filesystem";
                           }
                           Serial.println("Start updating " + type);
                       });
    ArduinoOTA.onEnd([]()
                     { Serial.println("\nEnd"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       {
                           Serial.printf("Error[%u]: ", error);
                           if (error == OTA_AUTH_ERROR)
                           {
                               Serial.println("Auth Failed");
                           }
                           else if (error == OTA_BEGIN_ERROR)
                           {
                               Serial.println("Begin Failed");
                           }
                           else if (error == OTA_CONNECT_ERROR)
                           {
                               Serial.println("Connect Failed");
                           }
                           else if (error == OTA_RECEIVE_ERROR)
                           {
                               Serial.println("Receive Failed");
                           }
                           else if (error == OTA_END_ERROR)
                           {
                               Serial.println("End Failed");
                           }
                       });
    ArduinoOTA.begin();
}

void loop(void)
{
    server.handleClient();
    ArduinoOTA.handle();
}

void handleOff()
{
    server.send(200, "application/json", "{\"mode\": \"off\"}");
    strip.setBrightness(0);
    strip.show();
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(LED_BUILTIN_AUX, HIGH);
}

void handleWarmWhite()
{
    setDefaultHeaders();
    server.send(200, "application/json", "{\"mode\": \"warmwhite\"}");
    strip.setBrightness(DEFAULT_BRIGHTNESS);
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, 255, 150, 100);
    }
    strip.show();
    digitalWrite(LED_BUILTIN, LOW);
}

void handleRoot()
{
    setDefaultHeaders();
    server.send(200, "application/json", "{\"mode\": \"root\"}");
    strip.setBrightness(DEFAULT_BRIGHTNESS);

    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, 255, 150, 100);
    }

    strip.show();
    digitalWrite(LED_BUILTIN_AUX, LOW);
}

void handleNotFound(){
  server.send(404, "application/json", "{\"mode\": \"notFound\"}");
}

void setDefaultHeaders()
{
    server.sendHeader("Access-Control-Allow-Origin", "http://localhost:8100");
}