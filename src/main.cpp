/*
  FILE: main.cpp
  AUTHOR: Lucas Emanuel da Silveira Batista
  PURPOSE: Sistema Embarcado - TTC 3
*/

#include "allfunctions.h"

void taskLTEM(void *arg);
void taskBLE(void *arg);

void setup()
{
  configurate();
  //initModem();
  restartModem();
  getModemName();
  getModemInfo();
  disableGPS();
  setNetworkMode();
  setPreferredMode();
  connectLTEM();
  serialATdecode();
  waitConnectionLTEM();
  httpGETRequest(); //Chama a função httpGETRequest, responsável pela requisição GET via protocolo http
  deserializableJSON();
  setUrlGlobalPosition();
  enableGPS();
  getCoordinatesGPS();
  configurateBLE();
  xTaskCreatePinnedToCore(taskLTEM,
                          "TaskOnApp",
                          3600,
                          NULL,
                          4,
                          NULL,
                          APP_CPU_NUM);

  xTaskCreatePinnedToCore(taskBLE,
                          "TaskOnPro",
                          3600,
                          NULL,
                          8,
                          NULL,
                          PRO_CPU_NUM);
}

void taskLTEM(void *arg)
{
  while (1)
  {
    if (isLTEMConnected)
    { // LTE-M está ligado?
      if (isGPSOn)
      {
        getCoordinatesGPS();
        delay(10);
      }
      if (sendCoordinates)
      {
        savePoints();
        httpPUTRequest();
        sendCoordinates = false;
      }
      if (updateJSON)
      {
        updateJSON = false;
        httpGETRequest(); //Chama a função httpGETRequest, responsável pela requisição GET via protocolo http
        delay(800);
      }

      //getGPS(); // Recupera posição global atual
    }
    vTaskDelay(100);
  }
}

void taskBLE(void *arg)
{
  while (1)
  {
    if (isBLEDeviceConnected)
    { // Algum dispositivo se conectou a este ônibus (ESP32) ? Se sim, faça:
      if (sendJSON)
      {
        delay(6000); // Aguardar um pouco para uma conexão estável
        sendJSONBLE(); // Envia os dados JSON referentes ao onibus para o usuário que se conectou.
        sendJSON = false;
      }
      if (checkDeviceCoordinates())
      {
        httpPUTRequest();
        sendLAT = false;
        sendLONG = false;
      }
    }
    if (countSearchDevice > 600) // com um contador de 600, a condição só sera verdadeira a cada ~1 min
    {
      searchDevicesBLE();
      Serial.print("Buscando por dispositivos");
      countSearchDevice = 0;
    }
    delay(100);
    countSearchDevice++;
  }
}

void loop()
{
  delay(1);
}