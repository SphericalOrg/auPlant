#include "WiFiProv.h"
#include "WiFi.h"
#include "SysProvEvent.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"

// --- Configuración del Provisioning WiFi ---
const char * pop = "AUPLANT_IOT"; // Proof of possession - PIN
const char * service_name = "PLANT_AUID001"; // Nombre del dispositivo
const char * service_key = "auPlantPassKeyy"; // Password para SoftAP
bool reset_provisioned = true; // Resetear datos de provisioning previos

// --- Variables globales del sistema de sensores ---
int valorHumedad;
int valorResistencia;
int humedad;
int resistencia_luz;

int humedadMin = 100, humedadMax = 0;
int luzMin = 100, luzMax = 0;
unsigned long tiempoInicio;
bool modoAutomaticoGlobal = false;

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

// --- Estados del sistema ---
enum SystemState {
  PROVISIONING,
  CONNECTING_MQTT,
  RUNNING
};
SystemState currentState = PROVISIONING;

// --- Declaración de funciones ---
void callbackMqtt(char* topic, byte* payload, unsigned int length);
void reconnectMqtt();
void publicarDatosSensores();
void verificarRiegoAutomatico();
void leerSensores();
void imprimirEstadoSistema();
void configurarPines();
bool verificarSeguridadSensor();
void activarRiegoSeguro();
void desactivarRiegoSeguro();

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
    WIFI_PROV_SECURITY_1, pop, service_name, service_key, NULL, reset_provisioned
  );
  
  Serial.println("\n=== Detalles de WiFi Provisioning ===");
  Serial.print("AP Name: ");
  Serial.println(service_name);
  Serial.print("AP Password: ");
  Serial.println(service_key);
  Serial.print("Provisioning PIN: ");
  Serial.println(pop);
  Serial.println("Conecta al AP y usa la app de provisioning de Espressif");
  Serial.println("=====================================\n");
  
  // Configurar MQTT
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callbackMqtt);
  
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
          Serial.print("🤖 Modo de riego automático: ");
          Serial.println(modoAutomaticoGlobal ? "ACTIVADO" : "DESACTIVADO");
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
      client.loop();
      
      // Leer sensores continuamente
      leerSensores();
      
      // Verificar seguridad del sensor periódicamente
      if (now - ultimaVerificacionSeguridad > 5000) { // Cada 5 segundos
        ultimaVerificacionSeguridad = now;
        verificarSeguridadSensor();
      }
      
      // Publicar datos por MQTT periódicamente
      if (now - ultimoMensajeMqtt > INTERVALO_MENSAJE_MQTT) {
        ultimoMensajeMqtt = now;
        publicarDatosSensores();
      }
      
      // Verificar riego automático
      verificarRiegoAutomatico();
      
      // Imprimir estado en Serial
      if (now - ultimoSerialPrint > INTERVALO_SERIAL_PRINT) {
        ultimoSerialPrint = now;
        imprimirEstadoSistema();
      }
      break;
  }
  
  delay(100);
}

void configurarPines() {
  pinMode(PIN_SEGURIDAD_MOISTURE, INPUT);
  pinMode(PIN_RELAY_AGUA, OUTPUT);
  pinMode(PIN_MOISTURE, INPUT);
  pinMode(PIN_FOTOR, INPUT);
  
  digitalWrite(PIN_RELAY_AGUA, LOW);
  
  // Configuración de la atenuación para los sensores analógicos
  analogSetAttenuation(ADC_11db);
}

void leerSensores() {
  valorHumedad = analogRead(PIN_MOISTURE);
  valorResistencia = analogRead(PIN_FOTOR);
  
  humedad = int(((float(valorHumedad) / 4095.0) - 1.0) * -100.0);
  resistencia_luz = int((float(valorResistencia) / 4095.0) * 100.0);
  
  // Limitar valores a 0-100%
  if (humedad < 0) humedad = 0;
  if (humedad > 100) humedad = 100;
  if (resistencia_luz < 0) resistencia_luz = 0;
  if (resistencia_luz > 100) resistencia_luz = 100;
  
  // Actualizar estadísticas
  if (humedad < humedadMin) humedadMin = humedad;
  if (humedad > humedadMax) humedadMax = humedad;
  if (resistencia_luz < luzMin) luzMin = resistencia_luz;
  if (resistencia_luz > luzMax) luzMax = resistencia_luz;
}

