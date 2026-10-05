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

    // Ukur jarak (cm). -1 bila echo timeout.
    float readCM() override;

private:
    int _trigPin;
    int _echoPin;
};
