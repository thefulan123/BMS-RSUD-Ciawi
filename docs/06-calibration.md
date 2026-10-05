# Sensor Calibration

Kalibrasi **di firmware MCU**, pakai **regresi linear (least squares)** dari 3 titik ukur.
Semua parameter ada di satu file: **`firmware/esp32/include/calibration.h`**

> Pindah tangki / ganti sensor → ganti **3 titik ukur** saja, compile ulang.
> `m` dan `b` dihitung otomatis di compile time.

## Rumus Regresi Linear

Persamaan umum:

```
V = m·d + b
```

dengan:

```
m = (n·ΣdV − Σd·ΣV) / (n·Σd² − (Σd)²)
b = (ΣV − m·Σd) / n
```

## 3 Titik Kalibrasi GWT-001

| # | Distance (d) | Volume (V) |
|---|--------------|------------|
| 1 | 4.16 cm | 1000 ml |
| 2 | 9.58 cm | 700 ml |
| 3 | 15.89 cm | 0 ml |

## Perhitungan

```
n     = 3
Σd    = 4.16 + 9.58 + 15.89 = 29.63
ΣV    = 1000 + 700 + 0      = 1700
Σd²   = 4.16² + 9.58² + 15.89² = 361.5741
ΣdV   = (4.16)(1000) + (9.58)(700) + (15.89)(0) = 10866

m = (3·10866 − 29.63·1700) / (3·361.5741 − 29.63²)
  = (32598 − 50371) / (1084.7223 − 877.9369)
  = −17773 / 206.7854
  ≈ −85.9490

b = (1700 − (−85.9490)(29.63)) / 3
  ≈ 1415.5564
```

Persamaan lengkap:

```
V(d) = −85.9490·d + 1415.5564
```

## `calibration.h`

```cpp
// --- 3 titik kalibrasi (UBAH DI SINI SAJA) ---
constexpr float CAL_D1 = 4.16f;    constexpr float CAL_V1 = 1000.0f;
constexpr float CAL_D2 = 9.58f;    constexpr float CAL_V2 = 700.0f;
constexpr float CAL_D3 = 15.89f;   constexpr float CAL_V3 = 0.0f;

// --- Regresi linear, dihitung di compile time ---
constexpr float CALIBRATION_SLOPE =
    (CAL_N * CAL_SUM_DV - CAL_SUM_D * CAL_SUM_V) /
    (CAL_N * CAL_SUM_DD - CAL_SUM_D * CAL_SUM_D);      // −85.9490

constexpr float CALIBRATION_INTERCEPT =
    (CAL_SUM_V - CALIBRATION_SLOPE * CAL_SUM_D) / CAL_N;  // 1415.5564
```

## Contoh Perhitungan

| Distance | Volume | Level | Keterangan |
|----------|--------|-------|------------|
| 3.00 cm | 1000.0 ml | 100.0% | clamp atas |
| 4.16 cm | 1000.0 ml | 100.0% | titik 1 (prediksi 1058 → clamp) |
| 4.84 cm | 1000.0 ml | 100.0% | batas clamp atas |
| 6.68 cm | 841.4 ml | 84.1% | |
| 9.58 cm | 592.2 ml | 59.2% | titik 2 (prediksi 592) |
| 12.00 cm | 384.2 ml | 38.4% | |
| 15.89 cm | 49.8 ml | 5.0% | titik 3 (prediksi 50 → hampir 0) |
| 16.47 cm | 0.0 ml | 0.0% | batas clamp bawah |
| 20.00 cm | 0.0 ml | 0.0% | clamp bawah |

**Batas clamp:** `V = 1000` untuk `d ≤ 4.83 cm`, `V = 0` untuk `d ≥ 16.47 cm`.

### Catatan akurasi

Garis regresi **tidak melewati tepat** 3 titik (itu memang sifat least squares —
garis terbaik secara keseluruhan, bukan interpolasi):

| Titik | Ukur | Prediksi garis |
|-------|------|----------------|
| 4.16 cm | 1000 ml | 1058 ml → di-clamp 1000 |
| 9.58 cm | 700 ml | 592 ml |
| 15.89 cm | 0 ml | 50 ml → masih 5% |

Keuntungan: cuma 1 persamaan, simpel, cocok untuk monitoring level.
Kalau butuh melewati tepat tiap titik, pakai piecewise linear
(interpolasi per interval) — lihat git history.

## Kompensasi Suhu

```cpp
constexpr float TEMPERATURE_C = 16.0f;   // ubah di calibration.h

float speed = 331.3f + (0.606f * TEMPERATURE_C);  // m/s
float distance_cm = echoUs * speed / 20000.0f;
```

Nanti kalau pasang sensor suhu, `TEMPERATURE_C` tinggal diganti
dengan pembacaan aktual — tidak perlu ubah tempat lain.

## Cara Kalibrasi Ulang (Tangki Baru)

1. Kosongkan wadah → catat distance (misal `15.89 cm`)
2. Isi penuh → catat distance (misal `4.16 cm`)
3. Isi titik tengah → catat distance & volume (misal `9.58 cm` @ 700 ml)
4. Masukkan ke `CAL_D1..CAL_D3` / `CAL_V1..CAL_V3`
5. Compile & upload: `pio run --target upload`
6. Cek serial monitor: pastikan `volume_ml` sesuai

## Multi-Tangki

Firmware dasar sama untuk semua device. Yang beda hanya 3 titik di `calibration.h`:

| Device | Titik kalibrasi |
|--------|-----------------|
| GWT-001 | (4.16,1000) (9.58,700) (15.89,0) |
| GWT-002 | ? |

Rencana lanjutan: simpan di **NVS** supaya OTA firmware tidak menghapus kalibrasi.

## Tips

- Pastikan sensor sejajar dengan permukaan air
- Hindari gelombang/air bergerak
- Kalibrasi di suhu yang sama dengan penggunaan
- Pilih titik tengah yang benar-benar representatif (titik inilah yang menentukan kemiringan garis)
