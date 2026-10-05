#include "wifi_connector.h"

// Constructor: simpan referensi settings agar bisa dipakai di fungsi lain.
WiFiManagerConnector::WiFiManagerConnector(const WifiSettings &settings)
    : _settings(settings)
{
}

// Cara pakai: panggil sekali di setup().
// Fungsi ini akan mencoba connect ke WiFi tersimpan, atau membuat
// Access Point "ESP32-BMS" kalau belum ada settingan.
// Kalau tombol config ditahan saat nyala, setting lama dihapus &
// portal konfigurasi dibuka kembali (untuk ganti WiFi).
bool WiFiManagerConnector::connect()
{
    Serial.println();
    Serial.println("==============================");
    Serial.println("WiFi Setup");
    Serial.println("==============================");

    WiFiManager wm;

    // Cek tombol reset: tahan saat power-on untuk masuk mode ganti WiFi.
    if (_settings.configButtonPin >= 0)
    {
        pinMode(_settings.configButtonPin, INPUT_PULLUP);

        if (digitalRead(_settings.configButtonPin) == LOW)
        {
            Serial.println("Tahan tombol untuk masuk mode config WiFi...");

            unsigned long start = millis();
            while (digitalRead(_settings.configButtonPin) == LOW &&
                   millis() - start < (unsigned long)_settings.configHoldMs)
            {
                delay(50);
            }

            if (millis() - start >= (unsigned long)_settings.configHoldMs)
            {
                Serial.println("Reset setting WiFi lama -> buka portal");
                wm.resetSettings();  // Hapus credential tersimpan
            }
            else
            {
                Serial.println("Tombol terlalu singkat, lanjut normal");
            }
        }
    }

    if (!wm.autoConnect(_settings.accessPointName))
    {
        Serial.println("WiFi connection failed.");
        return false;
    }

    Serial.println();
    Serial.println("WiFi CONNECTED");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    return true;
}

// Cara pakai: panggil kapan saja untuk mengecek apakah WiFi masih nyambung.
bool WiFiManagerConnector::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

// Cara pakai: panggil di loop() bila isConnected() bernilai false,
// supaya ESP32 otomatis mencoba menyambung ke WiFi tersimpan lagi.
bool WiFiManagerConnector::reconnect()
{
    return WiFi.reconnect();
}
