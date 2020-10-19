#include "header.h"

void enableGPS();
void disableGPS();
void getGPS();
void connectLTE_M();
void disconnectLTE_M();
void powerOFFModem();
void sleepModeON();
void unlockSIM();
void setNetworkMode();
void setPreferredMode();
void getModemName();
void getModemInfo();
void restartModem();
void configurate();
void testGPRS();
void httpGETRequest();
void httpPUTRequest();
void sendJSON();
void httpShowStatusCode();
void sendDataTest();
void setValueBusBLE();
void setValueBusDriverBLE();
void setValueCalendarBLE();
void setValueCharacteristcs();
void deserializableRequest();
//void configurateGPS();
//void GPS_Timezone_Adjust();
//void displayInfo();

//callback para eventos das características
class CharacteristicCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *characteristic)
  {
    //retorna ponteiro para o registrador contendo o valor atual da caracteristica
    std::string rxValue = characteristic->getValue();
    //verifica se existe dados (tamanho maior que zero)
    if (rxValue.length() > 0)
    {

      for (int i = 0; i < rxValue.length(); i++)
      {
        Serial.print(rxValue[i]);
      }
      Serial.println();
      if (rxValue.find("ready") != -1)
      {
        
      }
    }
  } //onWrite
};

void sendJSON()
{
  Serial.println(responseBody.length());
  int LOOP = responseBody.length() / MAX_SIZE;
  for (int i = 0; i < LOOP; i++)
  {
    char auxBytes[MAX_SIZE + 1] = "";
    for (int j = 0; j < MAX_SIZE; j++)
    {
      auxBytes[j] = responseBody[j + (MAX_SIZE * i)];
    }

    pCharacteristic->setValue(auxBytes); //Setting the json to the characteristic
    pCharacteristic->notify();           //Notify the connected client
    Serial.println(auxBytes);
    delay(500);
  }

  int REST = responseBody.length() % MAX_SIZE;
  if (REST != 0)
  {
    char auxBytes[REST + 1] = "";
    for (int j = 0; j <= REST; j++)
      auxBytes[j] = responseBody[j + (responseBody.length() - REST)];

    pCharacteristic->setValue(auxBytes); //Setting the json to the characteristic
    pCharacteristic->notify();           //Notify the connected client
    Serial.println(auxBytes);
    delay(500);
  }

  pCharacteristic->setValue("OK!");
  pCharacteristic->notify();
  delay(500);
}

void testGPRS()
{

  Serial.println("\n---Starting GPRS TEST---\n");
  Serial.println("Connecting to: " + String(apn));
  if (!modem.gprsConnect(apn, gprsUser, gprsPass))
  {
    delay(10000);
    return;
  }

  Serial.print("GPRS status: ");
  if (modem.isGprsConnected())
  {
    Serial.println("connected");
  }
  else
  {
    Serial.println("not connected");
  }

  String ccid = modem.getSimCCID();
  Serial.println("CCID: " + ccid);

  String imei = modem.getIMEI();
  Serial.println("IMEI: " + imei);

  String cop = modem.getOperator();
  Serial.println("Operator: " + cop);

  IPAddress local = modem.localIP();
  Serial.println("Local IP: " + String(local));

  int csq = modem.getSignalQuality();
  Serial.println("Signal quality: " + String(csq));

  SerialAT.println("AT+CPSI?"); //Get connection type and band
  delay(500);
  if (SerialAT.available())
  {
    String r = SerialAT.readString();
    Serial.println(r);
  }
  Serial.println("\n---End of GPRS TEST---\n");
}

void deserializableRequest()
{
  DynamicJsonDocument doc(12000);
  DeserializationError err = deserializeJson(doc, responseBody); // Realiza a deserialização do json e guarda resultado de erro se houver

  if (err)
  {
    Serial.print("Error: ");
    Serial.println(err.c_str());
    return;
  }
  //Armazena os dados que serão úteis para identificação do onibus
  bus.id = doc["id"].as<int>();
  bus.line = doc["line"].as<int>();
  Serial.print(bus.line);
  bus.isAvailable = doc["isAvailable"].as<bool>();
}
/*
  busDriver.id = doc["busDriver"]["id"].as<int>();
  busDriver.name = doc["busDriver"]["name"].as<String>();
  busDriver.averageRate = doc["busDriver"]["averageRate"].as<String>();

  calendar.id = doc["itinerary"]["calendar"]["id"].as<int>();

  JsonArray weeks = doc["itinerary"]["calendar"]["weeks"].as<JsonArray>();
  for (int i = 0; i < weeks.size(); i++)
    calendar.weeks[i] = weeks[i].as<String>();

  JsonArray weekendsHolidays = doc["itinerary"]["calendar"]["weekendsHolidays"].as<JsonArray>();
  for (int i = 0; i < weekendsHolidays.size(); i++)
    calendar.weekendsHolidays[i] = weekendsHolidays[i].as<String>();
} */

