#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"  // Incluir el archivo de configuración

// --- Variables globales ---
int valorHumedad;
int valorResistencia;
int humedad;
int resistencia_luz;

int humedadMin = 100, humedadMax = 0;
int luzMin = 100, luzMax = 0;
unsigned long tiempoInicio;
bool modoAutomaticoGlobal = false;

// --- Objetos WiFi y MQTT ---
WiFiClient espClient;
PubSubClient client(espClient);

// --- Intervalos ---
unsigned long ultimoMensajeMqtt = 0;
unsigned long ultimoSerialPrint = 0;

// --- Declaración de funciones ---
void setup_wifi();
void callbackMqtt(char* topic, byte* payload, unsigned int length);
void reconnectMqtt();
void publicarDatosSensores();
void verificarRiegoAutomatico();

int main() {
    Serial.begin(115200);
    Serial.println("\n🌱 Sistema de Monitoreo y Control MQTT Iniciando...");

    pinMode(PIN_RELAY_LUZ, OUTPUT);
    pinMode(PIN_RELAY_AGUA, OUTPUT);
    pinMode(PIN_MOISTURE, INPUT);
    pinMode(PIN_FOTOR, INPUT);

    digitalWrite(PIN_RELAY_LUZ, LOW);
    digitalWrite(PIN_RELAY_AGUA, LOW);

    // Configuración de la atenuación para el sensor de luz
    analogSetAttenuation(ADC_11db); 

    setup_wifi(); // Conectar a WiFi
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(callbackMqtt);

    tiempoInicio = millis();

    // Lecturas iniciales para estadísticas
    delay(100);
    valorHumedad = analogRead(PIN_MOISTURE);
    valorResistencia = analogRead(PIN_FOTOR);
    humedad = int(((float(valorHumedad) / 4095.0) - 1.0) * -100.0);
    resistencia_luz = int((float(valorResistencia) / 4095.0) * 100.0);

    if (humedad < 0) humedad = 0; if (humedad > 100) humedad = 100;
    if (resistencia_luz < 0) resistencia_luz = 0; if (resistencia_luz > 100) resistencia_luz = 100;

    humedadMin = humedad;
    humedadMax = humedad;
    luzMin = resistencia_luz;
    luzMax = resistencia_luz;

    Serial.println("✅ Sistema iniciado correctamente.");
    Serial.print("🤖 Modo de riego automático inicial: ");
    Serial.println(modoAutomaticoGlobal ? "ACTIVADO" : "DESACTIVADO");

    // El ciclo principal de ejecución en un sistema embebido
    while (true) {
        if (!client.connected()) {
            reconnectMqtt();
        }
        client.loop(); // Mantener la conexión MQTT y procesar mensajes

        unsigned long now = millis();

        // Publicar datos por MQTT periódicamente
        if (now - ultimoMensajeMqtt > INTERVALO_MENSAJE_MQTT) {
            ultimoMensajeMqtt = now;
            publicarDatosSensores();
        }

        // Leer sensores continuamente para estadísticas y riego automático
        valorHumedad = analogRead(PIN_MOISTURE);
        valorResistencia = analogRead(PIN_FOTOR);

        humedad = int(((float(valorHumedad) / 4095.0) - 1.0) * -100.0);
        resistencia_luz = int((float(valorResistencia) / 4095.0) * 100.0);

        if (humedad < 0) humedad = 0; if (humedad > 100) humedad = 100;
        if (resistencia_luz < 0) resistencia_luz = 0; if (resistencia_luz > 100) resistencia_luz = 100;

        if (humedad < humedadMin) humedadMin = humedad;
        if (humedad > humedadMax) humedadMax = humedad;
        if (resistencia_luz < luzMin) luzMin = resistencia_luz;
        if (resistencia_luz > luzMax) luzMax = resistencia_luz;

        verificarRiegoAutomatico(); // Lógica de riego automático

        // Imprimir estado en Serial
        if (now - ultimoSerialPrint > INTERVALO_SERIAL_PRINT) {
            ultimoSerialPrint = now;
            Serial.println("\n📊 ESTADO ACTUAL DEL SISTEMA:");
            Serial.print("💧 Humedad: " + String(humedad) + "% (Raw: " + String(valorHumedad) + ")");
            Serial.println(" | Min: " + String(humedadMin) + "% Max: " + String(humedadMax) + "%");
            Serial.print("☀️ Nivel de Luz: " + String(resistencia_luz) + "% (Raw: " + String(valorResistencia) + ")");
            Serial.println(" | Min: " + String(luzMin) + "% Max: " + String(luzMax) + "%");
            Serial.println("💡 Estado Relay Luz: " + String(digitalRead(PIN_RELAY_LUZ) ? "ENCENDIDO" : "APAGADO"));
            Serial.println("💧 Estado Relay Agua: " + String(digitalRead(PIN_RELAY_AGUA) ? "ENCENDIDO" : "APAGADO"));
            Serial.print("🤖 Modo Riego Automático: "); Serial.println(modoAutomaticoGlobal ? "ACTIVADO" : "DESACTIVADO");
            Serial.println("----------------------------------------------------");
        }
        delay(100); // Pequeño delay para estabilidad
    }
}

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("📡 Conectando a WiFi: ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("✅ WiFi conectado!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
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
        if (mensajePayload == "luz_on") {
            digitalWrite(PIN_RELAY_LUZ, HIGH);
            Serial.println("💡 Relay Luz ENCENDIDO por MQTT");
        } else if (mensajePayload == "luz_off") {
            digitalWrite(PIN_RELAY_LUZ, LOW);
            Serial.println("💡 Relay Luz APAGADO por MQTT");
        } else if (mensajePayload == "agua_on") {
            modoAutomaticoGlobal = false; 
            digitalWrite(PIN_RELAY_AGUA, HIGH);
            Serial.println("💧 Relay Agua ENCENDIDO por MQTT (Modo Auto DESACTIVADO)");
        } else if (mensajePayload == "agua_off") {
            modoAutomaticoGlobal = false; 
            digitalWrite(PIN_RELAY_AGUA, LOW);
            Serial.println("💧 Relay Agua APAGADO por MQTT (Modo Auto DESACTIVADO)");
        }
    }
}

void reconnectMqtt() {
    while (!client.connected()) {
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
    StaticJsonDocument<128> jsonDocTelemetria;
    jsonDocTelemetria["temperature"] = humedad;
    char bufferTelemetria[128];
    serializeJson(jsonDocTelemetria, bufferTelemetria);
    client.publish(TOPIC_TELEMETRY_PUBLISH, bufferTelemetria);
    Serial.print("📤 Publicado a "); Serial.print(TOPIC_TELEMETRY_PUBLISH); Serial.print(": "); Serial.println(bufferTelemetria);
}

void verificarRiegoAutomatico() {
    if (modoAutomaticoGlobal) {
        if (humedad < 20) {
            if (digitalRead(PIN_RELAY_AGUA) == LOW) {
                digitalWrite(PIN_RELAY_AGUA, HIGH);
                Serial.println("⚙️ RIEGO AUTOMÁTICO: Humedad baja. Bomba ENCENDIDA.");
            }
        } else if (humedad > 70) {
            if (digitalRead(PIN_RELAY_AGUA) == HIGH) {
                digitalWrite(PIN_RELAY_AGUA, LOW);
                Serial.println("⚙️ RIEGO AUTOMÁTICO: Humedad suficiente. Bomba APAGADA.");
            }
        }
    }
}
