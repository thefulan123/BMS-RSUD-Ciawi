# ESP32 Firmware

## Hardware

| Component | Pin |
|-----------|-----|
| HC-SR04 TRIG | GPIO 33 |
| HC-SR04 ECHO | GPIO 14 |
| WiFi | - |
| MQTT | - |

## Dependencies

- WiFiManager 2.0.17
- PubSubClient 2.8.0

## Struktur File

```
firmware/esp32/
├── include/
│   ├── config.h              ← WiFi, MQTT, GPIO, interval
│   ├── config_private.h      ← Password/credential (tidak di-commit)
│   ├── calibration.h         ← 🔥 SEMUA parameter kalibrasi tangki
│   ├── sensor.h              ← Interface + SensorReading struct
│   ├── hcsr04_sensor.h
│   ├── interfaces.h
│   ├── mqtt_connector.h
│   ├── wifi_connector.h
│   └── app.h
├── src/
│   ├── main.cpp              ← Entry point
│   ├── app.cpp               ← Orkestrasi
│   ├── hcsr04_sensor.cpp
│   ├── mqtt_connector.cpp
│   └── wifi_connector.cpp
└── platformio.ini
```

## Data Pipeline (di MCU)

```
HC-SR04 → raw echo (µs) → temperature compensation → calibration table → volume → %
                                                                              ↓
                                                              MQTT: {"distance_cm":13.42,
                                                                     "volume_ml":247.0,
                                                                     "level_percent":24.7}
```

## Kalibrasi

Semua parameter tangki ada di **`include/calibration.h`**. Pindah tangki = ubah 3 angka saja.

```cpp
// Suhu ruang (nanti bisa diganti sensor suhu)
constexpr float TEMPERATURE_C = 16.0f;

// 3 titik kalibrasi — UBAH DI SINI SAJA
constexpr float CAL_D1 = 4.16f;    constexpr float CAL_V1 = 1000.0f;
constexpr float CAL_D2 = 9.58f;    constexpr float CAL_V2 = 700.0f;
constexpr float CAL_D3 = 15.89f;   constexpr float CAL_V3 = 0.0f;

// m & b dihitung otomatis (regresi linear least squares):
//   V(d) = −85.9490·d + 1415.5564
constexpr float CALIBRATION_SLOPE     = ...;
constexpr float CALIBRATION_INTERCEPT = ...;
```

Rumus kompensasi suhu (otomatis dipakai):

```cpp
float speed = 331.3f + (0.606f * temperatureC);  // m/s
float distance_cm = echoUs * speed / 20000.0f;
```

## Configuration

Edit `include/config_private.h`:

```cpp
struct MqttSecrets
{
    const char *user = "bms";
    const char *password = "soke1234";
};
```

## Upload

```bash
cd firmware/esp32
~/.platformio/penv/bin/pio run --target upload
```

## Compile Test

```bash
~/.platformio/penv/bin/pio run
# Output: RAM 13.9%, Flash 66.7% → SUCCESS
```

## Monitor Serial

```bash
~/.platformio/penv/bin/pio device monitor --port /dev/ttyUSB0 --baud 115200
```

## WiFi Setup

1. ESP32 nyalakan → broadcast AP `ESP32-BMS`
2. Connect dari phone/laptop
3. Buka `192.168.4.1`
4. Pilih WiFi lo → isi password
5. ESP32 connect → MQTT publish tiap 2 detik

## OTA Update

1. Buka Arduino IDE
2. Tools → Port → Network → `esp32-bms`
3. Upload via network

## Output

```
==============================
ESP32 BMS MQTT
==============================
WiFi CONNECTED
IP: 192.168.1.100
MQTT: TERHUBUNG
Distance: 13.42 cm | Volume: 247.0 ml | Level: 24.7 %
JSON: {"distance_cm":13.42,"volume_ml":247.0,"level_percent":24.7}
```

## Strategi Multi-Tangki

Firmware dasar sama untuk semua device. Yang beda hanya `calibration.h`:

| Device | Firmware | Calibration |
|--------|----------|-------------|
| GWT-001 | 1.4.0 | `calibration.h` (GWT-001) |
| GWT-002 | 1.4.0 | `calibration.h` (GWT-002) |

Rencana lanjutan: simpan kalibrasi di **NVS** supaya OTA firmware tidak menghapus kalibrasi tangki.