void configurateBLE()
{
  // Converte int para char, e define o nome do dispositivo com o n° da linha correspondente ao ônibus do json
  String deviceName = String(bus.line);
  char deviceNameChar[deviceName.length()];
  deviceName.toCharArray(deviceNameChar, deviceName.length() + 1);
  Serial.println(deviceNameChar);
  BLEDevice::init(deviceNameChar);                // Cria um dispositivo BLE com o nome da linha
  BLEServer *pServer = BLEDevice::createServer(); // Cria um Servidor BLE
  pServer->setCallbacks(new MyServerCallbacks()); // Define as funções de callback para o ajuste de conexões e desconexões
  Serial.println("Waiting for a client connection to notify ...");
  BLEService *pService = pServer->createService(SERVICE_UUID);                                                                                      //Create the BLE Service
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_BUS, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ); //BLE2902 needed to notify
  pCharacteristic->addDescriptor(new BLE2902());                                                                                                    // Add a Descriptor to the Characteristic
  pServer->getAdvertising()->start();                                                                                                               //Start advertising
  pService->start();                                                                                                                                //Start the Service                 //Start the Service
}

void configurate()
{
  // Set console baud rate
  Serial.begin(9600);
  delay(10);
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, HIGH);
  delay(300);
  digitalWrite(PWR_PIN, LOW);
  Serial.println("\nWait...");
  SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
}

void initModem()
{
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  if (!modem.init())
  {
    Serial.println("Failed to restart modem, attempting to continue without restarting");
  }
}

void restartModem()
{
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  if (!modem.restart())
  {
    Serial.println("Failed to restart modem, attempting to continue without restarting");
  }
  delay(2000);
}

void setNetworkMode()
{
  /*
    2 Automatic
    13 GSM only
    38 LTE only
    51 GSM and LTE only
* * * */

  String res;
  do
  {
    res = modem.setNetworkMode(38);
    delay(500);
  } while (res != "OK");
}

void setPreferredMode()
{
  /* 
   1 CAT-M
   2 NB-Iot
   3 CAT-M and NB-IoT
* * * */

  String res;
  do
  {
    res = modem.setPreferredMode(1);
    delay(500);
  } while (res != "OK");
}

void unlockSIM()
{ // Unlock your SIM card with a PIN if needed
  if (GSM_PIN && modem.getSimStatus() != 3)
  {
    modem.simUnlock(GSM_PIN);
  }
}

void getModemName()
{
  String name = modem.getModemName();
  delay(500);
  Serial.println("Modem Name: " + name);
}

void getModemInfo()
{
  String modemInfo = modem.getModemInfo();
  delay(500);
  Serial.println("Modem Info: " + modemInfo);
}

void httpShowStatusCode()
{
  int status = http.responseStatusCode();
  Serial.print(F("Response status code: "));
  Serial.println(status);
  if (!status)
  {
    delay(10000);
    return;
  }
}

void httpDisconnect()
{
  // Shutdown
  http.stop();
  Serial.println(F("Server disconnected"));
}

void httpGETRequest()
{

  Serial.print(F("Performing HTTP GET request... "));
  int err = http.get(busId);
  if (err != 0)
  {
    Serial.println(F("failed to connect"));
    delay(10000);
    return;
  }

  httpShowStatusCode();

  Serial.println(F("Response Headers:"));
  while (http.headerAvailable())
  {
    String headerName = http.readHeaderName();
    String headerValue = http.readHeaderValue();
    Serial.println("    " + headerName + " : " + headerValue);
  }

  int length = http.contentLength();
  if (length >= 0)
  {
    Serial.print(F("Content length is: "));
    Serial.println(length);
  }
  if (http.isResponseChunked())
  {
    Serial.println(F("The response is chunked"));
  }

  responseBody = http.responseBody();
  Serial.println(F("Response:"));
  Serial.println(responseBody);

  Serial.print(F("Body length is: "));
  Serial.println(responseBody.length());

  httpDisconnect();
}

void httpPUTRequest()
{
  docGPS["latitude"] = currentPosition.latitude;
  docGPS["longitude"] = currentPosition.longitude;
  // Convert the document to an object
  String json;
  serializeJson(docGPS, json);

  Serial.print(F("Performing HTTP PUT request... "));
  Serial.println(json);
  int err = http.put(globalPosition, content_type, json);
  if (err != 0)
  {
    Serial.println(F("failed to connect"));
    delay(10000);
    return;
  }
  //httpShowStatusCode();
  httpDisconnect();
}

