#include "irrigation.h"
#include "safety.h"

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

void activarRiegoTemporizado() {
  if (verificarSeguridadSensor()) {
    digitalWrite(PIN_RELAY_AGUA, HIGH);
    riegoTemporizado = true;
    tiempoInicioRiego = millis();
    Serial.println("💧 Riego temporizado ACTIVADO por 5 segundos");
  } else {
    Serial.println("🚨 No se puede activar el riego temporizado: Sensor de humedad no seguro");
  }
}

void verificarRiegoTemporizado() {
  if (!riegoTemporizado) {
    return; // No hay riego temporizado activo
  }
  
  unsigned long tiempoTranscurrido = millis() - tiempoInicioRiego;
  
  // Verificar si el tiempo de riego ha terminado
  if (tiempoTranscurrido >= DURACION_RIEGO) {
    finalizarRiegoTemporizado();
  } else {
    mostrarTiempoRestanteRiego(tiempoTranscurrido);
  }
}

void finalizarRiegoTemporizado() {
  riegoTemporizado = false;
  Serial.println("⏰ Riego temporizado FINALIZADO");
  
  // Comprobar el estado del relay según el topic MQTT
  if (relayEstado && verificarSeguridadSensor()) {
    // Si el topic aún indica que debe estar encendido, mantenerlo encendido
    digitalWrite(PIN_RELAY_AGUA, HIGH);
    Serial.println("💧 Relay Agua se mantiene ENCENDIDO según estado MQTT");
  } else {
    // Si el topic indica apagado, o hay problemas de seguridad, apagar
    digitalWrite(PIN_RELAY_AGUA, LOW);
    Serial.println("💧 Relay Agua APAGADO");
  }
}

void mostrarTiempoRestanteRiego(unsigned long tiempoTranscurrido) {
  static unsigned long ultimoTiempoMostrado = 0;
  
  // Mostrar actualización cada segundo
  if (tiempoTranscurrido - ultimoTiempoMostrado >= 1000) {
    ultimoTiempoMostrado = tiempoTranscurrido;
    int segundosRestantes = (DURACION_RIEGO - tiempoTranscurrido) / 1000;
    
    Serial.print("💧 Riego temporizado activo - Tiempo restante: ");
    Serial.print(segundosRestantes);
    Serial.println(" segundos");
  }
}
