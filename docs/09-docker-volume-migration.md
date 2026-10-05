# Docker Volume — Setup, Backup & Migrasi

Tutorial lengkap: cara bikin data Docker **tidak kehapus**, backup, dan pindah ke PC lain.

> **Penting:** Data Docker (InfluxDB, Grafana, Node-RED, EMQX) ada di **volume**,
> bukan di container. Kalau container dihapus/rebuild, data aman.
> Tapi kalau **volume** yang dihapus — data hilang permanen.

## 1. Daftar Volume di Stack Ini

```bash
docker volume ls
```

| Volume | Isi | Path di Container |
|--------|-----|-------------------|
| `iot-stack_influxdb-data` | Data time-series | `/var/lib/influxdb2` |
| `iot-stack_grafana-data` | Dashboard & user | `/var/lib/grafana` |
| `iot-stack_nodered-data` | Flows Node-RED | `/data` |
| `iot-stack_emqx-data` | User & message MQTT | `/opt/emqx/data` |
| `iot-stack_mongodb-data` | Data MongoDB | `/data/db` |
| `iot-stack_komodo-data` | Data Komodo | `/data` |

Cek isi volume:

```bash
docker volume inspect iot-stack_influxdb-data
# Lihat "Mountpoint": /var/lib/docker/volumes/iot-stack_influxdb-data/_data
```

## 2. Setting Volume di docker-compose (Biar Data Gak Kehapus)

Kuncinya ada di bagian `volumes:` di **bawah** `docker-compose.yml`:

```yaml
services:
  influxdb:
    image: influxdb:2.7
    volumes:
      - influxdb-data:/var/lib/influxdb2   # ← format: nama:/path
    restart: always

  grafana:
    image: grafana/grafana
    volumes:
      - grafana-data:/var/lib/grafana
    restart: always

# ── Deklarasi named volume ──
volumes:
  influxdb-data:    # Docker buat & manage sendiri
  grafana-data:
  nodered-data:
  emqx-data:
  mongodb-data:
  komodo-data:
```

### 3 Jenis Volume & Bedanya

| Tipe | Contoh | Aman dari `docker compose down`? |
|------|--------|----------------------------------|
| **Named volume** ✅ | `- influxdb-data:/var/lib/influxdb2` | ✅ Aman |
| **Bind mount** | `- /home/soke/data:/var/lib/influxdb2` | ✅ Aman (data di path lo) |
| **Anonymous volume** | `- /var/lib/influxdb2` (tanpa nama) | ❌ Rawan kehapus |

**Selalu pakai named volume atau bind mount.** Jangan pernah pakai anonymous.

### Cara Cek Named Volume Benar

```bash
# Lihat volume yang dipakai container
docker inspect influxdb --format '{{json .Mounts}}' | jq

# Hasil yang benar:
# {"Type":"volume","Name":"iot-stack_influxdb-data", ...}
# Bukan {"Type":"volume","Name":"<random-hex>", ...}  ← anonymous, bahaya
```

### Perintah yang AMAN vs BAHAYA

```bash
# ✅ AMAN — container saja, volume tetap
docker compose down
docker compose down --remove-orphans
docker compose restart
docker compose up -d --force-recreate
docker compose pull && docker compose up -d

# ⚠️ HATI-HATI — ikut hapus volume
docker compose down -v          # -v = hapus named volume!
docker volume prune             # hapus semua volume tak terpakai
docker system prune -af --volumes  # BAHAYA: hapus semua
```

> **Aturan emas:** Jangan pernah pakai `docker compose down -v`
> kecuali lo yakin mau hapus data permanen.

### Auto-Backup Pakai Cron

```bash
mkdir -p ~/backup-docker

# Edit crontab
crontab -e
```

Tambah baris ini (backup tiap jam 2 malam):

```cron
0 2 * * * /home/soke/backup-docker/backup-volumes.sh >> /home/soke/backup-docker/backup.log 2>&1
```

Isi `backup-volumes.sh`:

```bash
#!/bin/bash
set -e

BACKUP_DIR="$HOME/backup-docker/$(date +%Y-%m-%d)"
mkdir -p "$BACKUP_DIR"

for vol in $(docker volume ls -q | grep iot-stack); do
  docker run --rm -v "${vol}:/from:ro" -v "${BACKUP_DIR}:/to" alpine \
    sh -c "cd /from && tar czf /to/${vol}.tar.gz ."
  echo "OK: ${vol}"
done

# Simpan 14 hari terakhir saja
find "$HOME/backup-docker" -maxdepth 1 -type d -mtime +14 -exec rm -rf {} +
```

```bash
chmod +x ~/backup-docker/backup-volumes.sh
```

## 3. Backup Volume

### Backup semua volume

