# Sensor Calibration

Kalibrasi sekarang **di firmware MCU**, dan pakai **satu persamaan linear global**.
Semua parameter ada di satu file: **`firmware/esp32/include/calibration.h`**

> Pindah tangki / ganti sensor → ubah **3 angka** saja, compile ulang.
> Gradien `m` dan offset `b` dihitung otomatis — tidak perlu hitung manual.

## Persamaan

```
V = m·d + b
```

Dihitung otomatis dari 3 parameter:

```cpp
m = (0 - MAX_VOLUME) / (MAX_DISTANCE - MIN_DISTANCE)
b = MAX_VOLUME - m·MIN_DISTANCE
```

Dengan parameter GWT-001 sekarang:

| Parameter | Nilai | Arti |
|-----------|-------|------|
| `CALIBRATION_MAX_VOLUME` | 1000.0 ml | volume penuh |
| `CALIBRATION_MIN_DISTANCE` | 4.19 cm | jarak saat penuh |
| `CALIBRATION_MAX_DISTANCE` | 16.02 cm | jarak saat kosong |

Menghasilkan:

```
m = -84.53
b = 1354.18

V = -84.53·d + 1354.18
```

## `calibration.h`

```cpp
// --- Suhu ruang (kompensasi kecepatan suara) ---
constexpr float TEMPERATURE_C = 16.0f;

// --- Parameter kalibrasi tangki (UBAH DI SINI SAJA) ---
constexpr float CALIBRATION_MAX_VOLUME   = 1000.0f;  // volume penuh (ml)
constexpr float CALIBRATION_MIN_DISTANCE = 4.19f;    // jarak saat penuh (cm)
constexpr float CALIBRATION_MAX_DISTANCE = 16.02f;   // jarak saat kosong (cm)

// --- Persamaan linear (dihitung otomatis) ---
constexpr float CALIBRATION_SLOPE =
    (0.0f - CALIBRATION_MAX_VOLUME) /
    (CALIBRATION_MAX_DISTANCE - CALIBRATION_MIN_DISTANCE);

constexpr float CALIBRATION_INTERCEPT =
    CALIBRATION_MAX_VOLUME - (CALIBRATION_SLOPE * CALIBRATION_MIN_DISTANCE);
```

## Contoh Perhitungan

| Distance | Volume | Level | Keterangan |
|----------|--------|-------|------------|
| 3.00 cm | 1000.0 ml | 100.0% | clamp atas |
| 4.19 cm | 1000.0 ml | 100.0% | penuh |
| 6.68 cm | 789.5 ml | 79.0% | |
| 10.00 cm | 508.9 ml | 50.9% | |
| 16.02 cm | 0.0 ml | 0.0% | kosong |
| 20.00 cm | 0.0 ml | 0.0% | clamp bawah |

## Kompensasi Suhu

Kecepatan suara berubah terhadap suhu — sudah otomatis:

```cpp
constexpr float TEMPERATURE_C = 16.0f;   // ubah di calibration.h

float speed = 331.3f + (0.606f * TEMPERATURE_C);  // m/s
float distance_cm = echoUs * speed / 20000.0f;
```

Nanti kalau pasang sensor suhu, `TEMPERATURE_C` tinggal diganti
dengan pembacaan aktual — tidak perlu ubah tempat lain.

## Cara Kalibrasi Ulang (Tangki Baru)

1. Kosongkan wadah → catat distance (misal `16.02 cm`)
2. Isi penuh → catat distance (misal `4.19 cm`)
3. Catat volume penuh (misal `1000 ml`)
4. Masukkan ke 3 parameter di `calibration.h`
5. Compile & upload: `pio run --target upload`
6. Cek serial monitor: pastikan `volume_ml` sesuai

### Cara Ukur dengan 1 Titik Tambahan (opsional)

Kalau mau cek akurasi, ukur di tengah (500 ml) lalu bandingkan:

```bash
# Lihat hasil firmware
pio device monitor --baud 115200
# Distance: 10.00 cm | Volume: 508.9 ml | Level: 50.9 %
```

Selisih wajar < ±5% di titik tengah.

## Akurasi: Linear Global vs Interpolasi

**Linear global** (yang dipakai sekarang) hanya 2 titik: penuh & kosong.

| Jarak | Titik kalibrasi asli | Linear global |
|-------|---------------------|---------------|
| 6.68 cm | ~755 ml | 789.5 ml |
| 10.00 cm | 500 ml | 508.9 ml |

Selisih muncul karena bentuk tangki tidak sepenuhnya linear.
Keuntungannya:

- Kode simpel, gampang debug
- Kalibrasi cukup 3 angka
- Cukup untuk monitoring level

Kalau butuh akurasi lebih tinggi, bisa kembali ke interpolasi
piecewise multi-titik (lihat git history).

## Multi-Tangki

Kalau ada 10 tangki, firmware dasar sama. Yang beda hanya 3 parameter:

| Device | MIN_DISTANCE | MAX_DISTANCE | MAX_VOLUME |
|--------|--------------|--------------|------------|
| GWT-001 | 4.19 | 16.02 | 1000 |
| GWT-002 | ? | ? | ? |

Rencana lanjutan: simpan di **NVS** supaya OTA firmware tidak menghapus kalibrasi.

## Tips

- Pastikan sensor sejajar dengan permukaan air
- Hindari gelombang/air bergerak
- Kalibrasi di suhu yang sama dengan penggunaan
- Titik kosong & penuh harus diukur dengan hati-hati (2 titik ini menentukan semuanya)
