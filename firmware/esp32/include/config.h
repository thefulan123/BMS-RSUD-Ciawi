#pragma once

#include <Arduino.h>
#include "config_private.h"

/*
 * Config: tempat menyimpan semua pengaturan perangkat (yang tidak rahasia).
 * Credential MQTT (user/password) dipisah di config_private.h agar aman.
 *
 * Cara pakai: ubah nilai di dalam struct di bawah sesuai kebutuhan
 * (misal ganti alamat server MQTT, nama WiFi AP, dll).
 * Struct dibuat bertingkat agar mudah dibaca:
 *   - WifiSettings  : semua yang berhubungan dengan WiFi
 *   - MqttSettings  : semua yang berhubungan dengan MQTT (kecuali credential)
 *   - DeviceSettings: membungkung keduanya + pengaturan umum
 */
struct WifiSettings
{
    const char *accessPointName = "ESP32-BMS";  // Nama AP saat mode konfigurasi WiFi
    int configButtonPin = 4;   // Pin tombol reset WiFi (tahan saat nyala -> buka portal)
    int configHoldMs = 3000;   // Berapa lama tombol ditahan untuk masuk mode config
};

struct MqttSettings
{
    const char *server = "broker.avisha.id";      // Alamat broker MQTT
    int port = 1883;                           // Port broker MQTT
    const char *topic = "bms/gwt1";          // Topik tempat mengirim pesan
    unsigned long reconnectIntervalMs = 5000;  // Jeda antar percobaan ulang MQTT (ms)
};

struct SensorSettings
{
    int trigPin = 33;    // Pin TRIG HC-SR04
    int echoPin = 14;   // Pin ECHO HC-SR04
};

struct DeviceSettings
{
    unsigned long serialBaud = 115200;       // Kecepatan Serial Monitor
    unsigned long publishIntervalMs = 5000;  // Interval kirim pesan (ms)
    WifiSettings wifi;                       // Pengaturan WiFi
    MqttSettings mqtt;                       // Pengaturan MQTT
    SensorSettings sensor;                   // Pengaturan sensor jarak
};

// Objek DEVICE berisi semua setting yang dipakai program.
extern const DeviceSettings DEVICE;
