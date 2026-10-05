#pragma once

#include "sensor.h"

#include <Arduino.h>

class HcSr04Sensor : public IDistanceSensor
{
public:
    HcSr04Sensor(int trigPin, int echoPin);

    bool begin() override;
    float readCM() override;
    unsigned long readEchoUs();  // New: return raw echo time

private:
    int _trigPin;
    int _echoPin;
};
