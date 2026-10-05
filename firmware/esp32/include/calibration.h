#pragma once

#include <Arduino.h>

// ============================================================
// KALIBRASI TANGKI & SENSOR
// --------------------------------
// File ini Sering diubah saat:
//   - Pindah tangki (GWT-001 → GWT-002)
//   - Ganti sensor
//   - Perubahan mounting
//
// Cara pakai:
//   1. Ukur jarak sensor → permukaan air (cm) untuk tiap volume
//   2. Masukkan ke CALIBRATION_TABLE
//   3. Firmware otomatis hitung volume & level
// ============================================================

// --- Konstanta Sensor ---
constexpr float TEMPERATURE_C = 16.0f;           // Suhu ruang (ganti bila perlu / nanti pakai sensor suhu)
constexpr float SOUND_SPEED_MPS = 341.0f;        // m/s pada 16°C
constexpr float SOUND_SPEED_CM_US = 0.0341f;     // cm/µs

// --- Kalibrasi Tangki ---
struct CalibrationPoint {
    float distance_cm;
    float volume_ml;
};

// Tabel kalibrasi: jarak sensor → volume air
// (diisi sesuai pengukuran tiap tangki)
constexpr CalibrationPoint CALIBRATION_TABLE[] = {
    {4.19f,  1000.0f},
    {5.625f,  900.0f},
    {6.10f,   800.0f},
    {8.66f,   600.0f},
    {10.00f,  500.0f},
    {11.34f,  400.0f},
    {12.66f,  300.0f},
    {14.00f,  200.0f},
    {15.35f,  100.0f},
    {16.02f,    0.0f}
};

constexpr size_t CALIBRATION_SIZE = sizeof(CALIBRATION_TABLE) / sizeof(CALIBRATION_TABLE[0]);

// --- Fungsi helper ---
namespace Calibration {

    // Convert echo time (µs) → distance (cm) dengan kompensasi suhu
    inline float echoUsToDistance(unsigned long echoUs, float temperatureC) {
        float speed = 331.3f + (0.606f * temperatureC); // m/s
        return echoUs * speed / 20000.0f;               // cm
    }

    // Convert distance (cm) → volume (ml) via piecewise linear interpolation
    inline float distanceToVolume(float distanceCm) {
        if (distanceCm >= CALIBRATION_TABLE[CALIBRATION_SIZE - 1].distance_cm) {
            return 0.0f;
        }
        if (distanceCm <= CALIBRATION_TABLE[0].distance_cm) {
            return CALIBRATION_TABLE[0].volume_ml;
        }

        for (size_t i = 0; i < CALIBRATION_SIZE - 1; i++) {
            float dHigh = CALIBRATION_TABLE[i].distance_cm;
            float dLow  = CALIBRATION_TABLE[i + 1].distance_cm;
            float vHigh = CALIBRATION_TABLE[i].volume_ml;
            float vLow  = CALIBRATION_TABLE[i + 1].volume_ml;

            if (distanceCm <= dHigh && distanceCm >= dLow) {
                float ratio = (dHigh - distanceCm) / (dHigh - dLow);
                return vLow + ratio * (vHigh - vLow);
            }
        }
        return 0.0f;
    }

    // Hitung level persen
    inline float volumeToPercent(float volumeMl) {
        float maxVol = CALIBRATION_TABLE[0].volume_ml;
        return (volumeMl / maxVol) * 100.0f;
    }
}
