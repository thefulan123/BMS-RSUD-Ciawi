#!/usr/bin/env python3
"""
Generator koefisien natural cubic spline untuk kalibrasi tangki BMS.

Pakai:
    python3 gen_spline.py                     # pakai tabel default GWT-001
    python3 gen_spline.py titik.txt           # dari file (baris: jarak volume)

Format file input (satu titik per baris, urut jarak naik):
    4.19 1000
    5.625 900
    ...

Output: tempelan array C++ untuk calibration.h
       (KNOTS[] dengan slope + SPLINE[] dengan koefisien a,b,c,d)
"""

import sys

# Tabel default GWT-001 (distance_cm, volume_ml)
DEFAULT_POINTS = [
    (4.19, 1000), (5.625, 900), (6.10, 800), (8.66, 600), (10.00, 500),
    (11.34, 400), (12.66, 300), (14.00, 200), (15.35, 100), (16.02, 0),
]


def load_points(path):
    pts = []
    with open(path) as f:
        for line in f:
            line = line.split("#")[0].strip()
            if not line:
                continue
            d, v = line.split()[:2]
            pts.append((float(d), float(v)))
    return pts


def natural_cubic_spline(pts):
    """Koefisien natural cubic spline (M0 = Mn = 0).

    Return (knots_x, knots_y, slopes, segments)
    segments[i] = (a, b, c, d) untuk V(t) = a + b t + c t^2 + d t^3
    """
    x = [p[0] for p in pts]
    y = [p[1] for p in pts]
    n = len(pts) - 1
    if n < 1:
        raise ValueError("minimal 2 titik")
    h = [x[i + 1] - x[i] for i in range(n)]

    # M = turunan kedua, M0 = Mn = 0 (natural)
    m = [0.0] * (n + 1)
    if n > 1:
        N = n - 1
        cp = [0.0] * N
        dp = [0.0] * N
        for i in range(1, n):
            A, B, C = h[i - 1], 2 * (h[i - 1] + h[i]), h[i]
            D = 6 * ((y[i + 1] - y[i]) / h[i] - (y[i] - y[i - 1]) / h[i - 1])
            if i == 1:
                cp[0], dp[0] = C / B, D / B
            else:
                den = B - A * cp[i - 2]
                cp[i - 1] = C / den
                dp[i - 1] = (D - A * dp[i - 2]) / den
        m[n - 1] = dp[N - 1]
        for i in range(n - 2, -1, -1):
            m[i + 1] = dp[i] - cp[i] * m[i + 2]

    segs = []
    for i in range(n):
        a = y[i]
        b = (y[i + 1] - y[i]) / h[i] - h[i] * (2 * m[i] + m[i + 1]) / 6
        c = m[i] / 2
        d = (m[i + 1] - m[i]) / (6 * h[i])
        segs.append((a, b, c, d))

    # Gradien (diferensial) di tiap knot — kontinu kiri/kanan
    slopes = []
    for i in range(n + 1):
        if i < n:
            slopes.append(segs[i][1])
        else:
            a, b, c, d = segs[-1]
            t = h[-1]
            slopes.append(b + 2 * c * t + 3 * d * t * t)

    return x, y, slopes, segs


def main():
    pts = load_points(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_POINTS

    # Validasi: jarak naik, volume turun
    for i in range(1, len(pts)):
        if pts[i][0] <= pts[i - 1][0]:
            sys.exit(f"ERROR: jarak harus NAIK — baris {i + 1} ({pts[i][0]})")
        if pts[i][1] > pts[i - 1][1]:
            sys.exit(f"WARNING: volume NAIK di baris {i + 1} — cek data")

    x, y, slopes, segs = natural_cubic_spline(pts)

    print("// --- copy ke calibration.h: KNOTS[] ---")
    print("// distance_cm, volume_ml, slope (ml/cm)")
    print("constexpr CalibrationKnot KNOTS[] = {")
    for xi, yi, si in zip(x, y, slopes):
        print(f"    {{{xi:.3f}f, {yi:.1f}f, {si:.4f}f}},")
    print("};\n")

    print("// --- copy ke calibration.h: SPLINE[] ---")
    print("// a, b, c, d")
    print("constexpr SplineSegment SPLINE[] = {")
    for i, (a, b, c, d) in enumerate(segs):
        print(f"    {{{a:.4f}f, {b:.4f}f, {c:.6f}f, {d:.6f}f}},"  #
              f" // {x[i]:.3f} -> {x[i + 1]:.3f}")
    print("};")

    # Verifikasi: lewat tepat semua titik
    def V(d):
        if d <= x[0]:
            return y[0]
        if d >= x[-1]:
            return y[-1]
        for i in range(len(segs)):
            if d <= x[i + 1]:
                t = d - x[i]
                a, b, c, dd = segs[i]
                return a + t * (b + t * (c + t * dd))
        return 0.0

    err = max(abs(V(xi) - yi) for xi, yi in zip(x, y))
    print(f"\n// verifikasi: max error di titik = {err:.6f} ml "
          f"({'OK' if err < 1e-6 else 'CEK LAGI'})")


if __name__ == "__main__":
    main()
