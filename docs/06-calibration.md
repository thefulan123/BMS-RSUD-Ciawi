# Sensor Calibration

Kalibrasi **di firmware MCU**, pakai **natural cubic spline** — kurva halus
yang **lewat tepat di semua titik kalibrasi**, dengan nilai diferensial
(gradien) di tiap titik.
Semua parameter ada di satu file: **`firmware/esp32/include/calibration.h`**

> Pindah tangki / ganti sensor → ganti **tabel titik** + regenerate koefisien.
> Hasilnya mulus kayak jarum analog, tanpa step.

## Kenapa Spline?

| Metode | Lewat tepat tiap titik? | Gradien kontinu? | Halus? |
|--------|------------------------|------------------|--------|
| Linear regression (3 titik) | ❌ | ✅ | garis lurus |
| Piecewise linear | ✅ | ❌ (patah di titik) | siku-siku |
| **Cubic spline** | ✅ | ✅ | ✅ **mulus** |

Spline = kumpulan persamaan kubik yang disambungkan, **turunan pertamanya
kontinu** di tiap sambungan — jadi gak ada sudut tajam.

## Rumus

Per interval `i` (t = distance − titik awal interval):

```
V(t) = a + b·t + c·t² + d·t³
```

Evaluasi pakai Horner (cepat, 3 kali operasi):

```
V = a + t·(b + t·(c + t·d))
```

**Diferensial** (turunan pertama = laju perubahan volume per cm):

```
V'(t) = b + 2·c·t + 3·d·t²
```

## Tabel Kalibrasi GWT-001 (Knots)

**Urut jarak NAIK** (4.19 → 16.02), volume MENURUN.

| Distance | Volume | Diferensial (ml/cm) | Arti |
|----------|--------|---------------------|------|
| 4.19 cm | 1000 ml | −11.61 | hampir penuh → volume nyaris gak berubah |
| 5.625 cm | 900 ml | −185.84 | **paling curam** — 1 cm ≈ 186 ml |
| 6.10 cm | 800 ml | −202.21 | **paling curam** — tangki sempit di sini |
| 8.66 cm | 600 ml | −52.55 | landai |
| 10.00 cm | 500 ml | −80.59 | |
| 11.34 cm | 400 ml | −72.84 | |
| 12.66 cm | 300 ml | −79.24 | |
| 14.00 cm | 200 ml | −61.29 | landai |
| 15.35 cm | 100 ml | −121.86 | makin curam di dasar |
| 16.02 cm | 0 ml | −162.95 | dasar tangki |

**Tiap diferensial ada nilainya** — kolom `slope` di `KNOTS[]` menyimpan
`dV/dd` di tiap titik, dan firmware bisa baca gradien di jarak manapun
lewat `Calibration::distanceToSlope()`.

## Koefisien Spline (SPLINE[])

9 interval → 9 baris koefisien (a, b, c, d):

```
interval            a          b           c            d
4.190 →  5.625   1000.0    -11.6072    0.000000   -28.204384
5.625 →  6.100    900.0   -185.8448 -121.419871   146.228822
6.100 →  8.660    800.0   -202.2150   86.956200   -15.032634
8.660 → 10.000    600.0    -52.5529  -28.494432     8.971123
10.000 → 11.340   500.0    -80.5923    7.569481    -2.326610
11.340 → 12.660   400.0    -72.8391   -1.783491    -0.323855
12.660 → 14.000   300.0    -79.2404   -3.065956     4.857356
14.000 → 15.350   200.0    -61.2915   16.460616   -19.206802
15.350 → 16.020   100.0   -121.8610  -61.326932    30.510911
```

Koefisien ini dihasilkan **offline** dari tabel titik (natural cubic spline,
M₀ = Mₙ = 0). Cara regenerate → lihat bagian bawah.

## Contoh Perhitungan: 6.68 cm

Masuk interval `6.10 → 8.66`, t = 6.68 − 6.10 = 0.58:

```
V = 800 + 0.58·(-202.215 + 0.58·(86.9562 + 0.58·(-15.032634)))
  ≈ 709.0 ml
```

Diferensialnya:

```
V' = -202.215 + 0.58·(2·86.9562 + 3·(-15.032634)·0.58)
   ≈ -116.52 ml/cm
```

Artinya: di titik itu, **tiap 1 cm penurunan air ≈ 116.5 ml volume berkurang**.

## Output MQTT (v1.4)

```json
{
  "distance_cm": 6.68,
  "volume_ml": 709.0,
  "level_percent": 70.9,
  "differential": -116.52
}
```

## Tabel Hasil

| Distance | Volume | Level | Diferensial |
|----------|--------|-------|-------------|
| 4.19 cm | 1000.0 ml | 100.0% | −11.61 ml/cm |
| 5.00 cm | 975.6 ml | 97.6% | −67.12 ml/cm |
| 5.625 cm | 900.0 ml | 90.0% | −185.84 ml/cm |
| 6.10 cm | 800.0 ml | 80.0% | −202.21 ml/cm |
| **6.68 cm** | **709.0 ml** | **70.9%** | −116.52 ml/cm |
| 8.66 cm | 600.0 ml | 60.0% | −52.55 ml/cm |
| 10.00 cm | 500.0 ml | 50.0% | −80.59 ml/cm |
| 14.00 cm | 200.0 ml | 20.0% | −61.29 ml/cm |
| 15.35 cm | 100.0 ml | 10.0% | −121.86 ml/cm |
| 16.02 cm | 0.0 ml | 0.0% | −162.95 ml/cm |

## Sifat yang Diverifikasi

- ✅ **Lewat tepat** semua 10 titik (error 0.000000 ml)
- ✅ **Gradien kontinu** di tiap knot (selisih kiri-kanan ~1e-14)
- ✅ **Monotonic** — volume selalu turun saat jarak naik (0 violations)
- ✅ **No overshoot** — tetap di rentang 0..1000 ml
- ✅ Clamp: `d ≤ 4.19` → 1000 ml, `d ≥ 16.02` → 0 ml

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
3. Masukkan ke `KNOTS[]` **urut jarak naik**
4. Regenerate `SPLINE[]` (script Python di bawah)
5. Compile & upload: `pio run --target upload`
6. Cek serial monitor: pastikan `volume_ml` sesuai

Titik kalibrasi makin banyak → kurva makin akurat mengikuti bentuk tangki.

### Script Regenerate Koefisien

```bash
python3 docs/tools/gen_spline.py titik_kalibrasi.txt
# Output: tempelan array SPLINE[] + KNOTS[] siap copy ke calibration.h
```

## Multi-Tangki

Firmware dasar sama untuk semua device. Yang beda hanya `calibration.h`:

| Device | Calibration |
|--------|-------------|
| GWT-001 | `calibration.h` (spline GWT-001) |
| GWT-002 | `calibration.h` (spline GWT-002) |

Rencana lanjutan: simpan di **NVS** supaya OTA firmware tidak menghapus kalibrasi.

## Tips

- Pastikan sensor sejajar dengan permukaan air
- Hindari gelombang/air bergerak
- Kalibrasi di suhu yang sama dengan penggunaan
- Pantau `differential` — nilai mendadak lonjak = kemungkinan noise sensor
