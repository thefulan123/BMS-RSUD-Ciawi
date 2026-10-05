# Volume Calculation

Perhitungan volume **sekarang dilakukan di firmware MCU** (`include/calibration.h`),
bukan lagi di Node-RED.

```
HC-SR04
   ↓
raw echo (µs)
   ↓
temperature compensation      ← v = 331.3 + 0.606*T
   ↓
calibration table (piecewise) ← calibration.h
   ↓
volume + level %
   ↓
MQTT
```

## Data Kalibrasi (GWT-001)

| Volume | Distance |
|--------|----------|
| 0 ml | 16.02 cm |
| 100 ml | 15.35 cm |
| 200 ml | 14.00 cm |
| 500 ml | 10.00 cm |
| 1000 ml | 4.19 cm |

## Temperature Compensation

```cpp
// calibration.h
constexpr float TEMPERATURE_C = 16.0f;

inline float echoUsToDistance(unsigned long echoUs, float temperatureC) {
    float speed = 331.3f + (0.606f * temperatureC); // m/s
    return echoUs * speed / 20000.0f;               // cm
}
```

## Piecewise Linear Interpolation

```cpp
inline float distanceToVolume(float distanceCm) {
    if (distanceCm >= TABLE[last].distance_cm)  return 0.0f;
    if (distanceCm <= TABLE[0].distance_cm)     return TABLE[0].volume_ml;

    for (i = 0; i < SIZE - 1; i++) {
        if (distance between TABLE[i] and TABLE[i+1]) {
            ratio = (dHigh - distanceCm) / (dHigh - dLow);
            return vLow + ratio * (vHigh - vLow);
        }
    }
}
```

## Level Persen

```cpp
inline float volumeToPercent(float volumeMl) {
    float maxVol = CALIBRATION_TABLE[0].volume_ml;
    return (volumeMl / maxVol) * 100.0f;
}
```

## Contoh Perhitungan

| Echo (µs) | Distance | Volume | Level |
|-----------|----------|--------|-------|
| 940 | 16.02 cm | 0 ml | 0% |
| 901 | 15.35 cm | 100 ml | 10% |
| 822 | 14.00 cm | 200 ml | 20% |
| 587 | 10.00 cm | 500 ml | 50% |
| 246 | 4.19 cm | 1000 ml | 100% |

## Output MQTT

```json
{
  "distance_cm": 13.42,
  "volume_ml": 247.0,
  "level_percent": 24.7
}
```

Node-RED tinggal terima → tulis ke InfluxDB. Tidak ada konversi lagi.