void callbackMqtt(char* topic, byte* payload, unsigned int length) {
  String mensajePayload = "";
  for (unsigned int i = 0; i < length; i++) {
    mensajePayload += (char)payload[i];
  }
  
  Serial.print("📬 Mensaje MQTT recibido [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(mensajePayload);
  
  if (String(topic) == TOPIC_CONTROL_SUBSCRIBE) {
    
    if (mensajePayload == "agua_on") {
      modoAutomaticoGlobal = false;
      activarRiegoSeguro();  // Usar función de seguridad
      Serial.println("(Modo Auto DESACTIVADO)");
    } else if (mensajePayload == "agua_off") {
      modoAutomaticoGlobal = false;
      desactivarRiegoSeguro();  // Usar función de seguridad
      Serial.println("(Modo Auto DESACTIVADO)");
    } else if (mensajePayload == "auto_on") {
      modoAutomaticoGlobal = true;
      Serial.println("🤖 Modo riego automático ACTIVADO por MQTT");
    } else if (mensajePayload == "auto_off") {
      modoAutomaticoGlobal = false;
      Serial.println("🤖 Modo riego automático DESACTIVADO por MQTT");
    }
  }
}

void reconnectMqtt() {
  while (!client.connected() && WiFi.isConnected()) {
    Serial.print("🔌 Intentando conexión MQTT...");
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("¡conectado!");
      client.subscribe(TOPIC_CONTROL_SUBSCRIBE);
      Serial.print("📡 Suscrito a: ");
      Serial.println(TOPIC_CONTROL_SUBSCRIBE);
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" | Intentando de nuevo en 5 segundos...");
      delay(5000);
    }
  }
}

void publicarDatosSensores() {
  if (client.connected()) {
    StaticJsonDocument<300> jsonDoc;
    jsonDoc["humedad"] = humedad;
    jsonDoc["luz"] = resistencia_luz;
    jsonDoc["humedad_min"] = humedadMin;
    jsonDoc["humedad_max"] = humedadMax;
    jsonDoc["luz_min"] = luzMin;
    jsonDoc["luz_max"] = luzMax;
    jsonDoc["relay_agua"] = digitalRead(PIN_RELAY_AGUA);
    jsonDoc["modo_auto"] = modoAutomaticoGlobal;
    jsonDoc["sensor_seguridad"] = sensorSeguridadOk;
    jsonDoc["intentos_seguridad_fallidos"] = intentosSeguridadFallidos;
    jsonDoc["valor_seguridad"] = analogRead(PIN_SEGURIDAD_MOISTURE);
    jsonDoc["uptime"] = millis() - tiempoInicio;
    
    char buffer[384];
    serializeJson(jsonDoc, buffer);
    client.publish(TOPIC_TELEMETRY_PUBLISH, buffer);
    
    Serial.print("📤 Publicado a ");
    Serial.print(TOPIC_TELEMETRY_PUBLISH);
    Serial.print(": ");
    Serial.println(buffer);
  }
}

void verificarRiegoAutomatico() {
  if (modoAutomaticoGlobal) {
    if (humedad < HUMEDAD_MINIMA) {
      if (digitalRead(PIN_RELAY_AGUA) == LOW) {
        // Verificar seguridad antes de activar
        if (verificarSeguridadSensor()) {
          digitalWrite(PIN_RELAY_AGUA, HIGH);
          Serial.println("⚙️ RIEGO AUTOMÁTICO: Humedad baja. Bomba ENCENDIDA.");
        } else {
          Serial.println("🚨 RIEGO AUTOMÁTICO: No se puede activar - Sensor no seguro");
        }
      }
    } else if (humedad > HUMEDAD_MAXIMA) {
      if (digitalRead(PIN_RELAY_AGUA) == HIGH) {
        desactivarRiegoSeguro();
        Serial.println("⚙️ RIEGO AUTOMÁTICO: Humedad suficiente. Bomba APAGADA.");
      }
    }
  }
}

