# BMS RSUD Ciawi

Sistem Battery Management System (BMS) untuk RSUD Ciawi menggunakan ESP32 + sensor ultrasonik HC-SR04 untuk monitoring volume air.

## Arsitektur

```
ESP32 (HC-SR04) → EMQX (MQTT) → Node-RED → InfluxDB → Grafana
```

## Komponen

| Komponen | Fungsi |
|----------|--------|
| ESP32 | Mikrokontroler, baca sensor |
| HC-SR04 | Sensor ultrasonik (jarak → volume) |
| EMQX 6.3.1 | MQTT Broker |
| Node-RED | Data processing & routing |
| InfluxDB 2.7 | Time-series database |
| Grafana | Dashboard & visualisasi |

## Quick Start

### 1. Upload Firmware ESP32

```bash
cd firmware/esp32
# Edit config_private.h dengan credential WiFi & MQTT
pio run --target upload
```

### 2. Jalankan Docker Stack

```bash
cd docker
docker compose up -d
```

### 3. Akses Dashboard

| App | URL | Keterangan |
|-----|-----|------------|
| **Grafana** | http://localhost:3000 | Dashboard monitoring |
| **Node-RED** | http://localhost:1880 | Flow programming |
| **EMQX Dashboard** | http://localhost:18083 | MQTT broker management |
| **Chronograf** | http://localhost:8888 | InfluxDB UI (opsional) |
| **InfluxDB API** | http://localhost:8086 | Database API |
| **Komodo** | http://localhost:9120 | Container management |

## Dokumentasi

- [01 Architecture](docs/01-architecture.md)
- [02 Docker Stack](docs/02-docker-stack.md)
- [03 MQTT Config](docs/03-mqtt-config.md)
- [04 ESP32 Firmware](docs/04-esp32-firmware.md)
- [05 Node-RED Flows](docs/05-nodered-flows.md)
- [06 Calibration](docs/06-calibration.md)
- [07 Troubleshooting](docs/07-troubleshooting.md)
- [08 Volume Calculation](docs/08-volume-calculation.md)
- [09 Docker Volume Migration](docs/09-docker-volume-migration.md)

## Credentials

| Service | Username | Password |
|---------|----------|----------|
| Grafana | admin | soke123 |
| InfluxDB | admin | soke123 |
| EMQX | admin | public |
| MQTT | bms | soke1234 |

## License

MIT