```bash
mkdir -p ~/backup

for vol in $(docker volume ls -q | grep iot-stack); do
  docker run --rm -v ${vol}:/from -v ~/backup:/to alpine \
    sh -c "cd /from && tar czf /to/${vol}-backup.tar.gz ."
done

ls -lh ~/backup/
```

### Backup 1 volume saja

```bash
docker run --rm -v iot-stack_influxdb-data:/from -v $(pwd):/to alpine \
  sh -c "cd /from && tar czf /to/influxdb-backup.tar.gz ."
```

## 4. Restore Volume

```bash
for file in ~/backup/*-backup.tar.gz; do
  vol=$(basename "$file" -backup.tar.gz)
  docker volume create "$vol"
  docker run --rm -v "$vol":/to -v ~/backup:/from alpine \
    sh -c "cd /to && tar xzf /from/$(basename "$file")"
done
```

> **Note:** Stop container dulu sebelum restore supaya data tidak bentrok:
> `docker compose stop influxdb grafana` → restore → `docker compose start ...`

## 5. Migrasi ke PC Lain

### Langkah 1 — Stop stack di PC lama

```bash
cd ~/iot-stack
docker compose stop
```

### Langkah 2 — Backup semua volume

```bash
mkdir -p ~/backup-migrasi

for vol in $(docker volume ls -q | grep iot-stack); do
  docker run --rm -v ${vol}:/from -v ~/backup-migrasi:/to alpine \
    sh -c "cd /from && tar czf /to/${vol}.tar.gz ."
done

ls -lh ~/backup-migrasi/
```

### Langkah 3 — Copy ke PC baru

```bash
# Via SSH
scp -r ~/backup-migrasi user@pc-baru:~/backup-migrasi

# Atau via USB/rsync
rsync -av ~/backup-migrasi/ /media/usb/backup-migrasi/
```

### Langkah 4 — Install Docker di PC baru

```bash
curl -fsSL https://get.docker.com | sh
sudo usermod -aG docker $USER
newgrp docker
docker compose version
```

### Langkah 5 — Clone repo & jalankan stack sekali

```bash
git clone https://github.com/thefulan123/BMS-RSUD-Ciawi.git
cd BMS-RSUD-Ciawi/docker
docker compose up -d
# Tunggu semua container running, lalu STOP
docker compose stop
```

Ini penting — supaya Docker membuat named volume dengan nama yang benar
(`iot-stack_*`) sebelum kita timpa dengan data backup.

### Langkah 6 — Restore volume

```bash
for file in ~/backup-migrasi/*.tar.gz; do
  vol=$(basename "$file" .tar.gz)
  docker volume create "$vol"
  docker run --rm -v "$vol":/to -v ~/backup-migrasi:/from alpine \
    sh -c "cd /to && tar xzf /from/$(basename "$file")"
  echo "Restored: $vol"
done
```

### Langkah 7 — Start stack & verifikasi

```bash
docker compose up -d
docker compose ps
```

Cek data:

| Cek | URL / Command |
|-----|---------------|
| Grafana | http://localhost:3000 → dashboard lama harus muncul |
| Node-RED | http://localhost:1880 → flows lama harus ada |
| EMQX | http://localhost:18083 → user `bms` harus ada |
| InfluxDB | `docker exec influxdb influx bucket list` |

### Troubleshooting Migrasi

| Masalah | Solusi |
|---------|--------|
| "volume already exists" | Normal, lanjut — tar akan timpa isi |
| Data tidak muncul | Cek nama volume: `docker volume ls` harus `iot-stack_*` |
| Permission denied | `sudo chown -R 999: /var/lib/docker/volumes/iot-stack_influxdb-data` |
| Container restart loop | `docker compose logs <service>` |

## 6. Cheat Sheet Perintah

```bash
# Lihat
docker volume ls                          # daftar volume
docker volume ls -q | grep iot-stack      # volume stack ini
docker volume inspect <nama>              # detail + mountpoint
docker system df -v                       # ukuran tiap volume

# Yang aman
docker compose down                       # stop + hapus container (volume AMAN)
docker compose up -d                      # start ulang

# Yang bahaya (cek dulu!)
docker volume rm <nama>                   # hapus 1 volume
docker volume prune                       # hapus volume tak terpakai
docker compose down -v                    # hapus container + volume!
```

## 7. Checklist Safety

- [ ] Selalu `docker volume ls` sebelum `prune`
- [ ] Jangan pakai `docker compose down -v` tanpa backup
- [ ] Auto-backup via cron (lihat bagian 2)
- [ ] Backup di tempat terpisah (USB / cloud)
- [ ] Test restore ke PC/VM sebelum butuh beneran
- [ ] `docker inspect` untuk pastikan pakai named volume, bukan anonymous
