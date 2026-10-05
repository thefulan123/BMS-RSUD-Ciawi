#pragma once

#include <Arduino.h>

// ============================================================
// KALIBRASI TANGKI & SENSOR — NATURAL CUBIC SPLINE
// --------------------------------
// File ini SERING diubah saat:
//   - Pindah tangki (GWT-001 → GWT-002)
//   - Ganti sensor
//   - Perubahan mounting
//
// Konsep:
//   Kurva halus V(d) yang LEWAT TEPAT di semua titik kalibrasi.
//   Tiap interval (diferensial) punya persamaan kubik sendiri:
//
//     V(t) = a + b·t + c·t² + d·t³
//     t    = distance − KNOTS[i].distance_cm
//
//   Dan tiap titik (knot) punya nilai diferensial (gradien dV/dd)
//   sendiri, kontinu antar interval — hasil mulus kayak jarum analog.
//
// Cara pakai:
//   1. Ukur jarak sensor → permukaan air (cm) untuk tiap volume
//   2. Masukkan titik ke KNOTS[] (urut jarak naik)
//   3. Regenerate koefisien SPLINE[] (lihat docs/06-calibration.md)
//
// Sifat:
//   - Lewat tepat semua titik (error 0)
//   - Gradien kontinu di tiap knot (C1 continuous)
//   - Monotonic, tidak ada wiggle, tidak overshoot 0..MAX_VOLUME
// ============================================================

// --- Suhu ruang (kompensasi kecepatan suara) ---
constexpr float TEMPERATURE_C = 16.0f;

// --- Batas clamp ---
constexpr float CALIBRATION_MAX_VOLUME = 1000.0f;

// --- Titik kalibrasi (knots): urut jarak NAIK ---
// distance_cm, volume_ml, slope (ml/cm) = diferensial dV/dd di titik ini
struct CalibrationKnot
{
    float distance_cm;
    float volume_ml;
    float slope;       // ml per cm — makin negatif = makin curam
};

constexpr CalibrationKnot KNOTS[] = {
    { 4.190f, 1000.0f,  -11.6072f},
    { 5.625f,  900.0f, -185.8448f},
    { 6.100f,  800.0f, -202.2150f},
    { 8.660f,  600.0f,  -52.5529f},
    {10.000f,  500.0f,  -80.5923f},
    {11.340f,  400.0f,  -72.8391f},
    {12.660f,  300.0f,  -79.2404f},
    {14.000f,  200.0f,  -61.2915f},
    {15.350f,  100.0f, -121.8610f},
    {16.020f,    0.0f, -162.9501f}
};

constexpr size_t KNOT_COUNT = sizeof(KNOTS) / sizeof(KNOTS[0]);
constexpr size_t SEG_COUNT  = KNOT_COUNT - 1;

// --- Koefisien spline per interval ---
// V(t) = a + b·t + c·t² + d·t³,  t = distance − KNOTS[i].distance_cm
// (natural cubic spline: M0 = Mn = 0, dihasilkan offline — lihat docs)
struct SplineSegment
{
    float a;
    float b;
    float c;
    float d;
};

constexpr SplineSegment SPLINE[] = {
    //  a           b            c            d            interval
    {1000.0000f,  -11.6072f,    0.000000f,  -28.204384f}, //  4.190 ->  5.625
    { 900.0000f, -185.8448f, -121.419871f,  146.228822f}, //  5.625 ->  6.100
    { 800.0000f, -202.2150f,   86.956200f,  -15.032634f}, //  6.100 ->  8.660
    { 600.0000f,  -52.5529f,  -28.494432f,    8.971123f}, //  8.660 -> 10.000
    { 500.0000f,  -80.5923f,    7.569481f,   -2.326610f}, // 10.000 -> 11.340
    { 400.0000f,  -72.8391f,   -1.783491f,   -0.323855f}, // 11.340 -> 12.660
    { 300.0000f,  -79.2404f,   -3.065956f,    4.857356f}, // 12.660 -> 14.000
    { 200.0000f,  -61.2915f,   16.460616f,  -19.206802f}, // 14.000 -> 15.350
    { 100.0000f, -121.8610f,  -61.326932f,   30.510911f}  // 15.350 -> 16.020
};

static_assert(sizeof(SPLINE) / sizeof(SPLINE[0]) == SEG_COUNT,
              "SPLINE[] dan KNOTS[] tidak cocok — regenerate koefisien!");

// --- Fungsi helper ---
namespace Calibration {

    // Echo time (µs) → jarak (cm), dengan kompensasi suhu
    inline float echoUsToDistance(unsigned long echoUs, float temperatureC) {
        float speed = 331.3f + (0.606f * temperatureC); // m/s
        return echoUs * speed / 20000.0f;               // cm
    }

    // Jarak (cm) → volume (ml) via natural cubic spline.
    // Lewat tepat semua titik kalibrasi, gradien kontinu antar interval.
    inline float distanceToVolume(float distanceCm) {

        // Clamp di luar rentang kalibrasi
        if (distanceCm <= KNOTS[0].distance_cm) {
            return KNOTS[0].volume_ml;              // penuh / terlalu dekat
        }
        if (distanceCm >= KNOTS[KNOT_COUNT - 1].distance_cm) {
            return KNOTS[KNOT_COUNT - 1].volume_ml;  // kosong / terlalu jauh
        }

        // Cari interval, lalu Horner: a + t·(b + t·(c + t·d))
        for (size_t i = 0; i < SEG_COUNT; i++) {
            if (distanceCm <= KNOTS[i + 1].distance_cm) {
                float t = distanceCm - KNOTS[i].distance_cm;
                const SplineSegment &s = SPLINE[i];

                float volume = s.a + t * (s.b + t * (s.c + t * s.d));
                return constrain(volume, 0.0f, CALIBRATION_MAX_VOLUME);
            }
        }

        return 0.0f;  // seharusnya tidak pernah sampai sini
    }

    // Jarak (cm) → diferensial / gradien (ml per cm) di titik itu.
    // Ini "nilai tiap diferensial": seberapa cepat volume berubah per cm.
    inline float distanceToSlope(float distanceCm) {

        if (distanceCm <= KNOTS[0].distance_cm) {
            return KNOTS[0].slope;
        }
        if (distanceCm >= KNOTS[KNOT_COUNT - 1].distance_cm) {
            return KNOTS[KNOT_COUNT - 1].slope;
        }

        for (size_t i = 0; i < SEG_COUNT; i++) {
            if (distanceCm <= KNOTS[i + 1].distance_cm) {
                float t = distanceCm - KNOTS[i].distance_cm;
                const SplineSegment &s = SPLINE[i];

                // Turunan: V'(t) = b + 2c·t + 3d·t²
                return s.b + t * (2.0f * s.c + 3.0f * s.d * t);
            }
        }

        return 0.0f;
    }

    // Volume (ml) → level (%)
    inline float volumeToPercent(float volumeMl) {
        float percent = (volumeMl / CALIBRATION_MAX_VOLUME) * 100.0f;
        return constrain(percent, 0.0f, 100.0f);
    }
}
