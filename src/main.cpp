/*
  FILE: main.cpp
  AUTHOR: Lucas Emanuel da Silveira Batista
  PURPOSE: Sistema Embarcado - TTC 3
*/

#include "functions.h"

void setup() {
  configurate();
  //initModem();
  restartModem();
  getModemName();
  getModemInfo();
  disableGPS();
  setNetworkMode();
  setPreferredMode();
  connectLTE_M();
  testGPRS();
  httpGETRequest(); //Chama a função httpGETRequest, responsável pela requisição GET via protocolo http
  deserializableRequest();
  configurateBLE();
  //enableGPS();
  //configurateGPS();
}

void loop() {
  
  if(LTE_M_Connected){ // LTE-M está ligado?
    if(updateJSON){
      httpGETRequest();
      updateJSON = false;
    }
    //setGPS(); // Recupera posição global atual
  }
  if(BLE_deviceConnected){ // Algum dispositivo se conectou a este ônibus (ESP32) ? Se sim, faça:
    Serial.println("\n device connected...");
    sendBusBLE();
    delay(100);
  }   
}