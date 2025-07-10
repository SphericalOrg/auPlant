#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"

// Funciones para la comunicación MQTT
void setupMQTT();
void callbackMqtt(char* topic, byte* payload, unsigned int length);
void procesarMensajeAtributos(const String& mensajePayload);
void actualizarEstadoRelay(bool nuevoEstado);
void reconnectMqtt();
void publicarDatosSensores();
StaticJsonDocument<300> prepararJsonTelemetria();
void solicitarAtributos();

// Variables externas que están definidas en main.cpp
extern WiFiClient espClient;
extern PubSubClient client;
extern bool relayEstado;
extern int humedad;
extern int resistencia_luz;
extern int humedadMin, humedadMax;
extern int luzMin, luzMax;
extern bool riegoTemporizado;
extern unsigned long tiempoInicioRiego;
extern bool sensorSeguridadOk;
extern int intentosSeguridadFallidos;
extern unsigned long tiempoInicio;

#endif // MQTT_CLIENT_H