/* void setValueBusBLE()
{
  StaticJsonDocument<100> doc;
  doc["id"] = bus.id;
  doc["line"] = bus.line;
  doc["isAvailable"] = bus.isAvailable;
  char jsonChar[100];
  serializeJson(doc, jsonChar);
  pCharacteristic_Bus->setValue(jsonChar);
  Serial.println(jsonChar);
  //pCharacteristic_Bus->notify();
}

void setValueBusDriverBLE()
{
  StaticJsonDocument<100> doc;
  doc["id"] = busDriver.id;
  doc["name"] = busDriver.name;
  doc["averageRate"] = busDriver.averageRate;
  char jsonChar[100];
  serializeJson(doc, jsonChar);

  pCharacteristic_BusDriver->setValue(jsonChar);
  Serial.println(jsonChar);
  //pCharacteristic_BusDriver->notify();
}

void setValueCalendarBLE()
{

  StaticJsonDocument<1024> doc;
  JsonObject obj = doc.to<JsonObject>();
  obj["id"] = calendar.id;

  JsonArray weeks = obj.createNestedArray("weeks");
  int i = 0;
  while (calendar.weeks[i] != "\0")
  {
    weeks.add(calendar.weeks[i]);
    i++;
  }

  JsonArray weekendsHolidays = obj.createNestedArray("weekendsHolidays");
  i = 0;
  while (calendar.weekendsHolidays[i] != "\0")
  {
    weekendsHolidays.add(calendar.weekendsHolidays[i]);
    i++;
  }

  char jsonChar[1024];
  serializeJson(obj, jsonChar);

  pCharacteristic_Calendar->setValue(jsonChar);
  Serial.println(jsonChar);
  //pCharacteristic_Calendar->notify();
}

void setValueCharacteristcs()
{
  setValueBusBLE();
  setValueBusDriverBLE();
  setValueCalendarBLE();
} */

void enableGPS()
{
  modem.sendAT("+SGPIO=0,4,1,1"); //Set SIM7000G GPIO4 HIGH ,turn on GPS power where CMD:AT+SGPIO=0,4,1,1
  modem.enableGPS();
  isGPSEnable = true;
}

void disableGPS()
{
  modem.disableGPS(); // Set SIM7000G GPIO4 LOW ,turn off GPS power where CMD:AT+SGPIO=0,4,1,0
  modem.sendAT("+SGPIO=0,4,1,0");
  isGPSEnable = false;
}

void getGPS()
{

  if (modem.getGPS(&currentPosition.latitude, &currentPosition.longitude))
  { // Se há latitude e longitude, faça:
    Serial.printf("latitude:%f longitude:%f\n", currentPosition.latitude, currentPosition.longitude);
    httpPUTRequest();
  }
  else
  { // Se não, espere:
    Serial.print("getGPS ");
    Serial.println(millis());
  }

  //Conexao com modulo GPS
  //  while (Serial_GPS.available() > 0)
  //    if (gps.encode(Serial_GPS.read()))
  //      displayInfo();
  //  if (millis() > 5000 && gps.charsProcessed() < 10)
  //  {
  //    Serial.println(F("No GPS detected: check wiring."));
  //    while (true);
  //  }
  //  delay(1000);
}

void connectLTE_M()
{

  SerialAT.println("AT+CGDCONT?");
  delay(500);
  if (SerialAT.available())
  {
    input = SerialAT.readString();
    for (int i = 0; i < input.length(); i++)
    {
      if (input.substring(i, i + 1) == "\n")
      {
        pieces[counter] = input.substring(lastIndex, i);
        lastIndex = i + 1;
        counter++;
      }
      if (i == input.length() - 1)
      {
        pieces[counter] = input.substring(lastIndex, i);
      }
    }
    // Reset for reuse
    input = "";
    counter = 0;
    lastIndex = 0;

    for (int y = 0; y < numberOfPieces; y++)
    {
      for (int x = 0; x < pieces[y].length(); x++)
      {
        char c = pieces[y][x]; //gets one byte from buffer
        if (c == ',')
        {
          if (input.indexOf(": ") >= 0)
          {
            String data = input.substring((input.indexOf(": ") + 1));
            if (data.toInt() > 0 && data.toInt() < 25)
            {
              modem.sendAT("+CGDCONT=" + String(data.toInt()) + ",\"IP\",\"" + String(apn) + "\",\"0.0.0.0\",0,0,0,0");
            }
            input = "";
            break;
          }
          // Reset for reuse
          input = "";
        }
        else
        {
          input += c;
        }
      }
    }
  }
  else
  {
    Serial.println("Failed to get PDP!");
  }

  Serial.println("\n\n\nWaiting for network...");
  if (!modem.waitForNetwork())
  {
    delay(10000);
    return;
  }

  if (modem.isNetworkConnected())
  {
    Serial.println("Network connected");
  }

  LTE_M_Connected = true;
}

