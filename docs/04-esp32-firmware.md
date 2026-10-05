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
pio run --target upload
```

## Monitor Serial

```bash
pio device monitor --port /dev/ttyUSB0 --baud 115200
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
WiFi CONNECTED
IP: 192.168.1.100
MQTT: TERHUBUNG
Distance: 14.63 cm
JSON: {"distance_cm":14.63}
```
