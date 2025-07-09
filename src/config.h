#ifndef CONFIG_H
#define CONFIG_H

// --- Configuración de Pines ---
#define PIN_SEGURIDAD_MOISTURE 35     // Pin para el relay de luz
#define PIN_RELAY_AGUA 18    // Pin para el relay de agua  
#define PIN_MOISTURE 32     // Pin analógico para sensor de humedad
#define PIN_FOTOR 33        // Pin analógico para sensor de luz

// --- Metadata del dispositivo ---
#define DEVICE_NAME "AuPlant_ESP32"  // Nombre del dispositivo
#define DEVICE_VERSION "1.0.0"        // Versión del firmware
#define DEVICE_ID "bigbang_001"

// --- Configuración WiFi ---
// Nota: Las credenciales WiFi se obtienen del provisioning
// pero puedes definir valores por defecto aquí si es necesario
#define WIFI_SSID "TU_SSID"
#define WIFI_PASSWORD "TU_PASSWORD"

// --- Configuración MQTT ---
#define MQTT_SERVER "iot.ceisufro.cl"  // Servidor MQTT público para pruebas
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "AuPlant_ESP32"
#define MQTT_USER "6dEMoPg5GMOW2BJ3n0j4"  // Vacío para brokers públicos
#define MQTT_PASSWORD ""  // Vacío para brokers públicos

// --- Tópicos MQTT ---
#define TOPIC_TELEMETRY_PUBLISH "v1/devices/me/telemetry"
#define TOPIC_CONTROL_SUBSCRIBE "v1/devices/me/control"

// --- Intervalos de tiempo (en milisegundos) ---
#define INTERVALO_MENSAJE_MQTT 30000    // 30 segundos
#define INTERVALO_SERIAL_PRINT 10000    // 10 segundos

// --- Configuración de riego automático ---
#define HUMEDAD_MINIMA 20    // Porcentaje mínimo de humedad para activar riego
#define HUMEDAD_MAXIMA 70    // Porcentaje máximo de humedad para desactivar riego

// --- Configuración de seguridad ---
#define SENSOR_SEGURIDAD_THRESHOLD 3000    // Umbral para detectar sensor desconectado
#define INTENTOS_MAXIMOS_SEGURIDAD 3       // Intentos antes de desactivar seguridad

#endif
