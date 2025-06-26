#ifndef CONFIG_H
#define CONFIG_H

// --- Configuración WiFi ---
#define WIFI_SSID             // Nombre de la red Wi-Fi
#define WIFI_PASSWORD    // Contraseña de la red Wi-Fi

// --- Configuración MQTT ---
#define MQTT_SERVER "iot.ceisufro.cl"    // Broker MQTT
#define MQTT_PORT 1883                    // Puerto del broker MQTT
#define MQTT_CLIENT_ID "78137b20-474a-11f0-a76f-af9873efe2ab" // ID de cliente MQTT
#define MQTT_USER "authKey"  // Usuario de autenticación
#define MQTT_PASSWORD ""                  // Contraseña de MQTT

// --- Tópicos MQTT ---
#define TOPIC_HUMEDAD_PUBLISH "sensor/humedad"
#define TOPIC_LUZ_PUBLISH "sensor/luz"
#define TOPIC_TELEMETRY_PUBLISH "v1/devices/me/telemetry"

// --- Pines de los componentes ---
#define PIN_RELAY_LUZ 19
#define PIN_RELAY_AGUA 18
#define PIN_MOISTURE 32
#define PIN_FOTOR 33

// --- Intervalos de publicación ---
#define INTERVALO_MENSAJE_MQTT 5000 // Publicar cada 5 segundos
#define INTERVALO_SERIAL_PRINT 10000 // Imprimir en serial cada 10 segundos

#endif
