#include "WiFiProv.h"
#include "WiFi.h"
#include "SysProvEvent.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "sensors.h"
#include "mqtt_client.h"
#include "irrigation.h"
#include "safety.h"

// --- Variables globales del sistema de sensores ---
int valorHumedad;
int valorResistencia;
int humedad;
int resistencia_luz;

int humedadMin = 100, humedadMax = 0;
int luzMin = 100, luzMax = 0;
unsigned long tiempoInicio;
bool relayEstado = false; // Estado del relay (controlado por MQTT)

// --- Variables para riego temporizado ---
bool riegoTemporizado = false;
unsigned long tiempoInicioRiego = 0;

// --- Variables de seguridad ---
bool sensorSeguridadOk = false;
int intentosSeguridadFallidos = 0;
unsigned long ultimaVerificacionSeguridad = 0;

// --- Objetos WiFi y MQTT ---
WiFiClient espClient;
PubSubClient client(espClient);

// --- Intervalos ---
unsigned long ultimoMensajeMqtt = 0;
unsigned long ultimoSerialPrint = 0;
unsigned long ultimaVerificacionAtributos = 0;

// Estado actual del sistema
SystemState currentState = PROVISIONING;

void setup() {
  Serial.begin(115200);
  Serial.println("\n🌱 Sistema AuPlant - Monitoreo y Control con WiFi Provisioning");
  
  configurarPines();
  
  
  // Configurar WiFi provisioning
  WiFi.onEvent(SysProvEvent);
  
  Serial.println("📡 Iniciando WiFi Provisioning...");
  Serial.println("Creando Access Point para provisioning...");
  
  WiFiProv.beginProvision(
    WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, 
    WIFI_PROV_SECURITY_1, WIFI_PROV_POP, WIFI_PROV_SERVICE_NAME, WIFI_PROV_SERVICE_KEY, NULL, false
  );
  
  Serial.println("\n=== Detalles de WiFi Provisioning ===");
  Serial.print("AP Name: ");
  Serial.println(WIFI_PROV_SERVICE_NAME);
  Serial.print("AP Password: ");
  Serial.println(WIFI_PROV_SERVICE_KEY);
  Serial.print("Provisioning PIN: ");
  Serial.println(WIFI_PROV_POP);
  Serial.println("Conecta al AP y usa la app de provisioning de Espressif");
  Serial.println("=====================================\n");
  
  // Configurar MQTT
  setupMQTT();
  
  tiempoInicio = millis();
  
  // Lecturas iniciales
  delay(100);
  leerSensores();
  humedadMin = humedad;
  humedadMax = humedad;
  luzMin = resistencia_luz;
  luzMax = resistencia_luz;
  
  Serial.println("✅ Sistema iniciado correctamente.");
}

void loop() {
  unsigned long now = millis();
  
  switch (currentState) {
    case PROVISIONING:
      if (provisioningComplete && WiFi.isConnected()) {
        Serial.println("\n🎉 Provisioning completado. Iniciando sistema MQTT...");
        currentState = CONNECTING_MQTT;
      } else {
        // Mostrar estado de provisioning
        static unsigned long lastProvisioningCheck = 0;
        if (now - lastProvisioningCheck > 5000) {
          Serial.println("⏳ Esperando provisioning WiFi...");
          lastProvisioningCheck = now;
        }
      }
      break;
      
    case CONNECTING_MQTT:
      if (WiFi.isConnected()) {
        if (!client.connected()) {
          reconnectMqtt();
        } else {
          Serial.println("✅ Conectado a MQTT. Sistema completamente operativo.");
          Serial.println("🔌 Estado del relay: " + String(relayEstado ? "ACTIVADO" : "DESACTIVADO"));
          currentState = RUNNING;
        }
      } else {
        Serial.println("❌ WiFi desconectado. Volviendo a provisioning...");
        currentState = PROVISIONING;
      }
      break;
      
    case RUNNING:
      if (!WiFi.isConnected()) {
        Serial.println("❌ WiFi desconectado. Volviendo a provisioning...");
        currentState = PROVISIONING;
        break;
      }
      
      if (!client.connected()) {
        reconnectMqtt();
      }
      
      
      // Leer sensores continuamente
      leerSensores();
      
      // Verificar seguridad del sensor periódicamente
      if (now - ultimaVerificacionSeguridad > INTERVALO_VERIFICACION_SEGURIDAD) { // Cada 5 segundos
        ultimaVerificacionSeguridad = now;
        verificarSeguridadSensor();
      }
      
      // Publicar datos por MQTT periódicamente
      if (now - ultimoMensajeMqtt > INTERVALO_MENSAJE_MQTT) {
        ultimoMensajeMqtt = now;
        publicarDatosSensores();
      }
      
      // Verificar atributos periódicamente para asegurar sincronización
      if (now - ultimaVerificacionAtributos > INTERVALO_VERIFICACION_ATRIBUTOS) {
        ultimaVerificacionAtributos = now;
        Serial.println("🔄 Verificación periódica de atributos...");
        solicitarAtributos();
      }
      
      // Verificar riego temporizado
      verificarRiegoTemporizado();
      
      // Imprimir estado en Serial
      if (now - ultimoSerialPrint > INTERVALO_SERIAL_PRINT) {
        ultimoSerialPrint = now;
        imprimirEstadoSistema();
      }
      
      client.loop();
      break;
  }
  
  delay(100);
}
