#include "hcsr04_sensor.h"

// Constructor: simpan nomor pin.
HcSr04Sensor::HcSr04Sensor(int trigPin, int echoPin)
    : _trigPin(trigPin),
      _echoPin(echoPin)
{
}

// Cara pakai: panggil sekali di setup().
bool HcSr04Sensor::begin()
{
    pinMode(_trigPin, OUTPUT);
    pinMode(_echoPin, INPUT);
    digitalWrite(_trigPin, LOW);
    return true;
}

// Cara pakai: panggil untuk membaca jarak. Kembalikan -1 bila timeout.
// Ambil 3 sampel lalu rata-rata supaya lebih stabil (noise berkurang).
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

        delay(50);  // jeda antar sampel
    }

    if (count == 0)
    {
        return -1;  // Semua sampel timeout
    }

    float sum = 0;
    for (int i = 0; i < count; i++)
    {
        sum += samples[i];
    }
    return sum / count;
}
