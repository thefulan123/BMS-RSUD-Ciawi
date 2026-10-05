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

// Baca lengkap: echo → jarak (kompensasi suhu) → volume → level (%).
// Pipeline: HC-SR04 → raw echo → temperature compensation → calibration → reading
SensorReading HcSr04Sensor::read()
{
    SensorReading r = {0.0f, 0.0f, 0.0f, false};

    unsigned long echoUs = readEchoUs();

    if (echoUs == 0)
    {
        return r;  // valid = false
    }

    // 1. Echo time → jarak, dengan kompensasi suhu (v = 331.3 + 0.606*T)
    r.distance_cm = Calibration::echoUsToDistance(echoUs, TEMPERATURE_C);

    // 2. Jarak → volume, via piecewise linear interpolation tabel kalibrasi
    r.volume_ml = Calibration::distanceToVolume(r.distance_cm);

    // 3. Volume → level persen
    r.level_percent = Calibration::volumeToPercent(r.volume_ml);

    r.valid = true;
    return r;
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
            // Pakai kompensasi suhu dari calibration.h, bukan konstanta hardcoded.
            samples[count++] = Calibration::echoUsToDistance(duration, TEMPERATURE_C);
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
