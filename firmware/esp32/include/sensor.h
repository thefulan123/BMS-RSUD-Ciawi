#pragma once

#include <Arduino.h>

/*
 * Kontrak untuk sensor jarak (distance). Memudahkan ganti hardware
 * (misal ultrasonic lain / ToF) tanpa menyentuh App.
 * Cara pakai: buat class baru dari IDistanceSensor, lalu inject ke App.
 */

// Hasil bacaan lengkap: jarak, volume (hasil kalibrasi), dan level (%).
struct SensorReading
{
    float distance_cm;     // Jarak sensor → permukaan air
    float volume_ml;       // PERSAMAAN A (regresi linear LS)
    float level_percent;   // Level % dari persamaan A
    float volume_b_ml;     // PERSAMAAN B (jangkar titik 0 ml)
    float level_b_percent; // Level % dari persamaan B
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