void imprimirEstadoSistema() {
  Serial.println("\n📊 ESTADO ACTUAL DEL SISTEMA:");
  Serial.print("💧 Humedad: " + String(humedad) + "% (Raw: " + String(valorHumedad) + ")");
  Serial.println(" | Min: " + String(humedadMin) + "% Max: " + String(humedadMax) + "%");
  Serial.print("☀️ Nivel de Luz: " + String(resistencia_luz) + "% (Raw: " + String(valorResistencia) + ")");
  Serial.println(" | Min: " + String(luzMin) + "% Max: " + String(luzMax) + "%");
  Serial.println("💧 Estado Relay Agua: " + String(digitalRead(PIN_RELAY_AGUA) ? "ENCENDIDO" : "APAGADO"));
  Serial.print("🤖 Modo Riego Automático: ");
  Serial.println(modoAutomaticoGlobal ? "ACTIVADO" : "DESACTIVADO");
  Serial.print(" Seguridad Sensor: ");
  Serial.print(sensorSeguridadOk ? "OK" : "FALLO");
  Serial.print(" | Valor: " + String(analogRead(PIN_SEGURIDAD_MOISTURE)));
  Serial.println(" | Intentos fallidos: " + String(intentosSeguridadFallidos));
  Serial.print("📡 Estado MQTT: ");
  Serial.println(client.connected() ? "CONECTADO" : "DESCONECTADO");
  Serial.print("🌐 Estado WiFi: ");
  Serial.println(WiFi.isConnected() ? "CONECTADO" : "DESCONECTADO");
  if (WiFi.isConnected()) {
    Serial.print("🔗 IP: ");
    Serial.println(WiFi.localIP());
  }
  Serial.println("----------------------------------------------------");
}

bool verificarSeguridadSensor() {
  int valorSeguridad = analogRead(PIN_SEGURIDAD_MOISTURE);
  
  // Si el valor es muy alto, significa que el sensor está desconectado
  if (valorSeguridad > SENSOR_SEGURIDAD_THRESHOLD) {
    intentosSeguridadFallidos++;
    
    if (intentosSeguridadFallidos >= INTENTOS_MAXIMOS_SEGURIDAD) {
      sensorSeguridadOk = false;
      Serial.println("⚠️ ALERTA DE SEGURIDAD: Sensor de humedad desconectado!");
      Serial.print("Valor de seguridad: ");
      Serial.println(valorSeguridad);
      
      // Desactivar riego por seguridad
      if (digitalRead(PIN_RELAY_AGUA) == HIGH) {
        digitalWrite(PIN_RELAY_AGUA, LOW);
        Serial.println("🚨 RIEGO DESACTIVADO POR SEGURIDAD");
      }
    }
    return false;
  } else {
    // Sensor conectado correctamente
    if (intentosSeguridadFallidos > 0) {
      Serial.println("✅ Sensor de humedad reconectado correctamente");
    }
    intentosSeguridadFallidos = 0;
    sensorSeguridadOk = true;
    return true;
  }
}

void activarRiegoSeguro() {
  if (verificarSeguridadSensor()) {
    digitalWrite(PIN_RELAY_AGUA, HIGH);
    Serial.println("💧 Relay Agua ENCENDIDO (Sensor seguro)");
  } else {
    Serial.println("🚨 No se puede activar el riego: Sensor de humedad no seguro");
  }
}

void desactivarRiegoSeguro() {
  digitalWrite(PIN_RELAY_AGUA, LOW);
  Serial.println("💧 Relay Agua APAGADO");
}