# Node-RED Flows

## Flow Structure

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

## Function Node: distance → volume

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

## Deploy

Klik **Deploy** di pojok kanan atas.

## Debug

Tambah debug node setelah function node untuk melihat output.
