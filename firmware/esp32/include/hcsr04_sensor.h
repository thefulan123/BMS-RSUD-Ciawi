#pragma once

#include "sensor.h"

#include <Arduino.h>

/*
 * Implementasi IDistanceSensor untuk sensor ultrasonic HC-SR04.
 * Cara pakai: buat objek dengan pin TRIG dan ECHO, lalu begin() + readCM().
 */
class HcSr04Sensor : public IDistanceSensor
{
public:
    // Constructor: pin trigger dan echo dari ESP32.
    HcSr04Sensor(int trigPin, int echoPin);

    // Setiap pin sebagai output/input.
    bool begin() override;

    // v1.2: Return raw echo time in microseconds (for Node-RED conversion)
    unsigned long readEchoUs();

    // Legacy: keep for backward compatibility
    float readCM() override;

private:
    int _trigPin;
    int _echoPin;
};
