#include "mqtt_connector.h"

#include <Arduino.h>

// Constructor: simpan settings + credential, buat clientId unik dari MAC address.
PubSubMqttConnector::PubSubMqttConnector(const MqttSettings &settings,
                                         const MqttSecrets &secrets)
    : _settings(settings),
      _secrets(secrets),
      _client(_wifiClient)
{
    _clientId = "ESP32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
}

// Cara pakai: panggil sekali di setup() setelah WiFi tersambung.
void PubSubMqttConnector::begin()
{
    _client.setServer(_settings.server, _settings.port);
}

// Internal: coba connect ke broker sekali. Kirim pesan "connected" bila sukses.
bool PubSubMqttConnector::connectOnce()
{
    Serial.print("MQTT: mencoba connect... ");

    if (_client.connect(_clientId.c_str(), _secrets.user, _secrets.password))
    {
        Serial.println("TERHUBUNG");
        _client.publish(_settings.topic, "ESP32 connected via WiFiManager");
        return true;
    }

    Serial.print("GAGAL, state=");
    Serial.println(_client.state());
    return false;
}

// Cara pakai: panggil di loop(). Akan otomatis reconnect bila putus,
// tapi hanya mencoba tiap `reconnectIntervalMs` agar tidak membebani ESP32.
bool PubSubMqttConnector::ensureConnected()
{
    if (_client.connected())
    {
        return true;
    }

    unsigned long now = millis();

    if (now - _lastAttempt < _settings.reconnectIntervalMs)
    {
        return false;  // Masih dalam jeda tunggu, jangan connect berulang-ulang.
    }

    _lastAttempt = now;
    return connectOnce();
}

// Cara pakai: kirim pesan String/char* ke topik MQTT.
bool PubSubMqttConnector::publish(const char *message)
{
    if (!_client.connected())
    {
        Serial.println("MQTT: publish dibatalkan, belum terhubung");
        return false;
    }

    return _client.publish(_settings.topic, message);
}

// Cara pakai: panggil di loop() setiap kali supaya MQTT memproses data.
void PubSubMqttConnector::loop()
{
    _client.loop();
}
