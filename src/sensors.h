#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "config.h"

// Funciones para el manejo de sensores
void configurarPines();
void leerSensores();
void imprimirEstadoSistema();

// Variables externas que están definidas en main.cpp
extern int valorHumedad;
extern int valorResistencia;
extern int humedad;
extern int resistencia_luz;
extern int humedadMin, humedadMax;
extern int luzMin, luzMax;
extern bool relayEstado;
extern bool riegoTemporizado;
extern unsigned long tiempoInicioRiego;
extern bool sensorSeguridadOk;
extern int intentosSeguridadFallidos;

#endif // SENSORS_H
