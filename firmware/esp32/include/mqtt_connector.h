#pragma once

#include "interfaces.h"
#include "config.h"
#include "config_private.h"

#include <WiFi.h>
#include <PubSubClient.h>

/*
 * Implementasi IMqttConnector menggunakan library PubSubClient.
 * Cara pakai: buat objek dengan settings MQTT + credential, panggil begin()
 * sekali di setup(), lalu pakai ensureConnected()/publish()/loop() di loop().
 */
class PubSubMqttConnector : public IMqttConnector
{
public:
    // Constructor: terima pengaturan MQTT (server, topik) + credential rahasia.
    PubSubMqttConnector(const MqttSettings &settings, const MqttSecrets &secrets);

    // Siapkan koneksi ke broker (dipanggil sekali di setup).
    void begin() override;

    // Pastikan terhubung; hubungkan kembali bila terputus dengan jeda tertentu.
    bool ensureConnected() override;

    // Kirim pesan teks ke topik yang sudah diatur.
    bool publish(const char *message) override;

    // Harus dipanggil terus-menerus di loop() agar tetap hidup.
    void loop() override;

private:
    // Percobaan menghubungkan sekali saja; true bila berhasil.
    bool connectOnce();

    const MqttSettings &_settings;
    const MqttSecrets &_secrets;
    WiFiClient _wifiClient;
    PubSubClient _client;
    String _clientId;
    unsigned long _lastAttempt = 0;  // Waktu percobaan ulang terakhir
};
