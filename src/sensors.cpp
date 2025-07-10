#include "sensors.h"
#include <Arduino.h>
#include <WiFi.h>

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

void imprimirEstadoSistema() {
  Serial.println("\n📊 ESTADO ACTUAL DEL SISTEMA:");
  Serial.print("💧 Humedad: " + String(humedad) + "% (Raw: " + String(valorHumedad) + ")");
  Serial.println(" | Min: " + String(humedadMin) + "% Max: " + String(humedadMax) + "%");
  Serial.print("☀️ Nivel de Luz: " + String(resistencia_luz) + "% (Raw: " + String(valorResistencia) + ")");
  Serial.println(" | Min: " + String(luzMin) + "% Max: " + String(luzMax) + "%");
  Serial.println("💧 Estado Relay Agua: " + String(digitalRead(PIN_RELAY_AGUA) ? "ENCENDIDO" : "APAGADO"));
  if (riegoTemporizado) {
    unsigned long tiempoRestante = DURACION_RIEGO - (millis() - tiempoInicioRiego);
    Serial.println("⏰ Riego Temporizado: ACTIVO (Tiempo restante: " + String(tiempoRestante / 1000) + "s)");
  } else {
    Serial.println("⏰ Riego Temporizado: INACTIVO");
  }
  Serial.print("🔌 Estado Relay (MQTT): ");
  Serial.println(relayEstado ? "ACTIVADO" : "DESACTIVADO");
  Serial.print(" Seguridad Sensor: ");
  Serial.print(sensorSeguridadOk ? "OK" : "FALLO");
  Serial.print(" | Valor: " + String(analogRead(PIN_SEGURIDAD_MOISTURE)));
  Serial.println(" | Intentos fallidos: " + String(intentosSeguridadFallidos));
  Serial.print("📡 Estado MQTT: ");
  Serial.println(WiFi.isConnected() ? "CONECTADO" : "DESCONECTADO");
  Serial.print("🌐 Estado WiFi: ");
  Serial.println(WiFi.isConnected() ? "CONECTADO" : "DESCONECTADO");
  if (WiFi.isConnected()) {
    Serial.print("🔗 IP: ");
    Serial.println(WiFi.localIP());
  }
  Serial.println("----------------------------------------------------");
}
