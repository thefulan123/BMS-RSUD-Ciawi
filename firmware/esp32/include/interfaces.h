#pragma once

#include <Arduino.h>

/*
 * Interfaces (kontrak): daftar "aturan" yang harus dipatuhi oleh
 * setiap implementasi konkret. Program utama hanya mengenal kontrak ini,
 * bukan cara kerja detail di belakangnya.
 *
 * Cara pakai: kalau ingin mengganti library (misal ganti WiFiManager
 * dengan library lain), cukup buat class baru yang mengikuti kontrak
 * di bawah. App tidak perlu diubah.
 */

// Kontrak untuk koneksi WiFi.
class IWiFiConnector
{
public:
    virtual ~IWiFiConnector() = default;

    // Menghubungkan ke WiFi (mode awal / buat AP kalau perlu).
    virtual bool connect() = 0;

    // Mengecek apakah WiFi masih terhubung.
    virtual bool isConnected() = 0;

    // Mencoba menyambung kembali ke WiFi yang pernah tersimpan (tanpa AP).
    virtual bool reconnect() = 0;
};

// Kontrak untuk koneksi MQTT.
class IMqttConnector
{
public:
    virtual ~IMqttConnector() = default;

    // Menyiapkan koneksi (memanggil setServer). Dipanggil sekali di setup.
    virtual void begin() = 0;

    // Menjamin MQTT terhubung. Jika putus, coba hubungkan lagi.
    virtual bool ensureConnected() = 0;

    // Mengirim pesan teks ke topik yang sudah diatur.
    virtual bool publish(const char *message) = 0;

    // Harus dipanggil berkala di loop() agar MQTT tetap hidup.
    virtual void loop() = 0;
};
