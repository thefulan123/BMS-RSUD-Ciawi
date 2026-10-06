#pragma once

#include <Arduino.h>

// ============================================================
// KALIBRASI TANGKI & SENSOR — PIECEWISE LINEAR
// --------------------------------
// ⚠️  STATUS: TABEL DI BAWAH BELUM DIKALIBRASI ULANG
//     (nilai lama, dipertahankan sebagai referensi awal)
//
//     Sementara firmware HANYA mengirim distance_cm.
//     Volume & level dimatikan sampai kalibrasi ulang selesai.
//
// Cara kalibrasi ulang:
//   1. Isi tangki bertahap, catat jarak dari serial monitor
//   2. Masukkan titik baru urut jarak NAIK
//   3. Nyalakan lagi distanceToVolume() di hcsr04_sensor.cpp
//   4. Nyalakan lagi volume_ml & level_percent di app.cpp
// ============================================================

// --- Suhu ruang (kompensasi kecepatan suara) ---
constexpr float TEMPERATURE_C = 16.0f;

// --- Titik kalibrasi ---
// PENTING: urut berdasarkan jarak NAIK (4.19 → 16.02)
//          sehingga volume MENURUN (1000 → 0)
struct CalibrationPoint {
    float distance_cm;
    float volume_ml;
};

constexpr CalibrationPoint CALIBRATION_TABLE[] = {
    { 4.19f, 1000.0f},
    { 5.625f, 900.0f},
    { 6.10f,  800.0f},
    { 8.66f,  600.0f},
    {10.00f,  500.0f},
    {11.34f,  400.0f},
    {12.66f,  300.0f},
    {14.00f,  200.0f},
    {15.35f,  100.0f},
    {16.02f,    0.0f}
};

constexpr size_t CALIBRATION_SIZE =
    sizeof(CALIBRATION_TABLE) / sizeof(CALIBRATION_TABLE[0]);

constexpr float CALIBRATION_MAX_VOLUME = CALIBRATION_TABLE[0].volume_ml;

// --- Fungsi helper ---
namespace Calibration {

    // Echo time (µs) → jarak (cm), dengan kompensasi suhu
    inline float echoUsToDistance(unsigned long echoUs, float temperatureC) {
        float speed = 331.3f + (0.606f * temperatureC); // m/s
        return echoUs * speed / 20000.0f;               // cm
    }

    // Jarak (cm) → volume (ml) via piecewise linear interpolation.
    //
    // Cari 2 titik kalibrasi yang mengapit distanceCm, lalu pakai:
    //   V = V1 + ((V2 - V1) / (d2 - d1)) * (d - d1)
    inline float distanceToVolume(float distanceCm) {

        // Di atas level maksimum (sensor lebih dekat dari kalibrasi penuh)
        if (distanceCm <= CALIBRATION_TABLE[0].distance_cm) {
            return CALIBRATION_TABLE[0].volume_ml;
        }

        // Di bawah level minimum (wadah kosong / jarak terlalu jauh)
        if (distanceCm >= CALIBRATION_TABLE[CALIBRATION_SIZE - 1].distance_cm) {
            return CALIBRATION_TABLE[CALIBRATION_SIZE - 1].volume_ml;
        }

        for (size_t i = 0; i < CALIBRATION_SIZE - 1; i++) {

            float d1 = CALIBRATION_TABLE[i].distance_cm;
            float d2 = CALIBRATION_TABLE[i + 1].distance_cm;

            float v1 = CALIBRATION_TABLE[i].volume_ml;
            float v2 = CALIBRATION_TABLE[i + 1].volume_ml;

            // Cari interval yang mengapit (d1 ≤ d ≤ d2, d1 selalu < d2)
            if (distanceCm >= d1 && distanceCm <= d2) {

                // Interpolasi linear
                float ratio = (distanceCm - d1) / (d2 - d1);

                return v1 + ratio * (v2 - v1);
            }
        }

        // Seharusnya tidak pernah sampai sini
        return 0.0f;
    }

    // Volume (ml) → level (%)
    inline float volumeToPercent(float volumeMl) {
        float percent = (volumeMl / CALIBRATION_MAX_VOLUME) * 100.0f;
        return constrain(percent, 0.0f, 100.0f);
    }
}
