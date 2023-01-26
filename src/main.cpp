#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WebServer.h>

#include "baby_einstein_six_key.h"
#include "mqtt.h"
#include "secret.h"

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);

volatile uint8_t color_pin = 0; // GPIO pin of color pressed or 0 otherwise

void interrupt_keyboard_press(void* arg) {
  color_pin = *((int*) arg);
}

void mqqt_color_pin_publish(int color_pin) {
  bool sent = mqtt_client.publish(BABY_EINSTEIN_SIX_KEY_TOPIC, std::to_string(color_pin).c_str());
  if (!sent) {
    Serial.print("MQTT ");
    Serial.println(mqtt_client.state());
  } else {
    Serial.println("MQTT sent successfully");
  }
}

void setup() {
  Serial.begin(9600);
  while(!Serial.available()) {}
  Serial.println("Booting");

  pinMode(RED_PIN, INPUT);
  pinMode(ORANGE_PIN, INPUT);
  pinMode(YELLOW_PIN, INPUT);
  pinMode(GREEN_PIN, INPUT);
  pinMode(BLUE_PIN, INPUT);
  pinMode(PURPLE_PIN, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println("connecting to wifi");
  WiFi.begin(WIFISSID, WIFIPASSWORD);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  mqtt_client.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt_client.setBufferSize(255);
  while (!mqtt_client.connect(BABY_EINSTEIN_SIX_KEY_USER, BABY_EINSTEIN_SIX_KEY_USER, BABY_EINSTEIN_SIX_KEY_PASSWORD)) {
    Serial.print("MQTT Connection Failed! Rebooting...");
    Serial.println(mqtt_client.state());
    delay(5000);
    ESP.restart();
  }

  attachInterruptArg(RED_PIN, interrupt_keyboard_press, (void*) &RED_PIN, RISING);
  attachInterruptArg(ORANGE_PIN, interrupt_keyboard_press, (void*) &ORANGE_PIN, RISING);
  attachInterruptArg(YELLOW_PIN, interrupt_keyboard_press, (void*) &YELLOW_PIN, RISING);
  attachInterruptArg(GREEN_PIN, interrupt_keyboard_press, (void*) &GREEN_PIN, RISING);
  attachInterruptArg(BLUE_PIN, interrupt_keyboard_press, (void*) &BLUE_PIN, RISING);
  attachInterruptArg(PURPLE_PIN, interrupt_keyboard_press, (void*) &PURPLE_PIN, RISING);


  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
}


void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("no wifi");
    delay(500);
  }

  if (!mqtt_client.connected() and !mqtt_client.connect(BABY_EINSTEIN_SIX_KEY_USER, BABY_EINSTEIN_SIX_KEY_USER, BABY_EINSTEIN_SIX_KEY_PASSWORD)) {
    Serial.print("no mqqt ");
    Serial.println(mqtt_client.state());
    delay(500);
  }

  if (color_pin > 0) {
    mqqt_color_pin_publish(color_pin);
    color_pin = 0;
    delay(500);
  }
  yield();
}
