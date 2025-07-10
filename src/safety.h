#ifndef SAFETY_H
#define SAFETY_H

#include <Arduino.h>
#include "config.h"

// Funciones para el control de seguridad de sensores
bool verificarSeguridadSensor();

// Variables externas que están definidas en main.cpp
extern bool sensorSeguridadOk;
extern int intentosSeguridadFallidos;

#endif // SAFETY_H
