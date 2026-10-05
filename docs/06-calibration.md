# Sensor Calibration

Kalibrasi sekarang **di firmware MCU**, bukan di Node-RED.
Semua parameter ada di satu file: **`firmware/esp32/include/calibration.h`**

> Pindah tangki / ganti sensor → edit 1 file ini saja, compile ulang.
> Kode sensor dan MQTT tidak perlu disentuh.

## Data Kalibrasi (GWT-001)

| Volume | Distance | Section |
|--------|----------|---------|
| 0 ml | 16.02 cm | Mengerucut |
| 100 ml | 15.35 cm | Mengerucut |
| 200 ml | 14.00 cm | Linear |
| 300 ml | 12.66 cm | Linear |
| 400 ml | 11.34 cm | Linear |
| 500 ml | 10.00 cm | Linear |
| 600 ml | 8.66 cm | Linear |
| 800 ml | 6.10 cm | Linear |
| 900 ml | 5.625 cm | Linear |
| 1000 ml | 4.19 cm | Linear |

## Tabel di `calibration.h`

```cpp
constexpr CalibrationPoint CALIBRATION_TABLE[] = {
    {4.19f,  1000.0f},
    {5.625f,  900.0f},
    {6.10f,   800.0f},
    {8.66f,   600.0f},
    {10.00f,  500.0f},
    {11.34f,  400.0f},
    {12.66f,  300.0f},
    {14.00f,  200.0f},
    {15.35f,  100.0f},
    {16.02f,    0.0f}
};
```

**Catatan:** tabel ini bukan karakteristik HC-SR04, tapi karakteristik
**HC-SR04 + posisi pemasangan + bentuk tangki GWT-001**.
Makanya ditempatkan di file konfigurasi, bukan di kode sensor.

## Cara Kerja Interpolasi

Tabel diurutkan dari volume terbesar (jarak terkecil) → terkecil (jarak terbesar).
Fungsi `Calibration::distanceToVolume()` melakukan piecewise linear interpolation:

```
distance ≤ 4.19  → 1000 ml (clamp atas)
distance ≥ 16.02 → 0 ml    (clamp bawah)
di antara        → interpolasi linear antar 2 titik terdekat
```

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

1. Kosongkan wadah → catat distance (`16.02 cm` misal)
2. Isi bertahap (100, 200, ... 1000 ml) → catat distance tiap titik
3. Masukkan ke `CALIBRATION_TABLE` di `calibration.h`
4. Compile & upload: `pio run --target upload`
5. Cek serial monitor: pastikan `volume_ml` sesuai

## Multi-Tangki

Kalau ada 10 tangki, firmware dasar sama. Yang beda hanya `calibration.h`:

| Device | Calibration |
|--------|-------------|
| GWT-001 | `calibration.h` v1 |
| GWT-002 | `calibration.h` v2 |

Rencana lanjutan: simpan di **NVS** supaya OTA firmware tidak menghapus kalibrasi.

## Tips

- Pastikan sensor sejajar dengan permukaan air
- Hindari gelombang/air bergerak
- Kalibrasi di suhu yang sama dengan penggunaan
- Titik kalibrasi makin banyak → makin akurat
