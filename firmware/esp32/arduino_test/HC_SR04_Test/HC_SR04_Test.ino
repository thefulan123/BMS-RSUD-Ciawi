/*
 * HC-SR04 Health Check + Distance Reader
 * ----------------------------------------
 * 1. Cek kesehatan sensor (20x pembacaan)
 * 2. Kalau sensor OK, lanjut baca jarak terus-menerus
 * 3. Kalau sensor mati, tampilkan diagnosa
 *
 * Wiring (ESP32):
 *   VCC -> 5V
 *   GND -> GND
 *   TRIG -> GPIO 33
 *   ECHO -> GPIO 14 (pakai voltage divider 1k/2k ke 3.3V!)
 *
 * Board: ESP32 Dev Module | Baud: 115200
 */

#define TRIG_PIN 33
#define ECHO_PIN 14
#define HEALTH_SAMPLES 20
#define HEALTH_DELAY_MS 100

float readCM()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);

    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duration == 0)
    {
        return -1;
    }

    return duration * 0.0343 / 2.0;
}

bool runHealthCheck()
{
    Serial.println("==================================");
    Serial.println("  HC-SR04 HEALTH CHECK");
    Serial.println("==================================");

    int success = 0;
    int timeout = 0;
    float sum = 0;
    float minVal = 9999;
    float maxVal = -9999;

    for (int i = 0; i < HEALTH_SAMPLES; i++)
    {
        float d = readCM();

        if (d < 0)
        {
            timeout++;
            Serial.print("["); Serial.print(i + 1); Serial.println("] TIMEOUT");
        }
        else
        {
            success++;
            sum += d;
            if (d < minVal) minVal = d;
            if (d > maxVal) maxVal = d;
            Serial.print("["); Serial.print(i + 1); Serial.print("] ");
            Serial.print(d, 2); Serial.println(" cm");
        }

        delay(HEALTH_DELAY_MS);
    }

    Serial.println("----------------------------------");
    Serial.println("  HASIL KESEHATAN");
    Serial.println("----------------------------------");
    Serial.print("Total sampel : "); Serial.println(HEALTH_SAMPLES);
    Serial.print("Sukses       : "); Serial.println(success);
    Serial.print("Timeout      : "); Serial.println(timeout);

    if (success > 0)
    {
        float healthPct = (success * 100.0) / HEALTH_SAMPLES;
        Serial.print("Health %     : ");
        Serial.print(healthPct, 1);
        Serial.println(" %");

        Serial.print("Rata-rata    : ");
        Serial.print(sum / success, 2); Serial.println(" cm");

        Serial.print("Terdekat     : ");
        Serial.print(minVal, 2); Serial.println(" cm");

        Serial.print("Terjauh      : ");
        Serial.print(maxVal, 2); Serial.println(" cm");

        Serial.println("----------------------------------");
        Serial.println("Sensor OK! Memulai pembacaan jarak...");
        Serial.println("==================================");
        return true;
    }
    else
    {
        Serial.println("Health %     : 0 % -> SENSOR TIDAK AKTIF");
        Serial.println();
        Serial.println("PENYEBAB KEMUNGKINAN:");
        Serial.println("  1. VCC bukan 5V (HC-SR04 wajib 5V)");
        Serial.println("  2. GND tidak nyambung / tidak common");
        Serial.println("  3. TRIG/ECHO pin salah");
        Serial.println("  4. ECHO tidak pakai voltage divider");
        Serial.println("     (ECHO output 5V, ESP32 max 3.3V)");
        Serial.println("  5. Sensor rusak");
        Serial.println("==================================");
        return false;
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);
    delay(500);

    bool sensorOK = runHealthCheck();

    if (!sensorOK)
    {
        Serial.println("Sensor tidak terdeteksi. Cek wiring lalu reset board.");
        while (true) { delay(1000); }
    }
}

void loop()
{
    float distance = readCM();

    if (distance < 0)
    {
        Serial.println("TIMEOUT - tidak ada pantulan");
    }
    else
    {
        Serial.print("Jarak: ");
        Serial.print(distance, 2);
        Serial.println(" cm");
    }

    delay(500);
}