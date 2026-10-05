# Docker Stack

## Services

| Service | Image | Ports |
|---------|-------|-------|
| mongodb | mongo:latest | 27017 |
| komodo-core | ghcr.io/moghtech/komodo-core:latest | 9120 |
| emqx | emqx/emqx:6.3.1 | 1883, 18083 |
| nodered | nodered/node-red:latest | 1880 |
| influxdb | influxdb:2.7 | 8086 |
| grafana | grafana/grafana:latest | 3000 |
| chronograf | chronograf:latest | 8888 |

## Commands

### Start
```bash
cd ~/iot-stack
docker compose up -d
```

### Stop
```bash
docker compose stop
```

### Restart
```bash
docker compose restart
```

### Logs
```bash
docker compose logs -f [service]
```

### Status
```bash
docker compose ps
```

### Update Images
```bash
docker compose pull
docker compose up -d
```

## Volumes

| Volume | Path | Isi |
|--------|------|-----|
| `iot-stack_mongodb-data` | /data/db | Data MongoDB |
| `iot-stack_komodo-data` | /data | Data Komodo |
| `iot-stack_emqx-data` | /opt/emqx/data | Data EMQX |
| `iot-stack_nodered-data` | /data | Flows Node-RED |
| `iot-stack_influxdb-data` | /var/lib/influxdb2 | Data InfluxDB |
| `iot-stack_grafana-data` | /var/lib/grafana | Dashboard Grafana |

## Environment

| Variable | Value |
|----------|-------|
| TZ | Asia/Jakarta |
| INFLUXDB_ADMIN_USER | admin |
| INFLUXDB_ADMIN_PASSWORD | soke123 |
| GF_SECURITY_ADMIN_USER | admin |
| GF_SECURITY_ADMIN_PASSWORD | soke123 |
