# Versioning Firmware

Repo ini menyimpan 3 versi firmware ESP32 supaya mudah dibandingkan dan di-rollback.

## Perbandingan

| Versi | Folder | Output MQTT | Konversi Volume | Status |
|-------|--------|-------------|-----------------|--------|
| **v1.3** | `firmware/esp32/` | `{"distance_cm":13.42,"volume_ml":247.0,"level_percent":24.7}` | **MCU** (`calibration.h`) | ✅ Aktif |
| v1.2 | `firmware/esp32/v1.2-time/` | `{"echo_us":940}` | Node-RED | Legacy |
| v1.1 | `firmware/esp32/v1.1-distance/` | `{"distance_cm":13.42}` | Node-RED | Legacy |

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

Flow: `nodered/flows-v1.2.json` (versi lama)

Kelebihan: bisa ganti rumus kalibrasi tanpa OTA firmware.
Kekurangan: dependensi Node-RED, kalibrasi tersebar di 2 tempat.

## v1.3 — Hitung di MCU (Aktif)

Seluruh pipeline dipindah ke firmware:

```
HC-SR04 → raw echo → temperature compensation → calibration table → volume → %
                                                                          ↓
                                                        MQTT lengkap → Node-RED → InfluxDB
```

**File kunci: `firmware/esp32/include/calibration.h`**

- `TEMPERATURE_C` — suhu ruang, untuk kompensasi kecepatan suhu
- `CALIBRATION_MAX_VOLUME` / `MIN_DISTANCE` / `MAX_DISTANCE` — 3 parameter tangki
- `Calibration::echoUsToDistance()` — rumus suhu
- `Calibration::distanceToVolume()` — linear global `V = m·d + b` (auto-calculated)
- `Calibration::volumeToPercent()` — level %

Flow: `nodered/flows-v1.2.json` (versi baru — tinggal parse & forward)

### Kenapa pindah ke MCU?

1. **Kalibrasi di 1 tempat** — pindah tangki = edit `calibration.h` saja
2. **Node-RED tipis** — hanya routing, gampang diganti/direplace
3. **Data utuh di MQTT** — consumer lain (HA, script Python) langsung dapat volume
4. **Siap multi-device** — 10 tangki, firmware sama, beda `calibration.h`

### Payload v1.3

```json
{
  "distance_cm": 13.42,
  "volume_ml": 247.0,
  "level_percent": 24.7
}
```

## Rencana v1.4

- Simpan kalibrasi di **NVS** → OTA firmware tidak menghapus kalibrasi tangki
- Tambah sensor suhu aktual → hilangkan `TEMPERATURE_C` hardcoded
- Filter moving average di firmware (kurangi noise)

## Cara Pindah Versi

```bash
# Pakai v1.3 (aktif)
cd firmware/esp32 && pio run --target upload

# Rollback ke v1.2
cd firmware/esp32/v1.2-time && pio run --target upload
# + import nodered/flows-v1.2.json (versi lama)
```
