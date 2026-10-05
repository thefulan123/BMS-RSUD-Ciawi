/*
 * BMS RSUD Ciawi - Firmware v1.2
 * --------------------------------
 * ESP32 + HC-SR04 → MQTT (raw echo time in µs)
 *
 * Output: {"echo_us": 880}
 * Topic:  bms/gwt1
 *
 * Node-RED akan convert: echo_us → distance_cm → volume_ml
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include "config.h"
#include "config_private.h"
#include "wifi_connector.h"
#include "mqtt_connector.h"
#include "hcsr04_sensor.h"

const DeviceSettings DEVICE;
const MqttSecrets SECRETS;

WiFiManagerConnector wifi(DEVICE.wifi);
PubSubMqttConnector mqtt(DEVICE.mqtt, SECRETS);
HcSr04Sensor sensor(DEVICE.sensor.trigPin, DEVICE.sensor.echoPin);

void setup()
{
    Serial.begin(DEVICE.serialBaud);
    delay(1000);

    Serial.println();
    Serial.println("==============================");
    Serial.println("ESP32 BMS MQTT v1.2");
    Serial.println("Output: raw echo time (µs)");
    Serial.println("==============================");

    sensor.begin();

    if (!wifi.connect())
    {
        Serial.println("WiFi gagal, restart...");
        ESP.restart();
    }

    mqtt.begin();

    if (!mqtt.ensureConnected())
    {
        Serial.println("MQTT belum terhubung, akan dicoba berkala di loop()");
    }
}

void loop()
{
    if (!wifi.isConnected())
    {
        Serial.println("WiFi putus, mencoba reconnect...");
        wifi.reconnect();
    }

    mqtt.ensureConnected();
    mqtt.loop();

    if (millis() - lastPublish >= DEVICE.publishIntervalMs)
    {
        lastPublish = millis();

        // Read raw echo time in microseconds
        unsigned long echoUs = sensor.readEchoUs();

        if (echoUs == 0)
        {
            Serial.println("HC-SR04 timeout!");
            return;
        }

        // Build JSON: {"echo_us": 880}
        String payload = "{\"echo_us\":";
        payload += String(echoUs);
        payload += "}";

        if (mqtt.publish(payload.c_str()))
        {
            Serial.print("Echo: ");
            Serial.print(echoUs);
            Serial.println(" µs");
        }
        else
        {
            Serial.println("MQTT -> GAGAL kirim");
        }
    }
}
