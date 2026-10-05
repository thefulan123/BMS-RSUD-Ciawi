#pragma once

#include <Arduino.h>

/*
 * Kontrak untuk sensor jarak (distance). Memudahkan ganti hardware
 * (misal ultrasonic lain / ToF) tanpa menyentuh App.
 * Cara pakai: buat class baru dari IDistanceSensor, lalu inject ke App.
 */
class IDistanceSensor
{
public:
    virtual ~IDistanceSensor() = default;

    // Siapkan pin/hardware. Dipanggil sekali di setup().
    virtual bool begin() = 0;

    // Baca jarak dalam cm. Kembalikan -1 bila gagal/timeout.
    virtual float readCM() = 0;
};
