# Volume Calculation

Perhitungan volume **dilakukan di firmware MCU** (`include/calibration.h`),
menggunakan **regresi linear (least squares)** dari 3 titik kalibrasi.

```
HC-SR04
   ↓
raw echo (µs)
   ↓
temperature compensation      ← v = 331.3 + 0.606·T
   ↓
linear regression             ← V = m·d + b
   ↓
volume + level %
   ↓
MQTT
```

## Persamaan

```
V(d) = −85.9490·d + 1415.5564
```

Untuk firmware:

```cpp
float volume = (-85.9490f * distanceCm) + 1415.5564f;
volume = constrain(volume, 0.0f, 1000.0f);
```

## 3 Titik Kalibrasi

| Distance | Volume |
|----------|--------|
| 4.16 cm | 1000 ml |
| 9.58 cm | 700 ml |
| 15.89 cm | 0 ml |

## Rumus Regresi

```
m = (n·ΣdV − Σd·ΣV) / (n·Σd² − (Σd)²)
b = (ΣV − m·Σd) / n
```

Dengan:

```
n   = 3
Σd  = 29.63
ΣV  = 1700
Σd² = 361.5741
ΣdV = 10866
```

Menghasilkan:

```
m = −85.9490
b = 1415.5564
```

## Temperature Compensation

```cpp
constexpr float TEMPERATURE_C = 16.0f;

inline float echoUsToDistance(unsigned long echoUs, float temperatureC) {
    float speed = 331.3f + (0.606f * temperatureC); // m/s
    return echoUs * speed / 20000.0f;               // cm
}
```

## Implementasi (auto-calculated di compile time)

```cpp
// 3 titik — ubah di sini saja
constexpr float CAL_D1 = 4.16f;    constexpr float CAL_V1 = 1000.0f;
constexpr float CAL_D2 = 9.58f;    constexpr float CAL_V2 = 700.0f;
constexpr float CAL_D3 = 15.89f;   constexpr float CAL_V3 = 0.0f;

// m & b dihitung otomatis dari rumus di atas
constexpr float CALIBRATION_SLOPE     = /* −85.9490 */;
constexpr float CALIBRATION_INTERCEPT = /* 1415.5564 */;

// Pakai persamaan
inline float distanceToVolume(float distanceCm) {
    float volume = (CALIBRATION_SLOPE * distanceCm) + CALIBRATION_INTERCEPT;
    return constrain(volume, 0.0f, CALIBRATION_MAX_VOLUME);
}
```

Keuntungannya: kalau GWT lain beda, cukup ganti 3 titik — `m` & `b` ikut berubah.

## Level Persen

```cpp
inline float volumeToPercent(float volumeMl) {
    float percent = (volumeMl / CALIBRATION_MAX_VOLUME) * 100.0f;
    return constrain(percent, 0.0f, 100.0f);
}
```

## Contoh Perhitungan

| Distance | Volume | Level |
|----------|--------|-------|
| 3.00 cm | 1000.0 ml (clamp) | 100.0% |
| 4.16 cm | 1000.0 ml (clamp) | 100.0% |
| 4.84 cm | 999.6 ml | 100.0% |
| 6.68 cm | 841.4 ml | 84.1% |
| 9.58 cm | 592.2 ml | 59.2% |
| 12.00 cm | 384.2 ml | 38.4% |
| 15.89 cm | 49.8 ml | 5.0% |
| 16.47 cm | 0.0 ml | 0.0% |
| 20.00 cm | 0.0 ml (clamp) | 0.0% |

**Batas clamp:** `d ≤ 4.83 cm` → 1000 ml, `d ≥ 16.47 cm` → 0 ml.

## Catatan: Garis Regresi vs Titik Ukur

Least squares memberi garis terbaik **secara keseluruhan**, bukan melewati
tepat tiap titik:

| Titik | Ukur | Prediksi |
|-------|------|----------|
| 4.16 cm | 1000 ml | 1058 ml (di-clamp 1000) |
| 9.58 cm | 700 ml | 592 ml |
| 15.89 cm | 0 ml | 50 ml (5%) |

Kalau butuh melewati tepat tiap titik → piecewise linear per interval.

## Output MQTT

```json
{
  "distance_cm": 6.68,
  "volume_ml": 841.4,
  "level_percent": 84.1
}
```

Node-RED tinggal terima → tulis ke InfluxDB. Tidak ada konversi lagi.
