# Versioning Firmware

Repo ini menyimpan versi-vensi firmware ESP32 supaya mudah dibandingkan dan di-rollback.

## Perbandingan

| Versi | Folder | Output MQTT | Konversi Volume | Rumus | Status |
|-------|--------|-------------|-----------------|-------|--------|
| **v1.4** | `firmware/esp32/` | `distance_cm`, `volume_ml`, `level_percent`, `differential` | **MCU** | natural cubic spline | ✅ Aktif |
| v1.3 | (git history) | `distance_cm`, `volume_ml`, `level_percent` | **MCU** | regresi linear 3 titik | Legacy |
| v1.2 | `firmware/esp32/v1.2-time/` | `{"echo_us":940}` | Node-RED | piecewise | Legacy |
| v1.1 | `firmware/esp32/v1.1-distance/` | `{"distance_cm":13.42}` | Node-RED | piecewise | Legacy |

## v1.1 — Kirim Jarak

Firmware baca HC-SR04 → hitung jarak (cm) → publish JSON.

```
ESP32 → distance_cm → MQTT → Node-RED (distance→volume) → InfluxDB
```

Flow: `nodered/flows-v1.1.json`

## v1.2 — Kirim Waktu (Echo)

Firmware publish echo time mentah (µs). Konversi suhu + jarak + volume
dilakukan di Node-RED.

```
ESP32 → echo_us → MQTT → Node-RED (time→distance→volume) → InfluxDB
```

Kelebihan: bisa ganti rumus kalibrasi tanpa OTA firmware.
Kekurangan: dependensi Node-RED, kalibrasi tersebar di 2 tempat.

## v1.3 — Regresi Linear di MCU

Volume dihitung di MCU dengan satu garis dari 3 titik:

```
V(d) = −85.9490·d + 1415.5564
```

Kelebihan: paling simpel (1 persamaan).
Kekurangan: gak lewat tepat tiap titik (9.58 cm ukur 700 ml → prediksi 592 ml).

Sudah digantikan v1.4. Rollback: `git show ee13943:firmware/esp32/include/calibration.h`

## v1.4 — Natural Cubic Spline (Aktif)

Seluruh pipeline di firmware, kurva halus yang **lewat tepat semua titik**:

```
HC-SR04 → raw echo → temperature compensation → cubic spline → volume → % → differential
                                                                          ↓
                                                        MQTT lengkap → Node-RED → InfluxDB
```

**File kunci: `firmware/esp32/include/calibration.h`**

- `TEMPERATURE_C` — suhu ruang, untuk kompensasi kecepatan suhu
- `KNOTS[]` — titik kalibrasi + **diferensial (ml/cm) di tiap titik**
- `SPLINE[]` — koefisien a,b,c,d per interval
- `Calibration::echoUsToDistance()` — rumus suhu
- `Calibration::distanceToVolume()` — `V = a + b·t + c·t² + d·t³` (Horner)
- `Calibration::distanceToSlope()` — diferensial/gradien di jarak manapun
- `Calibration::volumeToPercent()` — level %

Flow: `nodered/flows-v1.2.json` (tinggal parse & forward)

### Kenapa pindah ke MCU?

1. **Kalibrasi di 1 tempat** — pindah tangki = edit `calibration.h` saja
2. **Node-RED tipis** — hanya routing, gampang diganti/direplace
3. **Data utuh di MQTT** — consumer lain (HA, script Python) langsung dapat volume
4. **Siap multi-device** — 10 tangki, firmware sama, beda `calibration.h`

### Payload v1.4

```json
{
  "distance_cm": 6.68,
  "volume_ml": 709.0,
  "level_percent": 70.9,
  "differential": -116.52
}
```

### Evolusi rumus kalibrasi

| | v1.1/v1.2 | v1.3 | v1.4 |
|---|---|---|---|
| Rumus | piecewise (Node-RED) | 1 garis regresi | spline 9 interval |
| Lewat tepat titik | ✅ | ❌ | ✅ |
| Gradien kontinu | ❌ (siku) | ✅ | ✅ |
| Halus / analog | ❌ | ❌ | ✅ |
| Diferensial per titik | ❌ | 1 nilai (m) | ✅ per titik |

## Rencana v1.5

- Simpan kalibrasi di **NVS** → OTA firmware tidak menghapus kalibrasi tangki
- Tambah sensor suhu aktual → hilangkan `TEMPERATURE_C` hardcoded
- Filter moving average di firmware (kurangi noise)
- Regenerate spline on-device ( Python script pindah ke build hook)

## Cara Pindah Versi

```bash
# Pakai v1.4 (aktif)
cd firmware/esp32 && pio run --target upload

# Rollback ke v1.3 (regresi linear)
git show ee13943:firmware/esp32/include/calibration.h > firmware/esp32/include/calibration.h
pio run --target upload

# Rollback ke v1.2 (konversi di Node-RED)
cd firmware/esp32/v1.2-time && pio run --target upload
# + import nodered/flows-v1.2.json (versi lama)
```
