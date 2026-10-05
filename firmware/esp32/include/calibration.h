#pragma once

#include <Arduino.h>

// ============================================================
// KALIBRASI TANGKI & SENSOR — LINEAR GLOBAL
// --------------------------------
// Cukup ubah 3 ANGKA di bawah. Persamaan dihitung otomatis.
//
//   Pindah tangki (GWT-001 → GWT-002) → ubah 3 angka ini
//   Ganti sensor / mounting            → ubah 3 angka ini
//
// Persamaan:   V = m·d + b
//   m (gradien) = (0 - MAX_VOLUME) / (MAX_DISTANCE - MIN_DISTANCE)
//   b (offset)  = MAX_VOLUME - m·MIN_DISTANCE
//
// Titik jangkauan:
//   d = CALIBRATION_MIN_DISTANCE → V = MAX_VOLUME (penuh)
//   d = CALIBRATION_MAX_DISTANCE → V = 0          (kosong)
// ============================================================

// --- Suhu ruang (kompensasi kecepatan suara) ---
constexpr float TEMPERATURE_C = 16.0f;

// --- Parameter kalibrasi tangki (UBAH DI SINI SAJA) ---
constexpr float CALIBRATION_MAX_VOLUME   = 1000.0f;  // volume penuh (ml)
constexpr float CALIBRATION_MIN_DISTANCE = 4.19f;    // jarak saat penuh (cm)
constexpr float CALIBRATION_MAX_DISTANCE = 16.02f;   // jarak saat kosong (cm)

// --- Persamaan linear (dihitung otomatis dari parameter di atas) ---
// V = CALIBRATION_SLOPE * d + CALIBRATION_INTERCEPT
constexpr float CALIBRATION_SLOPE =
    (0.0f - CALIBRATION_MAX_VOLUME) /
    (CALIBRATION_MAX_DISTANCE - CALIBRATION_MIN_DISTANCE);

constexpr float CALIBRATION_INTERCEPT =
    CALIBRATION_MAX_VOLUME - (CALIBRATION_SLOPE * CALIBRATION_MIN_DISTANCE);

// --- Fungsi helper ---
namespace Calibration {

    // Echo time (µs) → jarak (cm), dengan kompensasi suhu
    inline float echoUsToDistance(unsigned long echoUs, float temperatureC) {
        float speed = 331.3f + (0.606f * temperatureC); // m/s
        return echoUs * speed / 20000.0f;               // cm
    }

    // Jarak (cm) → volume (ml): SATU persamaan linear, di-clamp 0..MAX_VOLUME
    inline float distanceToVolume(float distanceCm) {
        float volume = (CALIBRATION_SLOPE * distanceCm) + CALIBRATION_INTERCEPT;
        return constrain(volume, 0.0f, CALIBRATION_MAX_VOLUME);
    }

    // Volume (ml) → level (%): di-clamp 0..100
    inline float volumeToPercent(float volumeMl) {
        float percent = (volumeMl / CALIBRATION_MAX_VOLUME) * 100.0f;
        return constrain(percent, 0.0f, 100.0f);
    }
}
