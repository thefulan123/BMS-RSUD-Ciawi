#include "hcsr04_sensor.h"

HcSr04Sensor::HcSr04Sensor(int trigPin, int echoPin)
    : _trigPin(trigPin),
      _echoPin(echoPin)
{
}

bool HcSr04Sensor::begin()
{
    pinMode(_trigPin, OUTPUT);
    pinMode(_echoPin, INPUT);
    digitalWrite(_trigPin, LOW);
    return true;
}

// v1.2: Return raw echo time in microseconds (for Node-RED conversion)
unsigned long HcSr04Sensor::readEchoUs()
{
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(_trigPin, LOW);

    // Return raw echo duration in microseconds (timeout 30ms = 30000µs)
    return pulseIn(_echoPin, HIGH, 30000);
}

// Legacy: keep for backward compatibility
float HcSr04Sensor::readCM()
{
    float samples[3];
    int count = 0;

    for (int i = 0; i < 3; i++)
    {
        digitalWrite(_trigPin, LOW);
        delayMicroseconds(2);

        digitalWrite(_trigPin, HIGH);
        delayMicroseconds(10);

        digitalWrite(_trigPin, LOW);

        long duration = pulseIn(_echoPin, HIGH, 30000);

        if (duration > 0)
        {
            samples[count++] = duration * 0.0343 / 2.0;
        }

        delay(50);
    }

    if (count == 0)
    {
        return -1;
    }

    float sum = 0;
    for (int i = 0; i < count; i++)
    {
        sum += samples[i];
    }
    return sum / count;
}
