# Volume Calculation

Perhitungan volume **dilakukan di firmware MCU** (`include/calibration.h`),
menggunakan **piecewise linear** — kumpulan persamaan linear yang disambungkan
di titik-titik kalibrasi.

```
HC-SR04
   ↓
raw echo (µs)
   ↓
temperature compensation      ← v = 331.3 + 0.606·T
   ↓
piecewise linear              ← V = V1 + ((V2-V1)/(d2-d1))·(d-d1)
   ↓
volume + level %
   ↓
MQTT
```

## Rumus Umum

```
V(d) = V1 + ((V2 - V1) / (d2 - d1)) × (d - d1)
```

Program mencari 2 titik kalibrasi yang mengapit `d`, lalu pakai rumus ini.
Jadi kalau kalibrasi GWT berubah, cukup ganti tabel titiknya —
tidak perlu hitung ulang persamaan satu-satu.

## Temperature Compensation

```cpp
constexpr float TEMPERATURE_C = 16.0f;

inline float echoUsToDistance(unsigned long echoUs, float temperatureC) {
    float speed = 331.3f + (0.606f * temperatureC); // m/s
    return echoUs * speed / 20000.0f;               // cm
}
```

## Tabel Kalibrasi (urut jarak naik)

```cpp
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
```

## Interpolasi

```cpp
inline float distanceToVolume(float distanceCm) {
    if (distanceCm <= TABLE[0].distance_cm)   return TABLE[0].volume_ml;
    if (distanceCm >= TABLE[N-1].distance_cm) return TABLE[N-1].volume_ml;

    for (size_t i = 0; i < N - 1; i++) {
        float d1 = TABLE[i].distance_cm,     d2 = TABLE[i+1].distance_cm;
        float v1 = TABLE[i].volume_ml,       v2 = TABLE[i+1].volume_ml;

        if (distanceCm >= d1 && distanceCm <= d2) {
            float ratio = (distanceCm - d1) / (d2 - d1);
            return v1 + ratio * (v2 - v1);
        }
    }
    return 0.0f;
}
```

## Level Persen

```cpp
inline float volumeToPercent(float volumeMl) {
    float percent = (volumeMl / CALIBRATION_MAX_VOLUME) * 100.0f;
    return constrain(percent, 0.0f, 100.0f);
}
```

## Daftar Persamaan per Interval

| Interval (cm) | Persamaan V(d) |
|---------------|----------------|
| 4.19 – 5.625 | V = −69.6864·d + 1291.9861 |
| 5.625 – 6.10 | V = −210.5263·d + 2084.2105 |
| 6.10 – 8.66 | V = −78.125·d + 1276.5625 |
| 8.66 – 10.00 | V = −74.6269·d + 1246.2687 |
| 10.00 – 11.34 | V = −74.6269·d + 1246.2687 |
| 11.34 – 12.66 | V = −75.7576·d + 1259.0909 |
| 12.66 – 14.00 | V = −74.6269·d + 1244.7761 |
| 14.00 – 15.35 | V = −74.0741·d + 1237.0370 |
| 15.35 – 16.02 | V = −149.2537·d + 2391.0448 |

## Contoh Perhitungan

| Distance | Interval | Volume | Level |
|----------|----------|--------|-------|
| 3.00 cm | clamp | 1000.0 ml | 100.0% |
| 4.19 cm | batas | 1000.0 ml | 100.0% |
| 6.10 cm | titik | 800.0 ml | 80.0% |
| **6.68 cm** | 6.10–8.66 | **754.7 ml** | **75.5%** |
| 8.66 cm | titik | 600.0 ml | 60.0% |
| 10.00 cm | titik | 500.0 ml | 50.0% |
| 13.00 cm | 12.66–14.00 | 274.6 ml | 27.5% |
| 16.02 cm | batas | 0.0 ml | 0.0% |
| 20.00 cm | clamp | 0.0 ml | 0.0% |

## Output MQTT

```json
{
  "distance_cm": 6.68,
  "volume_ml": 754.7,
  "level_percent": 75.5
}
```

Node-RED tinggal terima → tulis ke InfluxDB. Tidak ada konversi lagi.
