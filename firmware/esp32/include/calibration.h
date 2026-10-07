#pragma once

#include <Arduino.h>

// ============================================================
// KALIBRASI TANGKI & SENSOR — PIECEWISE LINEAR
// --------------------------------
// STATUS: KALIBRASI ULANG SELESAI — 11 titik (0..1000 ml)
// Tanggal: 07 Okt 2026, sensor GWT-001, suhu ruang 16 C
//
// Rumus (persamaan linear per interval):
//   V(d) = V1 + ((V2 - V1) / (d2 - d1)) * (d - d1)
//
// Cara kalibrasi ulang lagi (pindah tangki / ganti sensor):
//   1. Isi tangki bertahap, catat jarak dari serial monitor
//   2. Masukkan titik urut jarak NAIK (volume MENURUN)
//   3. Sesuaikan CALIBRATION_MAX_VOLUME dengan kapasitas tangki
// ============================================================

// --- Suhu ruang (kompensasi kecepatan suara) ---
constexpr float TEMPERATURE_C = 16.0f;

// --- Titik kalibrasi ---
// PENTING: urut berdasarkan jarak NAIK (3.85 -> 15.87)
//          sehingga volume MENURUN (1000 -> 0)
struct CalibrationPoint {
    float distance_cm;
    float volume_ml;
};

constexpr CalibrationPoint CALIBRATION_TABLE[] = {
    { 3.85f, 1000.0f},
    { 5.42f,  900.0f},
    { 6.99f,  800.0f},
    { 7.93f,  700.0f},
    { 9.24f,  600.0f},
    { 9.92f,  500.0f},
    {12.24f,  400.0f},
    {12.91f,  300.0f},
    {14.22f,  200.0f},
    {15.21f,  100.0f},
    {15.87f,    0.0f}
};

constexpr size_t CALIBRATION_SIZE =
    sizeof(CALIBRATION_TABLE) / sizeof(CALIBRATION_TABLE[0]);

constexpr float CALIBRATION_MAX_VOLUME = 1000.0f;  // kapasitas tangki (ml)

// --- DUA PERSAMAAN KALIBRASI (regresi 11 titik, 07 Okt 2026) ---
// Dipakai BERSAMAAN biar bisa dibandingkan langsung.
//
//   [A] regresi linear LS : V = 81.498 * (16.481 - d)
//       RMSE 26.6 ml, max 54 ml. Paling akurat.
//       A  = penampang tangki = 81.498 cm^2
//       d0 = jarak dasar      = 16.481 cm  (V = 0)
//
//   [B] jangkar di titik 0 : V = 87.546 * (15.870 - d)
//       RMSE 36.8 ml, max 82 ml. Dipaksa 0 ml pas di 15.87 cm.
constexpr float CAL_A_SLOPE = 81.498f;   // cm^2 (penampang)
constexpr float CAL_A_D0    = 16.481f;   // cm (jarak saat V=0)
constexpr float CAL_B_SLOPE = 87.546f;
constexpr float CAL_B_D0    = 15.870f;   // cm (titik 0 ml terukur)

// --- Fungsi helper ---
namespace Calibration {

    // Echo time (µs) → jarak (cm), dengan kompensasi suhu
    inline float echoUsToDistance(unsigned long echoUs, float temperatureC) {
        float speed = 331.3f + (0.606f * temperatureC); // m/s
        return echoUs * speed / 20000.0f;               // cm
    }

    // ==== PERSAMAAN A (regresi linear LS) ====
    // V = CAL_A_SLOPE * (CAL_A_D0 - d), lalu clamp [0, MAX].
    inline float distanceToVolumeA(float distanceCm) {
        float v = CAL_A_SLOPE * (CAL_A_D0 - distanceCm);
        return constrain(v, 0.0f, CALIBRATION_MAX_VOLUME);
    }

    // ==== PERSAMAAN B (jangkar di titik 0 ml) ====
    // V = CAL_B_SLOPE * (CAL_B_D0 - d), lalu clamp [0, MAX].
    inline float distanceToVolumeB(float distanceCm) {
        float v = CAL_B_SLOPE * (CAL_B_D0 - distanceCm);
        return constrain(v, 0.0f, CALIBRATION_MAX_VOLUME);
    }

    // ==== PIECEWISE (tabel 11 titik) - dipertahankan sbg referensi ====
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
