#pragma once

#include <Arduino.h>

/*
 * CONTOH config_private.h — SALIN file ini menjadi "config_private.h"
 * lalu isi credential MQTT kamu. File config_private.h SUDAH di-.gitignore
 * sehingga tidak ikut ter-commit/ter-upload.
 */
struct MqttSecrets
{
    const char *user = "isi_user_mqtt";
    const char *password = "isi_password_mqtt";
};

extern const MqttSecrets SECRETS;
