#pragma once

#include "interfaces.h"
#include "sensor.h"
#include "config.h"

#include <Arduino.h>

/*
 * App: "otak" pengatur jalannya program. Ia tidak tahu detail WiFi/MQTT/sensor,
 * hanya mengenal kontrak (interface). Semua bagian disuntikkan (di-inject)
 * lewat constructor -> ini prinsip Dependency Inversion.
 *
 * Cara pakai:
 *   App app(wifi, mqtt, sensor, DEVICE);
 *   app.setup();   // di fungsi setup()
 *   app.loop();    // di fungsi loop()
 */
class App
{
public:
    // Terima connector WiFi, MQTT, sensor, dan pengaturan perangkat.
    App(IWiFiConnector &wifi,
        IMqttConnector &mqtt,
        IDistanceSensor &sensor,
        const DeviceSettings &settings);

    // Jalankan sekali di awal: nyalakan Serial, siapkan sensor, WiFi & MQTT.
    void setup();

    // Jalankan berulang: jaga koneksi, baca sensor & kirim pesan tiap interval.
    void loop();

private:
    // Buat string JSON dari hasil bacaan lengkap, misal:
    // {"distance_cm":6.68,"volume_ml":709.0,"level_percent":70.9,"differential":-95.2}
    String buildJson(const SensorReading &reading);

    IWiFiConnector &_wifi;
    IMqttConnector &_mqtt;
    IDistanceSensor &_sensor;
    const DeviceSettings &_settings;
    unsigned long _lastPublish = 0;
};
