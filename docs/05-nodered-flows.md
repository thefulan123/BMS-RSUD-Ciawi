# Node-RED Flows

## Flow Structure

### v1.2 (aktif) — firmware sudah hitung volume di MCU

```
[MQTT in: bms/gwt1] → [Function: parse & flatten] → [InfluxDB out]
```

Node-RED **tidak perlu** konversi jarak → volume lagi, karena ESP32 sudah mengirim
`distance_cm`, `volume_ml`, dan `level_percent` langsung.

### v1.1 (legacy) — konversi di Node-RED

```
[MQTT in: bms/gwt1] → [Function: distance→volume] → [InfluxDB out]
```

## MQTT In Node

| Field | Value |
|-------|-------|
| Server | emqx |
| Port | 1883 |
| Topic | bms/gwt1 |
| Username | bms |
| Password | soke1234 |

## Function Node (v1.2)

```javascript
// Firmware v1.2 sudah hitung semua di MCU.
// Input: {distance_cm:13.42, volume_ml:247.0, level_percent:24.7}

var p = msg.payload;

// Handle string (beberapa broker kirim JSON sebagai string)
if (typeof p === "string") {
    try { p = JSON.parse(p); } catch (e) {
        node.error("JSON parse error: " + e.message, msg);
        return null;
    }
}

if (p.distance_cm === undefined) {
    node.error("Payload tidak punya distance_cm", msg);
    return null;
}

msg.payload = {
    distance_cm: Number(p.distance_cm),
    volume_ml: Number(p.volume_ml || 0),
    level_percent: Number(p.level_percent || 0)
};

return msg;
```

## Function Node (v1.1 legacy)

```javascript
var d = msg.payload.distance_cm;

var volume;
if (d >= 16.30) {
    volume = 0;
} else if (d >= 15.98) {
    volume = (16.30 - d) / (16.30 - 15.98) * 100;
} else if (d >= 5.13) {
    volume = 100 + (15.98 - d) / (15.98 - 5.13) * 900;
} else {
    volume = 1000;
}

msg.payload = {
    distance_cm: d,
    volume_ml: Math.round(volume * 100) / 100
};

return msg;
```

## InfluxDB Out Node

| Field | Value |
|-------|-------|
| URL | http://influxdb:8086 |
| Database | iot |
| Measurement | sensor_data |

## Import Flow

```bash
# Copy flow ke container
docker cp nodered/flows-v1.2.json nodered:/data/flows.json
docker restart nodered
```

Atau lewat UI: menu kanan atas → **Import** → pilih file JSON.

## Deploy

Klik **Deploy** di pojok kanan atas.

## Debug

Tambah debug node setelah function node untuk melihat output.
