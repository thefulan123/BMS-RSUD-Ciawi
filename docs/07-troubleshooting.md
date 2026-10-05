# Troubleshooting

## ESP32 ga connect WiFi

| Penyebab | Solusi |
|----------|--------|
| Password salah | Cek config_private.h |
| WiFi 5GHz | Ganti ke 2.4GHz |
| Sinyal lemah | Dekatkan ke router |

## MQTT gagal connect

| Penyebab | Solusi |
|----------|--------|
| Broker mati | `docker ps \| grep emqx` |
| Port blocked | `sudo ufw allow 1883` |
| Auth salah | Cek username/password |

## Node-RED ga terima data

| Penyebab | Solusi |
|----------|--------|
| Topic salah | Cek di MQTT node |
| Flow belum deploy | Klik Deploy |
| Debug mati | Aktifkan debug node |

## InfluxDB ga nyimpen data

| Penyebab | Solusi |
|----------|--------|
| Volume hilang | Cek `docker volume ls` |
| DB salah | Cek database name |
| Token expired | Buat token baru |

## Grafana kosong

| Penyebab | Solusi |
|----------|--------|
| Data belum masuk | Cek InfluxDB |
| Query salah | Cek time range |
| Panel error | Edit panel → Refresh |

## Docker Compose Error

```bash
# Cek status
docker compose ps

# Cek logs
docker compose logs

# Restart
docker compose restart

# Rebuild
docker compose down && docker compose up -d
```
