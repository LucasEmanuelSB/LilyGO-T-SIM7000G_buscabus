/*
  FILE: main.cpp
  AUTHOR: Lucas Emanuel da Silveira Batista
  PURPOSE: Sistema Embarcado - TTC 3
*/
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "functions.h"

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
  connectLTE_M();
  testGPRS();
  httpGETRequest(); //Chama a função httpGETRequest, responsável pela requisição GET via protocolo http
  deserializableRequest();
  setUrlGlobalPosition();
  enableGPS();
  getGPS();
  configurateBLE();
  xTaskCreatePinnedToCore(TaskRunningOnAppCore,
                          "TaskOnApp",
                          3600,
                          NULL,
                          4,
                          NULL,
                          APP_CPU_NUM);

  xTaskCreatePinnedToCore(TaskRunningOnProtocolCore,
                          "TaskOnPro",
                          3600,
                          NULL,
                          8,
                          NULL,
                          PRO_CPU_NUM);
}

void TaskRunningOnAppCore(void *arg)
{
  while (1)
  {
    if (LTE_M_Connected)
    { // LTE-M está ligado?
      if (isGPS_ON)
      {
        getGPS();
        delay(10);
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

void TaskRunningOnProtocolCore(void *arg)
{
  while (1)
  {
    if (BLE_deviceConnected)
    { // Algum dispositivo se conectou a este ônibus (ESP32) ? Se sim, faça:
      //Serial.println("....");
      if (sendJSON)
      {
        delay(6000);
        sendJSONBus();
        sendJSON = false;
      }
      if (checkDeviceCoordinates())
      {
        httpPUTRequest();
        sendLAT = false;
        sendLONG = false;
      }
    }
    scanBLE();
    delay(1000);
    vTaskDelay(100);
  }
}
void loop()
{
  Serial.print(__func__);
  Serial.print(" : ");
  Serial.print(xTaskGetTickCount());
  Serial.print(" : ");
  Serial.print("Arduino loop is running on core:");
  Serial.println(xPortGetCoreID());
  Serial.println();

  delay(500);
}