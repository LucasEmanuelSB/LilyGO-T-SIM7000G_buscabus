
/* GPS */
void enableGPS(); // Habilita o GPS
void disableGPS(); // Desabilita o GPS
void getCoordinatesGPS(); // Localiza antena de GPS e recupera latitude e longitude atual

void enableGPS()
{
  modem.sendAT("+SGPIO=0,4,1,1"); //Set SIM7000G GPIO4 HIGH ,turn on GPS power where CMD:AT+SGPIO=0,4,1,1
  modem.enableGPS();
  isGPSOn = true;
}

void disableGPS()
{
  modem.disableGPS(); // Set SIM7000G GPIO4 LOW ,turn off GPS power where CMD:AT+SGPIO=0,4,1,0
  modem.sendAT("+SGPIO=0,4,1,0");
  isGPSOn = false;
}

void getCoordinatesGPS()
{
  if (modem.getGPS(&currentPosition.latitude, &currentPosition.longitude))
  {
    gettimeofday(&currentPosition.timestamp, NULL); //Recupera tempo atual do dia
    Serial.printf("latitude:%f longitude:%f\n", currentPosition.latitude, currentPosition.longitude);
    sendCoordinates = true; // indica que é possível enviar as coordenadas lat e long
    isGPSEnable = true; // indica que o gps está disponível
  }
  else
  {
    Serial.print("procurando antena -> ");
    Serial.println(millis());
    isGPSEnable = false;
  }
  
}
