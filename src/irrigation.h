#ifndef IRRIGATION_H
#define IRRIGATION_H

#include <Arduino.h>
#include "config.h"
#include "safety.h"

// Funciones para el control de riego
void activarRiegoSeguro();
void desactivarRiegoSeguro();
void activarRiegoTemporizado();
void verificarRiegoTemporizado();
void finalizarRiegoTemporizado();
void mostrarTiempoRestanteRiego(unsigned long tiempoTranscurrido);

// Variables externas que están definidas en main.cpp
extern bool riegoTemporizado;
extern unsigned long tiempoInicioRiego;
extern bool relayEstado;

#endif // IRRIGATION_H
