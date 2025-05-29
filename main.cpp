#include "Arduino.h"

#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME "Planta v1"
#define BLYNK_AUTH_TOKEN ""

#include <BlynkSimpleEsp32.h>


char ssid[] = "";
char pass[] = "";


int pinRelayLuz = 19;
int pinRelayAgua = 18;
int pinMoisture =32;
int pinfotor = 33;
int valorHumedad;
int valorResistencia;

BlynkTimer timer;

void myTimer(){
  Blynk.virtualWrite(V0, valorHumedad);
  Blynk.virtualWrite(V1, valorResistencia);
}

BLYNK_WRITE(V2) {
  int relayState = param.asInt(); // 1 or 0 from app
  digitalWrite(pinRelayAgua, relayState);
}


void setup() {

  Serial.begin(9600);
  Serial.println("\n\n\n\n\n Serial begin");

  analogSetAttenuation(ADC_11db);
  Serial.println("ADC set to 11db\n"); 

  pinMode(pinRelayLuz, OUTPUT);
  pinMode(pinRelayAgua, OUTPUT);
  pinMode(pinMoisture, INPUT);
  pinMode(pinfotor, INPUT);
  Serial.println("\n Pins Defined.");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Blynk started");

  timer.setInterval(4000L, myTimer);

}

void loop() {
  valorHumedad = analogRead(pinMoisture);
  valorResistencia = analogRead(pinfotor);

  // Remove manual relay control here if you want to control it only from Blynk
  // digitalWrite(pinRelayAgua, HIGH);
  // delay(1000);
  // digitalWrite(pinRelayAgua, LOW);

  Serial.print("FotoResistencia ");
  Serial.println(valorResistencia);
  Serial.print("Humedad:");
  Serial.println(valorHumedad);

  Blynk.run();
  timer.run();


  delay(950);
}
