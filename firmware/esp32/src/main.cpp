#include <Arduino.h>

#include "config.h"
#include "config_private.h"
#include "app.h"
#include "wifi_connector.h"
#include "mqtt_connector.h"
#include "hcsr04_sensor.h"

// Semua pengaturan perangkat didefinisikan di sini (lihat config.h).
const DeviceSettings DEVICE;

// Credential rahasia (user/password) — TIDAK di-commit ke Git.
const MqttSecrets SECRETS;

// Buat implementasi konkret (bisa diganti tanpa menyentuh App).
WiFiManagerConnector wifi(DEVICE.wifi);
PubSubMqttConnector mqtt(DEVICE.mqtt, SECRETS);
HcSr04Sensor sensor(DEVICE.sensor.trigPin, DEVICE.sensor.echoPin);

// Rangkai semua bagian: App menerima WiFi, MQTT, sensor, dan settings.
App app(wifi, mqtt, sensor, DEVICE);

// Fungsi wajib Arduino: dipanggil sekali saat board nyala.
void setup()
{
    app.setup();
}

// Fungsi wajib Arduino: dipanggil berulang-ulang.
void loop()
{
    app.loop();
}
