# Architecture

## Diagram Sistem

```
┌─────────────┐     MQTT      ┌─────────┐     HTTP     ┌──────────┐
│   ESP32     │ ─────────────→│  EMQX   │─────────────→│ Node-RED │
│  HC-SR04    │   bms/gwt1    │  :1883  │              │  :1880   │
└─────────────┘               └─────────┘              └────┬─────┘
                                                            │
                                                            ↓
                                                      ┌──────────┐
                                                      │ InfluxDB │
                                                      │  :8086   │
                                                      └────┬─────┘
                                                           │
                                                           ↓
                                                      ┌──────────┐
                                                      │ Grafana  │
                                                      │  :3000   │
                                                      └──────────┘
```

## Flow Data

1. **ESP32** membaca jarak dari HC-SR04 (cm)
2. **ESP32** publish JSON ke topic `bms/gwt1`
3. **EMQX** menerima dan mendistribusikan message
4. **Node-RED** subscribe, parse JSON, convert ke volume
5. **Node-RED** write ke InfluxDB
6. **Grafana** query InfluxDB dan tampilkan dashboard

## Network

Semua container dalam satu Docker network `iot-stack_default`, bisa saling komunikasi via service name.

## Ports

| Port | Service |
|------|---------|
| 1883 | MQTT (TCP) |
| 8883 | MQTT (SSL) |
| 18083 | EMQX Dashboard |
| 1880 | Node-RED |
| 8086 | InfluxDB |
| 3000 | Grafana |
| 8888 | Chronograf |
