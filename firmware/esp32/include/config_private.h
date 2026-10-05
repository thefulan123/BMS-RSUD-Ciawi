#pragma once

#include <Arduino.h>

/*
 * config_private.h: berisi data rahasia (credential MQTT).
 * PENTING: file ini sengaja TIDAK di-commit ke Git (sudah masuk .gitignore).
 * Cara pakai: salin contoh ini ke file ini, lalu isi user & password sendiri.
 * Jangan upload file ini ke orang lain / ChatGPT.
 */
struct MqttSecrets
{
    const char *user = "bms";
    const char *password = "soke1234";
};

extern const MqttSecrets SECRETS;
