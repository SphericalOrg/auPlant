#ifndef SYSPROVEVENT_H
#define SYSPROVEVENT_H

#include <Arduino.h>
#include <WiFiProv.h>

// Declaración de la función del manejador de eventos
void SysProvEvent(arduino_event_t *sys_event);

// Variable externa para indicar si el provisioning ha terminado
extern bool provisioningComplete;

#endif