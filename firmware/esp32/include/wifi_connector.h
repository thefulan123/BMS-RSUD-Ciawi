#pragma once

#include "interfaces.h"
#include "config.h"

#include <WiFi.h>
#include <WiFiManager.h>

/*
 * Implementasi IWiFiConnector menggunakan library WiFiManager.
 * Cara pakai: buat objek dengan settings WiFi, lalu panggil connect().
 */
class WiFiManagerConnector : public IWiFiConnector
{
public:
    // Constructor: terima pengaturan WiFi (nama AP, dll).
    explicit WiFiManagerConnector(const WifiSettings &settings);

    // Hubungkan ke WiFi (atau buat AP konfigurasi jika gagal).
    bool connect() override;

    // Cek status koneksi WiFi saat ini.
    bool isConnected() override;

    // Coba hubungkan lagi ke WiFi tersimpan (dipanggil kalau putus di runtime).
    bool reconnect() override;

private:
    const WifiSettings &_settings;
};
