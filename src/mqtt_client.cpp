#define MQTT_MAX_PACKET_SIZE 512
#include "mqtt_client.h"
#include "irrigation.h"
#include <WiFi.h>

void setupMQTT() {
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callbackMqtt);
  client.setBufferSize(MQTT_MAX_PACKET_SIZE);
}

void callbackMqtt(char* topic, byte* payload, unsigned int length) {
  String mensajePayload = "";
  for (unsigned int i = 0; i < length; i++) {
    mensajePayload += (char)payload[i];
  }
  
  String topicStr = String(topic);
  
  LOG_DEBUG_F("Mensaje MQTT recibido [%s]: %s", topicStr.c_str(), mensajePayload.c_str());
  
  // Procesamos los mensajes según el topic
  if (topicStr.startsWith("v1/devices/me/attributes")) {
    LOG_DEBUG("Procesando mensaje de atributos...");
    procesarMensajeAtributos(mensajePayload);
  } else {
    LOG_WARNING_F("Topic no manejado: %s", topicStr.c_str());
  }
}

void procesarMensajeAtributos(const String& mensajePayload) {
  StaticJsonDocument<512> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, mensajePayload);
  
  if (error) {
    LOG_ERROR_F("Error al parsear JSON: %s", error.c_str());
    return;
  }
  
  // Verificar diferentes estructuras JSON posibles
  bool encontradoSprinkler = false;
  bool nuevoEstado = false;
  
  // Caso 1: Respuesta a atributos con "shared"
  if (jsonDoc.containsKey("shared")) {
    JsonObject shared = jsonDoc["shared"];
    if (shared.containsKey("sprinkler")) {
      nuevoEstado = shared["sprinkler"].as<bool>();
      encontradoSprinkler = true;
    }
  } 
  // Caso 2: Publicación directa con "sprinkler"
  else if (jsonDoc.containsKey("sprinkler")) {
    nuevoEstado = jsonDoc["sprinkler"].as<bool>();
    encontradoSprinkler = true;
  }
  
  if (encontradoSprinkler) {
    actualizarEstadoRelay(nuevoEstado);
  }
}

void actualizarEstadoRelay(bool nuevoEstado) {
  // Si el estado es el mismo que ya tenemos, no hacer nada para evitar acciones duplicadas
  if (relayEstado == nuevoEstado) {
    LOG_DEBUG_F("Estado del relay sin cambios: %s", relayEstado ? "ACTIVADO" : "DESACTIVADO");
    return;
  }
  
  LOG_INFO_F("Cambio estado relay: %s → %s", 
            relayEstado ? "ACTIVADO" : "DESACTIVADO", 
            nuevoEstado ? "ACTIVADO" : "DESACTIVADO");
  
  relayEstado = nuevoEstado;
  
  if (relayEstado) {
    activarRiegoTemporizado();  // Activar riego por 5 segundos
  } else {
    desactivarRiegoSeguro();
  }
  
  LOG_INFO_F("Estado del relay actualizado: %s", relayEstado ? "ACTIVADO" : "DESACTIVADO");
}

void reconnectMqtt() {
  while (!client.connected() && WiFi.isConnected()) {
    LOG_INFO("Intentando conexión MQTT...");
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      LOG_INFO("¡MQTT conectado!");
      
      // Suscribirse a las respuestas de atributos
      client.subscribe(TOPIC_ATTRIBUTES_RESPONSE);
      LOG_DEBUG_F("Suscrito a: %s", TOPIC_ATTRIBUTES_RESPONSE);
      
      // Suscribirse a los atributos compartidos
      client.subscribe(TOPIC_ATTRIBUTES);
      LOG_DEBUG_F("Suscrito a: %s", TOPIC_ATTRIBUTES);
      
      // Solicitar atributos después de la suscripción exitosa
      solicitarAtributos();
      
      LOG_INFO("Todas las suscripciones MQTT realizadas correctamente");
      
    } else {
      LOG_WARNING_F("Falló la conexión MQTT, rc=%d | Reintentando en 5s...", client.state());
      delay(5000);
    }
  }
}

void publicarDatosSensores() {
  if (!client.connected()) {
    LOG_WARNING("MQTT desconectado: telemetría NO enviada");
    return;
  }

  LOG_INFO("Iniciando publicación de telemetría...");
  
  // Construir objeto JSON
  StaticJsonDocument<300> jsonDoc = prepararJsonTelemetria();
  
  // Serializar y enviar
  char buffer[384];
  size_t size = serializeJson(jsonDoc, buffer);
  
  LOG_DEBUG_F("Publicando en: %s (Tamaño: %d bytes)", TOPIC_TELEMETRY_PUBLISH, size);
  LOG_VERBOSE_F("Payload: %s", buffer);
  
  bool resultado = client.publish(TOPIC_TELEMETRY_PUBLISH, buffer);
  
  if (resultado) {
    LOG_INFO("Telemetría publicada exitosamente");
    
    // Solicitar atributos actualizados después de enviar telemetría
    solicitarAtributos();
    LOG_DEBUG("Solicitando actualización de atributos de control");
  } else {
    LOG_ERROR_F("Error al publicar telemetría. Estado MQTT: %d", client.state());
  }
}

StaticJsonDocument<300> prepararJsonTelemetria() {
  StaticJsonDocument<300> jsonDoc;
  
  // Datos de sensores
  jsonDoc["humidity"] = humedad;
  jsonDoc["light"] = resistencia_luz;
  
  // Estadísticas
  jsonDoc["min_humidity"] = humedadMin;
  jsonDoc["max_humidity"] = humedadMax;
  jsonDoc["min_light"] = luzMin;
  jsonDoc["max_light"] = luzMax;
  
  // Estado de dispositivos
  jsonDoc["relay_water"] = digitalRead(PIN_RELAY_AGUA);
  jsonDoc["status_relay"] = relayEstado;
  
  // Estado de riego
  jsonDoc["timed_irrigation"] = riegoTemporizado;
  if (riegoTemporizado) {
    jsonDoc["irrigation_time_left"] = (DURACION_RIEGO - (millis() - tiempoInicioRiego)) / 1000;
  }
  
  // Seguridad
  jsonDoc["security_sensor"] = sensorSeguridadOk;
  jsonDoc["security_atempts_failed"] = intentosSeguridadFallidos;
  jsonDoc["security_value"] = analogRead(PIN_SEGURIDAD_MOISTURE);
  
  // Uptime
  jsonDoc["uptime"] = millis() - tiempoInicio;
  
  return jsonDoc;
}

void solicitarAtributos() {
  if (client.connected()) {
    static unsigned long ultimaSolicitud = 0;
    unsigned long ahora = millis();
    
    // Evitar solicitudes demasiado frecuentes (mínimo 1 segundo entre solicitudes)
    if (ahora - ultimaSolicitud < 1000) {
      LOG_DEBUG("Evitando solicitud de atributos demasiado frecuente");
      return;
    }
    
    ultimaSolicitud = ahora;
    
    StaticJsonDocument<128> requestDoc;
    requestDoc["clientKeys"] = "deviceId";
    requestDoc["sharedKeys"] = "sprinkler";
    
    char buffer[256];
    serializeJson(requestDoc, buffer);
    
    client.publish(TOPIC_ATTRIBUTES_REQUEST, buffer);
    
    LOG_DEBUG("Solicitud de atributos enviada");
    LOG_VERBOSE_F("Payload: %s", buffer);
  }
}
