#ifndef CONFIG_H
#define CONFIG_H

// --- Configuración de logs ---
// Define el nivel de log actual: 
// 0 = Solo errores críticos
// 1 = Errores y advertencias importantes
// 2 = Información general (recomendado para uso normal)
// 3 = Información detallada (debug)
// 4 = Información muy detallada (debug verbose)
#define LOG_LEVEL 2

// Macros para logs de diferentes niveles
#define LOG_CRITICAL(msg) if(LOG_LEVEL >= 0) { Serial.println("❌ " msg); }
#define LOG_ERROR(msg) if(LOG_LEVEL >= 0) { Serial.println("❌ " msg); }
#define LOG_WARNING(msg) if(LOG_LEVEL >= 1) { Serial.println("⚠️ " msg); }
#define LOG_INFO(msg) if(LOG_LEVEL >= 2) { Serial.println("ℹ️ " msg); }
#define LOG_DEBUG(msg) if(LOG_LEVEL >= 3) { Serial.println("🔍 " msg); }
#define LOG_VERBOSE(msg) if(LOG_LEVEL >= 4) { Serial.println("🔎 " msg); }

// Macros para logs con formato
#define LOG_CRITICAL_F(format, ...) if(LOG_LEVEL >= 0) { Serial.printf("❌ " format "\n", __VA_ARGS__); }
#define LOG_ERROR_F(format, ...) if(LOG_LEVEL >= 0) { Serial.printf("❌ " format "\n", __VA_ARGS__); }
#define LOG_WARNING_F(format, ...) if(LOG_LEVEL >= 1) { Serial.printf("⚠️ " format "\n", __VA_ARGS__); }
#define LOG_INFO_F(format, ...) if(LOG_LEVEL >= 2) { Serial.printf("ℹ️ " format "\n", __VA_ARGS__); }
#define LOG_DEBUG_F(format, ...) if(LOG_LEVEL >= 3) { Serial.printf("🔍 " format "\n", __VA_ARGS__); }
#define LOG_VERBOSE_F(format, ...) if(LOG_LEVEL >= 4) { Serial.printf("🔎 " format "\n", __VA_ARGS__); }

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
#define MQTT_USER "ev1dmxege252nbbielxu"  // Vacío para brokers públicos
#define MQTT_PASSWORD ""  // Vacío para brokers públicos

// --- Tópicos MQTT ---
#define TOPIC_TELEMETRY_PUBLISH "v1/devices/me/telemetry"
#define TOPIC_ATTRIBUTES_REQUEST "v1/devices/me/attributes/request/1"
#define TOPIC_ATTRIBUTES_RESPONSE "v1/devices/me/attributes/response/+"
#define TOPIC_ATTRIBUTES "v1/devices/me/attributes"

// --- Intervalos de tiempo (en milisegundos) ---
#define INTERVALO_MENSAJE_MQTT 10000    // 30 segundos
#define INTERVALO_SERIAL_PRINT 10000    // 10 segundos
#define INTERVALO_VERIFICACION_ATRIBUTOS 30000 // 30 segundos
#define INTERVALO_VERIFICACION_SEGURIDAD 5000  // 5 segundos

// --- Configuración de riego automático ---
#define HUMEDAD_MINIMA 20    // Porcentaje mínimo de humedad para activar riego
#define HUMEDAD_MAXIMA 70    // Porcentaje máximo de humedad para desactivar riego
#define DURACION_RIEGO 5000  // 5 segundos en milisegundos

// --- Configuración de seguridad ---
#define SENSOR_SEGURIDAD_THRESHOLD 3000    // Umbral para detectar sensor desconectado
#define INTENTOS_MAXIMOS_SEGURIDAD 3       // Intentos antes de desactivar seguridad

// --- Configuración del Provisioning WiFi ---
#define WIFI_PROV_POP "AUPLANT_IOT"           // Proof of possession - PIN
#define WIFI_PROV_SERVICE_NAME "PLANT_AUID001" // Nombre del dispositivo
#define WIFI_PROV_SERVICE_KEY "auPlantPassKeyy" // Password para SoftAP

// --- Definición de estados del sistema ---
enum SystemState {
  PROVISIONING,
  CONNECTING_MQTT,
  RUNNING
};

// --- Variables externas que se declaran en main.cpp ---
extern int valorHumedad;
extern int valorResistencia;
extern int humedad;
extern int resistencia_luz;
extern int humedadMin, humedadMax;
extern int luzMin, luzMax;
extern unsigned long tiempoInicio;
extern bool relayEstado;
extern bool riegoTemporizado;
extern unsigned long tiempoInicioRiego;
extern bool sensorSeguridadOk;
extern int intentosSeguridadFallidos;
extern unsigned long ultimaVerificacionSeguridad;
extern unsigned long ultimoMensajeMqtt;
extern unsigned long ultimoSerialPrint;
extern unsigned long ultimaVerificacionAtributos;
extern SystemState currentState;

#endif
