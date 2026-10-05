#pragma once

#include <Arduino.h>

/*
 * Kontrak untuk sensor jarak (distance). Memudahkan ganti hardware
 * (misal ultrasonic lain / ToF) tanpa menyentuh App.
 * Cara pakai: buat class baru dari IDistanceSensor, lalu inject ke App.
 */

// Hasil bacaan lengkap: jarak, volume (hasil kalibrasi), level (%),
// dan diferensial (gradien dV/dd — seberapa cepat volume berubah per cm).
struct SensorReading
{
    float distance_cm;     // Jarak sensor → permukaan air
    float volume_ml;       // Volume air hasil spline kalibrasi
    float level_percent;   // Persen isi tangki (0-100)
    float differential;    // Gradien dV/dd (ml per cm) di titik ini
    bool valid;            // false bila timeout/sensor gagal
};

class IDistanceSensor
{
public:
    virtual ~IDistanceSensor() = default;

    // Siapkan pin/hardware. Dipanggil sekali di setup().
    virtual bool begin() = 0;

    // Baca jarak dalam cm. Kembalikan -1 bila gagal/timeout.
    virtual float readCM() = 0;

    // Baca echo time mentah dalam mikrodetik (µs). 0 bila timeout.
    virtual unsigned long readEchoUs() = 0;

    // Baca lengkap: echo → jarak (kompensasi suhu) → volume → level.
    virtual SensorReading read() = 0;
};
