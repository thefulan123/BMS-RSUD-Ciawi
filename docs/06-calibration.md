# Sensor Calibration

Kalibrasi **di firmware MCU**, pakai **piecewise linear** — kumpulan persamaan linear
yang disambungkan di titik-titik kalibrasi.
Semua parameter ada di satu file: **`firmware/esp32/include/calibration.h`**

> Pindah tangki / ganti sensor → ganti **tabel titik** saja, compile ulang.
> Persamaan per interval dihitung otomatis — tidak perlu hitung manual.

## Rumus (Persamaan Linear per Interval)

```
V(d) = V1 + ((V2 - V1) / (d2 - d1)) × (d - d1)
```

| Simbol | Arti |
|--------|------|
| `d` | jarak sensor saat ini (cm) |
| `V` | volume hasil (ml) |
| `d1, V1` | titik kalibrasi pertama (interval bawah) |
| `d2, V2` | titik kalibrasi kedua (interval atas) |

Program tinggal **cari 2 titik yang mengapit `d`**, lalu pakai rumus di atas.

## Tabel Kalibrasi GWT-001

**PENTING: urut berdasarkan jarak NAIK** (4.19 → 16.02), sehingga volume MENURUN.

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

## Daftar Persamaan per Interval

Dihitung otomatis dari tabel:

| Interval (cm) | Persamaan V(d) |
|---------------|----------------|
| 4.19 – 5.625 | V = −69.6864·d + 1291.9861 |
| 5.625 – 6.10 | V = −210.5263·d + 2084.2105 |
| **6.10 – 8.66** | **V = −78.125·d + 1276.5625** |
| 8.66 – 10.00 | V = −74.6269·d + 1246.2687 |
| 10.00 – 11.34 | V = −74.6269·d + 1246.2687 |
| 11.34 – 12.66 | V = −75.7576·d + 1259.0909 |
| 12.66 – 14.00 | V = −74.6269·d + 1244.7761 |
| 14.00 – 15.35 | V = −74.0741·d + 1237.0370 |
| 15.35 – 16.02 | V = −149.2537·d + 2391.0448 |

## Contoh: 6.68 cm

Masuk interval `6.10 ≤ d ≤ 8.66`, pakai `V = −78.125·d + 1276.5625`:

```
V = 800 + ((600 - 800) / (8.66 - 6.10)) × (6.68 - 6.10)
  = 800 + (-78.125 × 0.58)
  ≈ 754.7 ml
```

Output MQTT:

```json
{
  "distance_cm": 6.68,
  "volume_ml": 754.7,
  "level_percent": 75.5
}
```

## Implementasi di Firmware

Tidak perlu tulis 9 persamaan manual — cukup rumus umum + loop cari interval:

```cpp
inline float distanceToVolume(float distanceCm) {

    // Clamp di luar rentang kalibrasi
    if (distanceCm <= TABLE[0].distance_cm)          return TABLE[0].volume_ml;
    if (distanceCm >= TABLE[N-1].distance_cm)        return TABLE[N-1].volume_ml;

    for (size_t i = 0; i < N - 1; i++) {
        float d1 = TABLE[i].distance_cm;
        float d2 = TABLE[i + 1].distance_cm;
        float v1 = TABLE[i].volume_ml;
        float v2 = TABLE[i + 1].volume_ml;

        if (distanceCm >= d1 && distanceCm <= d2) {
            float ratio = (distanceCm - d1) / (d2 - d1);
            return v1 + ratio * (v2 - v1);   // ← rumus umum
        }
    }
    return 0.0f;
}
```

### Kenapa urutan tabel penting?

Karena tabel urut jarak **naik**, maka `d1 < d2` selalu benar.
Kondisi `d1 ≤ d ≤ d2` jadi valid — beda kalau tabel diurutkan volume,
kondisi bisa kebalik dan loop jatuh ke `return 0.0f` (bug lama: 6.68 cm → 0 ml).

## Kompensasi Suhu

```cpp
constexpr float TEMPERATURE_C = 16.0f;   // ubah di calibration.h

float speed = 331.3f + (0.606f * TEMPERATURE_C);  // m/s
float distance_cm = echoUs * speed / 20000.0f;
```

Nanti kalau pasang sensor suhu, `TEMPERATURE_C` tinggal diganti
dengan pembacaan aktual — tidak perlu ubah tempat lain.

## Cara Kalibrasi Ulang (Tangki Baru)

1. Kosongkan wadah → catat distance
2. Isi bertahap (100, 200, ... ml) → catat distance tiap titik
3. Masukkan ke `CALIBRATION_TABLE` **urut jarak naik**
4. Compile & upload: `pio run --target upload`
5. Cek serial monitor: pastikan `volume_ml` sesuai

Titik kalibrasi makin banyak → makin akurat (mengikuti bentuk tangki aktual).

## Multi-Tangki

Firmware dasar sama untuk semua device. Yang beda hanya `calibration.h`:

| Device | Calibration |
|--------|-------------|
| GWT-001 | `calibration.h` (tabel GWT-001) |
| GWT-002 | `calibration.h` (tabel GWT-002) |

Rencana lanjutan: simpan di **NVS** supaya OTA firmware tidak menghapus kalibrasi.

## Tips

- Pastikan sensor sejajar dengan permukaan air
- Hindari gelombang/air bergerak
- Kalibrasi di suhu yang sama dengan penggunaan
- Titik di ujung (penuh/kosong) wajib ada — itu batas clamp
