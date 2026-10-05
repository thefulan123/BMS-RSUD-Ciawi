# Docker Volume Migration

## Daftar Volume

| Volume | Isi |
|--------|-----|
| `iot-stack_mongodb-data` | Data MongoDB |
| `iot-stack_komodo-data` | Data Komodo |
| `iot-stack_emqx-data` | Data EMQX |
| `iot-stack_nodered-data` | Flows Node-RED |
| `iot-stack_influxdb-data` | Data InfluxDB |
| `iot-stack_grafana-data` | Dashboard Grafana |

## Backup Volume

```bash
# Backup semua volume
for vol in $(docker volume ls -q | grep iot-stack); do
  docker run --rm -v ${vol}:/from -v ~/backup:/to alpine \
    sh -c "cd /from && tar czf /to/${vol}-backup.tar.gz ."
done
```

## Restore Volume

```bash
# Restore semua volume
for file in ~/backup/*-backup.tar.gz; do
  vol=$(echo $file | sed 's/.*\///' | sed 's/-backup.tar.gz//')
  docker volume create ${vol}
  docker run --rm -v ${vol}:/to -v ~/backup:/from alpine \
    sh -c "cd /to && tar xzf /from/$(basename $file)"
done
```

## Migrasi ke PC Lain

### 1. Backup di PC Lama

```bash
mkdir -p ~/backup
for vol in $(docker volume ls -q | grep iot-stack); do
  docker run --rm -v ${vol}:/from -v ~/backup:/to alpine \
    sh -c "cd /from && tar czf /to/${vol}-backup.tar.gz ."
done
```

### 2. Copy ke PC Baru

```bash
scp -r ~/backup user@pc-baru:~/
```

### 3. Restore di PC Baru

```bash
for file in ~/backup/*-backup.tar.gz; do
  vol=$(echo $file | sed 's/.*\///' | sed 's/-backup.tar.gz//')
  docker volume create ${vol}
  docker run --rm -v ${vol}:/to -v ~/backup:/from alpine \
    sh -c "cd /to && tar xzf /from/$(basename $file)"
done
```

## Cek Volume

```bash
# List semua volume
docker volume ls

# Inspect volume
docker volume inspect iot-stack_influxdb-data

# Cek ukuran
docker system df -v
```

## Hapus Volume (Hati-hati!)

```bash
# Hapus volume specific
docker volume rm iot-stack_influxdb-data

# Hapus semua volume tidak terpakai
docker volume prune
```