void disconnectLTE_M()
{
  modem.gprsDisconnect();
  if (!modem.isGprsConnected())
  {
    Serial.println("GPRS disconnected");
  }
  else
  {
    Serial.println("GPRS disconnect: Failed.");
  }
  LTE_M_Connected = false;
}

void powerOFFModem()
{
  // Try to power-off (modem may decide to restart automatically)
  // To turn off modem completely, please use Reset/Enable pins
  modem.poweroff();
  Serial.println("Modem Poweroff.");
}

void sleepModeON()
{
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  delay(200);
  esp_deep_sleep_start();
  // Do nothing forevermore
  while (true)
  {
    modem.maintain();
  }
}

//void configurateGPS(){
//  Serial_GPS.begin(9600, SERIAL_8N1, RXPin, TXPin);
//  //Mostra informacoes iniciais no serial monitor
//  Serial.println(F("Data, Hora, Latitude e Longitude"));
//  Serial.println(F("Modulo GPS GY-NEO6MV2"));
//  Serial.print(F("Biblioteca TinyGPS++ v. "));
//  Serial.println(TinyGPSPlus::libraryVersion());
//  Serial.println();
//}

//void displayInfo()
//{
//  //Mostra informacoes no Serial Monitor
//  Serial.print(F("Location: "));
//  if (gps.location.isValid())
//  {
//    Serial.print(gps.location.lat(), 6); //latitude
//    Serial.print(F(","));
//    Serial.print(gps.location.lng(), 6); //longitude
//  }
//  else
//  {
//    Serial.print(F("INVALID"));
//  }
//  Serial.print(F("  Date/Time: "));
//  if (gps.date.isValid())
//  {
//    Serial.print(gps.date.day()); //dia
//    Serial.print(F("/"));
//    Serial.print(gps.date.month()); //mes
//    Serial.print(F("/"));
//    Serial.print(gps.date.year()); //ano
//  }
//  else
//  {
//    Serial.print(F("INVALID"));
//  }
//  Serial.print(F(" "));
//  if (gps.time.isValid())
//  {
//    if (gps.time.hour() < 10) Serial.print(F("0"));
//    Serial.print(gps.time.hour()); //hora
//    Serial.print(F(":"));
//    if (gps.time.minute() < 10) Serial.print(F("0"));
//    Serial.print(gps.time.minute()); //minuto
//    Serial.print(F(":"));
//    if (gps.time.second() < 10) Serial.print(F("0"));
//    Serial.print(gps.time.second()); //segundo
//    Serial.print(F("."));
//    if (gps.time.centisecond() < 10) Serial.print(F("0"));
//    Serial.print(gps.time.centisecond());
//  }
//  else
//  {
//    Serial.print(F("INVALID"));
//  }
//  Serial.println();
//}
//void GPS_Timezone_Adjust()
//{
//  while (Serial_GPS.available())
//  {
//    if (gps.encode(Serial_GPS.read()))
//    {
//      int Year = gps.date.year();
//      byte Month = gps.date.month();
//      byte Day = gps.date.day();
//      byte Hour = gps.time.hour();
//      byte Minute = gps.time.minute();
//      byte Second = gps.time.second();
//      //Ajusta data e hora a partir dos dados do GPS
//      setTime(Hour, Minute, Second, Day, Month, Year);
//      //Aplica offset para ajustar data e hora
//      //de acordo com a timezone
//      adjustTime(UTC_offset * SECS_PER_HOUR);
//    }
//  }
//}

//void sendJSON(){
//
//    Serial.println(responseBody.length());
//      int LOOP = responseBody.length()/MAX_SIZE;
//      for(int i = 0; i < LOOP ; i++){
//        char auxBytes[MAX_SIZE+1] = "";
//          for(int j = 0; j < MAX_SIZE; j++)
//            auxBytes[j] = responseBody[j+(MAX_SIZE*i)];
//      Serial.println(auxBytes);
//      pCharacteristic->setValue(auxBytes); //Setting the json to the characteristic
//      pCharacteristic->notify(); //Notify the connected client
//      delay(50);
//      }
//
//      int REST = responseBody.length()%MAX_SIZE;
//      if(REST != 0){
//      char auxBytes[REST+1] = "";
//      for(int j = 0; j <= REST; j++)
//            auxBytes[j] = responseBody[j+(responseBody.length()- REST)];
//      Serial.println(auxBytes);
//      pCharacteristic->setValue(auxBytes); //Setting the json to the characteristic
//      pCharacteristic->notify(); //Notify the connected client
//      delay(50);
//      }
//}

//void sendDataTest(){
//  uint32_t cont = 1;
//  while(cont <= 100){
//    pCharacteristic->setValue((uint8_t*)&cont, 4);
//    pCharacteristic->notify(); //Notify the connected client
//    cont++;
//    delay(10);
//  }
//}
