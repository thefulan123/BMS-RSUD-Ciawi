/*
 * BMS RSUD Ciawi - Firmware v1.1
 * --------------------------------
 * ESP32 + HC-SR04 → MQTT (JSON distance)
 *
 * Output: {"distance_cm": 14.63}
 * Topic:  bms/gwt1
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
    Serial.println("ESP32 BMS MQTT v1.1");
    Serial.println("Output: JSON distance (cm)");
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

        float distanceCM = sensor.readCM();

        if (distanceCM < 0)
        {
            Serial.println("HC-SR04 timeout!");
            return;
        }

        // Build JSON: {"distance_cm": 14.63}
        String payload = "{\"distance_cm\":";
        payload += String(distanceCM, 2);
        payload += "}";

        if (mqtt.publish(payload.c_str()))
        {
            Serial.print("Distance: ");
            Serial.print(distanceCM, 2);
            Serial.println(" cm");
        }
        else
        {
            Serial.println("MQTT -> GAGAL kirim");
        }
    }
}
