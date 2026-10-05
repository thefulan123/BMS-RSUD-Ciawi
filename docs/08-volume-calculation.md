# Volume Calculation

Perhitungan volume **dilakukan di firmware MCU** (`include/calibration.h`),
menggunakan **satu persamaan linear global**.

```
HC-SR04
   ↓
raw echo (µs)
   ↓
temperature compensation      ← v = 331.3 + 0.606·T
   ↓
linear equation               ← V = m·d + b
   ↓
volume + level %
   ↓
MQTT
```

## Parameter Kalibrasi (GWT-001)

```cpp
constexpr float CALIBRATION_MAX_VOLUME   = 1000.0f;  // ml
constexpr float CALIBRATION_MIN_DISTANCE = 4.19f;    // cm (penuh)
constexpr float CALIBRATION_MAX_DISTANCE = 16.02f;   // cm (kosong)
```

## Persamaan Linear

Dihitung otomatis di compile time:

```cpp
// m = (0 - MAX_VOLUME) / (MAX_DISTANCE - MIN_DISTANCE)
constexpr float CALIBRATION_SLOPE =
    (0.0f - CALIBRATION_MAX_VOLUME) /
    (CALIBRATION_MAX_DISTANCE - CALIBRATION_MIN_DISTANCE);
// = -84.53

// b = MAX_VOLUME - m·MIN_DISTANCE
constexpr float CALIBRATION_INTERCEPT =
    CALIBRATION_MAX_VOLUME - (CALIBRATION_SLOPE * CALIBRATION_MIN_DISTANCE);
// = 1354.18

// Jadi: V = -84.53·d + 1354.18
```

## Temperature Compensation

```cpp
// calibration.h
constexpr float TEMPERATURE_C = 16.0f;

inline float echoUsToDistance(unsigned long echoUs, float temperatureC) {
    float speed = 331.3f + (0.606f * temperatureC); // m/s
    return echoUs * speed / 20000.0f;               // cm
}
```

## Rumus Volume

```cpp
inline float distanceToVolume(float distanceCm) {
    float volume = (CALIBRATION_SLOPE * distanceCm) + CALIBRATION_INTERCEPT;
    return constrain(volume, 0.0f, CALIBRATION_MAX_VOLUME);
}
```

Clamp penting — supaya di luar rentang kalibrasi tetap 0..1000 ml.

## Level Persen

```cpp
inline float volumeToPercent(float volumeMl) {
    float percent = (volumeMl / CALIBRATION_MAX_VOLUME) * 100.0f;
    return constrain(percent, 0.0f, 100.0f);
}
```

## Contoh Perhitungan

| Echo (µs) | Distance | Volume | Level |
|-----------|----------|--------|-------|
| 940 | 16.02 cm | 0.0 ml | 0.0% |
| 822 | 14.00 cm | 170.8 ml | 17.1% |
| 587 | 10.00 cm | 508.9 ml | 50.9% |
| 392 | 6.68 cm | 789.5 ml | 79.0% |
| 246 | 4.19 cm | 1000.0 ml | 100.0% |
| 176 | 3.00 cm | 1000.0 ml | 100.0% (clamp) |

## Output MQTT

```json
{
  "distance_cm": 6.68,
  "volume_ml": 789.5,
  "level_percent": 79.0
}
```

Node-RED tinggal terima → tulis ke InfluxDB. Tidak ada konversi lagi.
