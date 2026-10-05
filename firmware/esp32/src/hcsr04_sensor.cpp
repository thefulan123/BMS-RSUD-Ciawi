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

// Echo time mentah dalam mikrodetik (µs). 0 bila timeout.
// Dipakai untuk debug / kalibrasi manual.
unsigned long HcSr04Sensor::readEchoUs()
{
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(_trigPin, LOW);

    // Timeout 30ms = ~5 meter jangkauan maks
    return pulseIn(_echoPin, HIGH, 30000);
}

// Baca lengkap: echo → jarak (kompensasi suhu) → volume → level (%).
// Pipeline: HC-SR04 → raw echo → temperature compensation → calibration → reading
SensorReading HcSr04Sensor::read()
{
    SensorReading r = {0.0f, 0.0f, 0.0f, 0.0f, false};

    unsigned long echoUs = readEchoUs();

    if (echoUs == 0)
    {
        return r;  // valid = false
    }

    // 1. Echo time → jarak, dengan kompensasi suhu (v = 331.3 + 0.606*T)
    r.distance_cm = Calibration::echoUsToDistance(echoUs, TEMPERATURE_C);

    // 2. Jarak → volume, natural cubic spline (halus, lewat tepat tiap titik)
    r.volume_ml = Calibration::distanceToVolume(r.distance_cm);

    // 3. Volume → level persen
    r.level_percent = Calibration::volumeToPercent(r.volume_ml);

    // 4. Diferensial (gradien dV/dd) di titik ini
    r.differential = Calibration::distanceToSlope(r.distance_cm);

    r.valid = true;
    return r;
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
            // Pakai kompensasi suhu dari calibration.h, bukan konstanta hardcoded.
            samples[count++] = Calibration::echoUsToDistance(duration, TEMPERATURE_C);
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
