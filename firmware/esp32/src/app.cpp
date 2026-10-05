#include "app.h"

// Constructor: simpan referensi ke WiFi, MQTT, sensor, dan settings yang disuntikkan.
App::App(IWiFiConnector &wifi,
         IMqttConnector &mqtt,
         IDistanceSensor &sensor,
         const DeviceSettings &settings)
    : _wifi(wifi),
      _mqtt(mqtt),
      _sensor(sensor),
      _settings(settings)
{
}

// Cara pakai: panggil di setup(). Menyiapkan semua koneksi.
void App::setup()
{
    Serial.begin(_settings.serialBaud);
    delay(1000);

    Serial.println();
    Serial.println("==============================");
    Serial.println("ESP32 BMS MQTT");
    Serial.println("==============================");

    // Siapkan hardware sensor (pin TRIG/ECHO).
    _sensor.begin();

    if (!_wifi.connect())
    {
        Serial.println("WiFi gagal, restart...");
        ESP.restart();
    }

    _mqtt.begin();

    // Coba connect MQTT sekali saja, TIDAK menunggu (non-blocking).
    // Kalau broker mati, ESP32 tetap jalan dan mencoba lagi di loop().
    if (!_mqtt.ensureConnected())
    {
        Serial.println("MQTT belum terhubung, akan dicoba berkala di loop()");
    }
}

// Cara pakai: panggil di loop(). Menjaga koneksi & mengirim pesan berkala.
void App::loop()
{
    // Jaga WiFi: kalau putus, coba sambungkan lagi.
    if (!_wifi.isConnected())
    {
        Serial.println("WiFi putus, mencoba reconnect...");
        _wifi.reconnect();
    }

    // Jaga MQTT: otomatis reconnect dengan jeda (lihat mqtt_connector).
    _mqtt.ensureConnected();
    _mqtt.loop();

    // Kirim data hanya tiap publishIntervalMs agar tidak membebani jaringan.
    if (millis() - _lastPublish >= _settings.publishIntervalMs)
    {
        _lastPublish = millis();

        float distanceCM = _sensor.readCM();

        if (distanceCM < 0)
        {
            Serial.println("HC-SR04 timeout!");
            return;
        }

        String payload = buildJson(distanceCM);

        if (_mqtt.publish(payload.c_str()))
        {
            Serial.print("Distance: ");
            Serial.print(distanceCM, 2);
            Serial.println(" cm");

            Serial.print("JSON: ");
            Serial.println(payload);
        }
        else
        {
            Serial.println("MQTT -> GAGAL kirim (belum terhubung)");
        }
    }
}

// Buat payload JSON sederhana dari jarak (cara awam: susun string manual).
String App::buildJson(float distanceCM)
{
    String payload = "{";
    payload += "\"distance_cm\":";
    payload += String(distanceCM, 2);
    payload += "}";
    return payload;
}
