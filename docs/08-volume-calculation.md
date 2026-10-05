# Volume Calculation

## Data Kalibrasi

| Volume | Distance |
|--------|----------|
| 0 ml | 16.30 cm |
| 100 ml | 15.98 cm |
| 200 ml | 14.65 cm |
| 1000 ml | 5.13 cm |

## Fungsi Piecewise Linear

```javascript
function distanceToVolume(d) {
    if (d >= 16.30) return 0;
    if (d >= 15.98) return (16.30 - d) / (16.30 - 15.98) * 100;
    if (d >= 5.13)  return 100 + (15.98 - d) / (15.98 - 5.13) * 900;
    return 1000;
}
```

## Temperature Compensation

Kecepatan suara berubah terhadap suhu:

```javascript
var T = msg.payload.temperature_c || 16;
var v = 331.3 + (0.606 * T); // m/s
var distance_cm = echo_us * v / 20000;
```

## Contoh Perhitungan

| Distance | Volume |
|----------|--------|
| 16.30 cm | 0 ml |
| 15.98 cm | 100 ml |
| 14.65 cm | 200 ml |
| 10.00 cm | ~570 ml |
| 5.13 cm | 1000 ml |

## Node-RED Function Node

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
