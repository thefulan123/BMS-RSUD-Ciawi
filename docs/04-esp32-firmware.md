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
HC-SR04 → raw echo (µs) → temperature compensation → cubic spline → volume → % → differential
                                                                                    ↓
                                                  MQTT: {"distance_cm":6.68,
                                                         "volume_ml":709.0,
                                                         "level_percent":70.9,
                                                         "differential":-116.52}
```

## Kalibrasi

Semua parameter tangki ada di **`include/calibration.h`**.
Pindah tangki = ganti `KNOTS[]` + regenerate `SPLINE[]`.

```cpp
// Suhu ruang (nanti bisa diganti sensor suhu)
constexpr float TEMPERATURE_C = 16.0f;

// Titik kalibrasi (urut jarak naik) + diferensial (ml/cm) di tiap titik
constexpr CalibrationKnot KNOTS[] = {
    { 4.190f, 1000.0f,  -11.6072f},
    { 5.625f,  900.0f, -185.8448f},
    // ... lihat file lengkap
    {16.020f,    0.0f, -162.9501f}
};

// Koefisien natural cubic spline per interval:
//   V(t) = a + b·t + c·t² + d·t³
// (regenerate dengan: python3 docs/tools/gen_spline.py)
constexpr SplineSegment SPLINE[] = {
    {1000.0000f, -11.6072f, 0.000000f, -28.204384f},
    // ...
};
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
Distance: 6.68 cm | Volume: 709.0 ml | Level: 70.9 % | Diff: -116.52 ml/cm
JSON: {"distance_cm":6.68,"volume_ml":709.0,"level_percent":70.9,"differential":-116.52}
```

## Strategi Multi-Tangki

Firmware dasar sama untuk semua device. Yang beda hanya `calibration.h`:

| Device | Firmware | Calibration |
|--------|----------|-------------|
| GWT-001 | 1.4.0 | `calibration.h` (GWT-001) |
| GWT-002 | 1.4.0 | `calibration.h` (GWT-002) |

Rencana lanjutan: simpan kalibrasi di **NVS** supaya OTA firmware tidak menghapus kalibrasi tangki.
