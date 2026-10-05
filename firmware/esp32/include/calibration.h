#pragma once

#include <Arduino.h>

// ============================================================
// KALIBRASI TANGKI & SENSOR — REGRESI LINEAR (least squares)
// --------------------------------
// File ini SERING diubah saat:
//   - Pindah tangki (GWT-001 → GWT-002)
//   - Ganti sensor
//   - Perubahan mounting
//
// Cara pakai:
//   1. Ukur jarak sensor → permukaan air (cm) di 3 titik
//   2. Masukkan 3 titik di bawah (C1/C2/C3)
//   3. m & b dihitung otomatis, firmware pakai V = m·d + b
//
// Rumus regresi linear (n = 3):
//   m = (n·ΣdV − Σd·ΣV) / (n·Σd² − (Σd)²)
//   b = (ΣV − m·Σd) / n
//
// Dengan 3 titik GWT-001 (4.16,1000) (9.58,700) (15.89,0):
//   V(d) = −85.9490·d + 1415.5564
// ============================================================

// --- Suhu ruang (kompensasi kecepatan suara) ---
constexpr float TEMPERATURE_C = 16.0f;

// --- 3 titik kalibrasi (UBAH DI SINI SAJA) ---
constexpr float CAL_D1 = 4.16f;    constexpr float CAL_V1 = 1000.0f;
constexpr float CAL_D2 = 9.58f;    constexpr float CAL_V2 = 700.0f;
constexpr float CAL_D3 = 15.89f;   constexpr float CAL_V3 = 0.0f;

// --- Volume penuh (batas clamp atas) ---
constexpr float CALIBRATION_MAX_VOLUME = 1000.0f;

// --- Regresi linear least-squares, dihitung di compile time ---
constexpr float CAL_N      = 3.0f;
constexpr float CAL_SUM_D  = CAL_D1 + CAL_D2 + CAL_D3;              // Σd  = 29.63
constexpr float CAL_SUM_V  = CAL_V1 + CAL_V2 + CAL_V3;              // ΣV  = 1700
constexpr float CAL_SUM_DD = CAL_D1 * CAL_D1
                           + CAL_D2 * CAL_D2
                           + CAL_D3 * CAL_D3;                       // Σd² = 361.5741
constexpr float CAL_SUM_DV = CAL_D1 * CAL_V1
                           + CAL_D2 * CAL_V2
                           + CAL_D3 * CAL_V3;                       // ΣdV = 10866

// m = (n·ΣdV − Σd·ΣV) / (n·Σd² − (Σd)²)   →  −85.9490
constexpr float CALIBRATION_SLOPE =
    (CAL_N * CAL_SUM_DV - CAL_SUM_D * CAL_SUM_V) /
    (CAL_N * CAL_SUM_DD - CAL_SUM_D * CAL_SUM_D);

// b = (ΣV − m·Σd) / n                       →  1415.5564
constexpr float CALIBRATION_INTERCEPT =
    (CAL_SUM_V - CALIBRATION_SLOPE * CAL_SUM_D) / CAL_N;

// --- Fungsi helper ---
namespace Calibration {

    // Echo time (µs) → jarak (cm), dengan kompensasi suhu
    inline float echoUsToDistance(unsigned long echoUs, float temperatureC) {
        float speed = 331.3f + (0.606f * temperatureC); // m/s
        return echoUs * speed / 20000.0f;               // cm
    }

    // Jarak (cm) → volume (ml) via persamaan regresi linear.
    //
    //   V(d) = m·d + b
    //
    // Hasil di-clamp supaya di luar rentang kalibrasi tetap 0..MAX_VOLUME.
    inline float distanceToVolume(float distanceCm) {
        float volume = (CALIBRATION_SLOPE * distanceCm) + CALIBRATION_INTERCEPT;
        return constrain(volume, 0.0f, CALIBRATION_MAX_VOLUME);
    }

    // Volume (ml) → level (%)
    inline float volumeToPercent(float volumeMl) {
        float percent = (volumeMl / CALIBRATION_MAX_VOLUME) * 100.0f;
        return constrain(percent, 0.0f, 100.0f);
    }
}
