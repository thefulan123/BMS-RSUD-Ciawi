# Volume Calculation

Perhitungan volume **dilakukan di firmware MCU** (`include/calibration.h`),
menggunakan **natural cubic spline** — kurva halus yang lewat tepat di semua
titik kalibrasi, lengkap dengan nilai diferensial (gradien) di tiap titik.

```
HC-SR04
   ↓
raw echo (µs)
   ↓
temperature compensation      ← v = 331.3 + 0.606·T
   ↓
cubic spline                  ← V = a + b·t + c·t² + d·t³
   ↓
volume + level % + differential
   ↓
MQTT
```

## Persamaan Spline

Per interval:

```
V(t) = a + b·t + c·t² + d·t³      t = distance − titik awal interval
```

Evaluasi dengan Horner (3 operasi):

```
V = a + t·(b + t·(c + t·d))
```

**Diferensial** (turunan pertama):

```
V'(t) = b + 2·c·t + 3·d·t²        ← satuan ml/cm
```

## Titik Kalibrasi (GWT-001)

| Distance | Volume |
|----------|--------|
| 4.19 cm | 1000 ml |
| 5.625 cm | 900 ml |
| 6.10 cm | 800 ml |
| 8.66 cm | 600 ml |
| 10.00 cm | 500 ml |
| 11.34 cm | 400 ml |
| 12.66 cm | 300 ml |
| 14.00 cm | 200 ml |
| 15.35 cm | 100 ml |
| 16.02 cm | 0 ml |

## Temperature Compensation

```cpp
constexpr float TEMPERATURE_C = 16.0f;

inline float echoUsToDistance(unsigned long echoUs, float temperatureC) {
    float speed = 331.3f + (0.606f * temperatureC); // m/s
    return echoUs * speed / 20000.0f;               // cm
}
```

## Implementasi di Firmware

```cpp
inline float distanceToVolume(float distanceCm) {
    if (distanceCm <= KNOTS[0].distance_cm)          return KNOTS[0].volume_ml;
    if (distanceCm >= KNOTS[KNOT_COUNT-1].distance_cm)
        return KNOTS[KNOT_COUNT-1].volume_ml;

    for (size_t i = 0; i < SEG_COUNT; i++) {
        if (distanceCm <= KNOTS[i + 1].distance_cm) {
            float t = distanceCm - KNOTS[i].distance_cm;
            const SplineSegment &s = SPLINE[i];
            float volume = s.a + t * (s.b + t * (s.c + t * s.d)); // Horner
            return constrain(volume, 0.0f, CALIBRATION_MAX_VOLUME);
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

## Tiap Diferensial Punya Nilai

Gradien `dV/dd` disimpan di `KNOTS[].slope` dan bisa dihitung di jarak
manapun lewat `Calibration::distanceToSlope()`:

| Distance | Volume | Diferensial | Arti |
|----------|--------|-------------|------|
| 4.19 cm | 1000.0 ml | −11.61 ml/cm | hampir penuh, landai |
| 5.625 cm | 900.0 ml | −185.84 ml/cm | curam |
| 6.10 cm | 800.0 ml | −202.21 ml/cm | **paling curam** |
| 6.68 cm | 709.0 ml | −116.52 ml/cm | |
| 8.66 cm | 600.0 ml | −52.55 ml/cm | landai |
| 10.00 cm | 500.0 ml | −80.59 ml/cm | |
| 14.00 cm | 200.0 ml | −61.29 ml/cm | landai |
| 15.35 cm | 100.0 ml | −121.86 ml/cm | curam di dasar |
| 16.02 cm | 0.0 ml | −162.95 ml/cm | dasar tangki |

Gradien **kontinu antar interval** (beda kiri-kanan di knot ~1e-14) —
tidak ada lompatan, jadi grafik di Grafana mulus.

## Contoh Perhitungan: 6.68 cm

```
t = 6.68 - 6.10 = 0.58
segmen: a=800, b=-202.215, c=86.9562, d=-15.032634

V   = 800 + 0.58·(-202.215 + 0.58·(86.9562 + 0.58·(-15.032634)))
    ≈ 709.0 ml

V'  = -202.215 + 0.58·(2·86.9562 + 3·(-15.032634)·0.58)
    ≈ -116.52 ml/cm
```

## Tabel Hasil

| Distance | Volume | Level |
|----------|--------|-------|
| 4.19 cm | 1000.0 ml | 100.0% |
| 5.00 cm | 975.6 ml | 97.6% |
| 6.68 cm | 709.0 ml | 70.9% |
| 8.00 cm | 626.6 ml | 62.7% |
| 9.58 cm | 534.5 ml | 53.5% |
| 12.00 cm | 351.1 ml | 35.1% |
| 15.89 cm | 21.1 ml | 2.1% |
| 16.02 cm | 0.0 ml | 0.0% |
| >16.02 cm | 0.0 ml (clamp) | 0.0% |

## Sifat yang Diverifikasi

- ✅ Lewat tepat semua 10 titik (error 0)
- ✅ Gradien kontinu di tiap knot
- ✅ Monotonic (0 violations pada sampling 0.001 cm)
- ✅ Range tetap 0..1000 ml (no overshoot)

## Regenerate Koefisien

Kalau titik kalibrasi berubah:

```bash
python3 docs/tools/gen_spline.py titik.txt
# tempel outputnya ke KNOTS[] dan SPLINE[] di calibration.h
```

## Output MQTT (v1.4)

```json
{
  "distance_cm": 6.68,
  "volume_ml": 709.0,
  "level_percent": 70.9,
  "differential": -116.52
}
```

Node-RED tinggal terima → tulis ke InfluxDB. Tidak ada konversi lagi.
