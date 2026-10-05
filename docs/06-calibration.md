# Sensor Calibration

## Data Kalibrasi

| Volume | Distance | Section |
|--------|----------|---------|
| 0 ml | 16.30 cm | Mengerucut |
| 100 ml | 15.98 cm | Mengerucut |
| 200 ml | 14.65 cm | Linear |
| 1000 ml | 5.13 cm | Linear |

## Fungsi Piecewise

```
0-100 ml:     mengerucut (tapered)
100-1000 ml:  linear
```

## Rumus

```javascript
if (d >= 16.30) → 0 ml
if (d >= 15.98) → (16.30 - d) / 0.32 * 100
if (d >= 5.13)  → 100 + (15.98 - d) / 10.85 * 900
if (d < 5.13)   → 1000 ml
```

## Cara Kalibrasi Ulang

1. Kosongkan wadah → catat distance
2. Isi 100 ml → catat distance
3. Isi 200 ml → catat distance
4. Isi 1000 ml → catat distance
5. Update tabel di function node

## Tips

- Pastikan sensor sejajar dengan permukaan air
- Hindari gelombang/air bergerak
- Kalibrasi di suhu yang sama dengan penggunaan
