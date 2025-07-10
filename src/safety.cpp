#include "safety.h"

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
