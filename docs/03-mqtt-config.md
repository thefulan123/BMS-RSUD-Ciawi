# MQTT Configuration

## Broker: EMQX

| Setting | Value |
|---------|-------|
| Host | broker.avisha.id |
| Port | 1883 |
| Username | bms |
| Password | soke1234 |

## Topics

| Topic | Direction | Format |
|-------|-----------|--------|
| `bms/gwt1` | ESP32 → Broker | JSON |

## Payload Format

### v1.2 (aktif) — firmware hitung di MCU

```json
{
  "distance_cm": 13.42,
  "volume_ml": 247.0,
  "level_percent": 24.7
}
```

### v1.1 (legacy)

```json
{
  "echo_us": 880,
  "temperature_c": 16
}
```

## Test Publish

```bash
mosquitto_pub -h broker.avisha.id -p 1883 -t "bms/gwt1" \
  -m '{"distance_cm":13.42,"volume_ml":247.0,"level_percent":24.7}' \
  -u bms -P soke1234
```

## Test Subscribe

```bash
mosquitto_sub -h broker.avisha.id -p 1883 -t "bms/gwt1" -u bms -P soke1234
```

## Node-RED MQTT Config

| Field | Value |
|-------|-------|
| Server | emqx (atau broker.avisha.id) |
| Port | 1883 |
| Topic | bms/gwt1 |
| QoS | 0 |
| Username | bms |
| Password | soke1234 |
