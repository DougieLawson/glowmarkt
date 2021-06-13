//
// Copyright © Dougie Lawson 2020, All rights reserved.
//

#include "FastLED.h"
#include <pixeltypes.h>
#define MQTT_KEEPALIVE 240
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN     2
#define NUM_LEDS    3
#define BRIGHTNESS  255
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
WiFiClient wemosCL;
PubSubClient mqttClient(wemosCL);

String clientId;

const char* MQTT_broker = "192.168.3.14";
const char* MQTT_topic = "meterReading/out/elec";

const int led = LED_BUILTIN;

const CRGB colourArray[] = {
  0x00ff00, 0x2eff00,
  0x62ff00, 0x96ff00,
  0xd0ff00, 0xffff00,
  0xf5ca04, 0xea9208,
  0xe2670b, 0xda3c0e,
  0xa20707, 0x000000,
};
const CRGB initial[] = {0x0F0F0F,};

void mqttCallBack(char* topic, byte* payload, unsigned int plLen)
{
  String mqttJSON = String((char*)payload);
  StaticJsonDocument<96> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, mqttJSON);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  float meter = jsonDoc["Meter"];
  const char* timestamp = jsonDoc["Timestamp"];

  float powerPercent = (meter / 3.5) * 100;
  if (powerPercent >= 100) powerPercent = 100;
  int index = int(powerPercent / 10);
  if (index >= 11) index = 11;

  for (int indexO = NUM_LEDS; indexO > 0; indexO--) {
    leds[indexO] = leds[indexO - 1];
    FastLED.show();
  }
  leds[0] = colourArray[index];
  FastLED.show();
}

void reconnect()
{
  while (mqttClient.state() != MQTT_CONNECTED)
  {
    Serial.println("MQTT client not connected");
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("MQTT subscribe");
      mqttClient.subscribe(MQTT_topic);
    } else
    {
      delay(1000);
    }
  }
}

void setup()
{
  clientId = "WemosMQTTclient";
  clientId += String(random(0xffff), HEX);
  Serial.begin(115200);
  while (!Serial);
  WiFiManager wifiManager;
  wifiManager.setBreakAfterConfig(true);
  //wifiManager.resetSettings();
  if (!wifiManager.autoConnect("configure-me", "configuration"))
  {
    delay(100);
    digitalWrite(led, 1);
    //ESP.reset();
    delay(100);
    digitalWrite(led, 0);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  mqttClient.setServer(MQTT_broker, 1883);
  mqttClient.setCallback(mqttCallBack);
  delay(3000); // 3 second delay for recovery
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  for (int indexO = NUM_LEDS; indexO >= 0; indexO--)
  {
    leds[indexO] = initial[0];
    FastLED.show();
  }
}

void loop()
{
  if (mqttClient.state() != MQTT_CONNECTED)
  {
    Serial.println("Call to reconnect()");
    reconnect();
  }

  mqttClient.loop();
  delay(10);
}
