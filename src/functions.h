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
void httpShowStatusCode();
void sendDataTest();
void setValueBusBLE();
void setValueBusDriverBLE();
void setValueCalendarBLE();
void setValueCharacteristcs();
void deserializableRequest();
void setUrlGlobalPosition();
void TaskRunningOnAppCore(void *arg);
void TaskRunningOnProtocolCore(void *arg);
void scanBLE();
void configurateBeaconBLE();

void sendJSONBus()
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

    pCharacteristic_TX->setValue(auxBytes); //Setting the json to the characteristic
    pCharacteristic_TX->notify();           //Notify the connected client
    Serial.println(auxBytes);
    delay(500);
  }

  int REST = responseBody.length() % MAX_SIZE;
  if (REST != 0)
  {
    char auxBytes[REST + 1] = "";
    for (int j = 0; j <= REST; j++)
      auxBytes[j] = responseBody[j + (responseBody.length() - REST)];

    pCharacteristic_TX->setValue(auxBytes); //Setting the json to the characteristic
    pCharacteristic_TX->notify();           //Notify the connected client
    Serial.println(auxBytes);
    delay(500);
  }

  pCharacteristic_TX->setValue("OK!");
  pCharacteristic_TX->notify();
  delay(500);
}

/* double calculateVelocity(){

  if(((p2.latitude ?? 0) == 0) || ((p2.longitude ?? 0) == 0))
    return 0;
  else if(((p2.timestamp ?? 0) == 0))
    return 0;
    
  double distance = distanceOnGeoid(p1.latitude, p1.longitude, p2.latitude,p2.longitude);
  double time = differenceTimestamp(p1.timestamp,p2.timestamp);
  double speed_mps = distance / time ;
  return speed_mps;
  //double speed_kph = (speed_mps * 3600.0) / 1000.0;
  //return speed_kph;
} */

double differenceTimestamp(double timestamp1, double timestamp2)
{
  return (timestamp2 - timestamp1);
}

double distanceOnGeoid(double lat1, double lon1, double lat2, double lon2)
{

  // Convert degrees to radians
  lat1 = lat1 * M_PI / 180.0;
  lon1 = lon1 * M_PI / 180.0;

  lat2 = lat2 * M_PI / 180.0;
  lon2 = lon2 * M_PI / 180.0;

  // radius of earth in metres
  double r = 6378100;

  // P
  double rho1 = r * cos(lat1);
  double z1 = r * sin(lat1);
  double x1 = rho1 * cos(lon1);
  double y1 = rho1 * sin(lon1);

  // Q
  double rho2 = r * cos(lat2);
  double z2 = r * sin(lat2);
  double x2 = rho2 * cos(lon2);
  double y2 = rho2 * sin(lon2);

  // Dot product
  double dot = (x1 * x2 + y1 * y2 + z1 * z2);
  double cos_theta = dot / (r * r);

  double theta = acos(cos_theta);

  // Distance in Metres
  return r * theta;
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

  currentPosition.id = doc["currentPosition"]["id"].as<int>();
}

void scanBLE()
{
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  //pBLEScan->setInterval(mSeconds);
  BLEScanResults foundDevices = pBLEScan->start(5);
  Serial.print("foundDevices -> ");
  int deviceCount = foundDevices.getCount();
  Serial.println(deviceCount);
  for (uint32_t i = 0; i < deviceCount; i++)
  {
    BLEAdvertisedDevice device = foundDevices.getDevice(i);
    Serial.print("Device ");
    Serial.print(i + 1);
    Serial.print(" -> ");
    Serial.println(device.getAddress().toString().c_str());
    /*   if (strcmp(device.getName(), "Parking") == 0)
    {
      Serial.print("We found a device named \"Parking\"");
    }  */
  }
  Serial.println("Vector -> ");
  for (int i = 0; i < adressesDevicesDetected.size(); i++)
  {
    Serial.print("Device ");
    Serial.print(i + 1);
    Serial.print(" -> ");
    Serial.println(adressesDevicesDetected[i].getAddress().toString().c_str());
  }
  adressesDevicesDetected.clear();
}

void configurateBeaconBLE()
{

  BLEBeacon ble_beacon = BLEBeacon();
  ble_beacon.setManufacturerId(ID_FABRICANTE_BEACON);
  /* Configura proximityu, major e minor do Beacon */
  ble_beacon.setProximityUUID(BLEUUID(BEACON_UUID));
  ble_beacon.setMajor(MAJOR_BEACON);
  ble_beacon.setMinor(MINOR_BEACON);
  /* Configura advertiser BLE */
  BLEAdvertisementData advertisement_data = BLEAdvertisementData();
  BLEAdvertisementData scan_response_data = BLEAdvertisementData();
  /* Indica que Bluetooth clássico não deve ser suportado */
  advertisement_data.setFlags(0x04);
  /* Informando os dados do beacon */
  std::string strServiceData = "";
  strServiceData += (char)BEACON_DATA_SIZE;
  strServiceData += (char)BEACON_DATA_TYPE;
  strServiceData += ble_beacon.getData();
  advertisement_data.addData(strServiceData);
  /* configura informações dos dados a serem enviados pelo beacon e informações de scan
       no advertiser */
  pAdvertising->setAdvertisementData(advertisement_data);
  pAdvertising->setScanResponseData(scan_response_data);
}

void configurateBLE()
{
  /* 
  // Converte int para char, e define o nome do dispositivo com o n° da linha correspondente ao ônibus do json
  String deviceName = String(bus.line);
  char deviceNameChar[deviceName.length()];
  deviceName.toCharArray(deviceNameChar, deviceName.length() + 1);
  //Serial.println(deviceNameChar);
  BLEDevice::init(deviceNameChar);                // Cria um dispositivo BLE com o nome da linha
  
  BLEServer *pServer = BLEDevice::createServer(); // Cria um Servidor BLE
  pServer->setCallbacks(new MyServerCallbacks()); // Define as funções de callback para o ajuste de conexões e desconexões
  Serial.println("Waiting for a client connection to notify ...");
  BLEService *pService = pServer->createService(SERVICE_UUID);                                                                                        //Create the BLE Service
  pCharacteristic_TX = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ); //BLE2902 needed to notify
  pCharacteristic_TX->addDescriptor(new BLE2902());                                                                                                   // Add a Descriptor to the Characteristic
  pCharacteristic_RX_LAT = pService->createCharacteristic(CHARACTERISTIC_UUID_RX_LAT, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic_RX_LAT->setCallbacks(new CharacteristicCallbacks_LAT());
  pCharacteristic_RX_LONG = pService->createCharacteristic(CHARACTERISTIC_UUID_RX_LONG, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic_RX_LONG->setCallbacks(new CharacteristicCallbacks_LONG());
  pService->start();  */
  /* Inicializa e configura advertising */
  //pAdvertising = pServer->getAdvertising();//Start advertising
  /* Inicializa e configura advertising 
   pAdvertising = BLEDevice::getAdvertising();
   BLEDevice::startAdvertising();
  configurateBeaconBLE();
  pAdvertising->start();
  */

  Serial.begin(9600);
  Serial.println("Fazendo inicializacao do beacon...");

  /* Configura breathing light */
  //pinMode(GPIO_BREATHING_LIGHT, OUTPUT);
  //digitalWrite(GPIO_BREATHING_LIGHT, LOW);

  /* Cria e configura um device e server BLE */
  String deviceName = String(bus.line);
  char deviceNameChar[deviceName.length()];
  deviceName.toCharArray(deviceNameChar, deviceName.length() + 1);

  BLEDevice::init(deviceNameChar);
  //BLEDevice::init("ESP32 - Beacon BLE");
  BLEServer *pServer = BLEDevice::createServer();

  pServer->setCallbacks(new MyServerCallbacks()); // Define as funções de callback para o ajuste de conexões e desconexões
  Serial.println("Waiting for a client connection to notify ...");
  BLEService *pService = pServer->createService(SERVICE_UUID);                                                                                        //Create the BLE Service
  pCharacteristic_TX = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ); //BLE2902 needed to notify
  pCharacteristic_TX->addDescriptor(new BLE2902());                                                                                                   // Add a Descriptor to the Characteristic
  pCharacteristic_RX_LAT = pService->createCharacteristic(CHARACTERISTIC_UUID_RX_LAT, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic_RX_LAT->setCallbacks(new CharacteristicCallbacks_LAT());
  pCharacteristic_RX_LONG = pService->createCharacteristic(CHARACTERISTIC_UUID_RX_LONG, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic_RX_LONG->setCallbacks(new CharacteristicCallbacks_LONG());
  pService->start();

  /* Inicializa e configura advertising */
  pAdvertising = BLEDevice::getAdvertising();
  BLEDevice::startAdvertising();
  configurateBeaconBLE();
  /* Começa a funcionar como beacon (advertiser entra em ação) */
  pAdvertising->start();

  Serial.println("O beacon foi inicializado e ja esta operando.");
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

void setUrlGlobalPosition()
{
  strcpy(urlPUTRequest, urlGlobalPosition);
  Serial.print("current-Position-id: ");
  Serial.println(currentPosition.id);
  char id[10];
  sprintf(id, "%d", currentPosition.id);
  strcat(urlPUTRequest, id);
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
  int err = http.put(urlPUTRequest, content_type, json);
  if (err != 0)
  {
    Serial.println(F("failed to connect"));
    delay(10000);
    return;
  }
  //httpShowStatusCode();
  httpDisconnect();
}

void enableGPS()
{
  modem.sendAT("+SGPIO=0,4,1,1"); //Set SIM7000G GPIO4 HIGH ,turn on GPS power where CMD:AT+SGPIO=0,4,1,1
  modem.enableGPS();
  isGPS_ON = true;
}

void disableGPS()
{
  modem.disableGPS(); // Set SIM7000G GPIO4 LOW ,turn off GPS power where CMD:AT+SGPIO=0,4,1,0
  modem.sendAT("+SGPIO=0,4,1,0");
  isGPS_ON = false;
}

void savePoints()
{
  p2.latitude = p1.latitude;
  p2.longitude = p1.longitude;
  p2.timestamp = p1.timestamp;

  p1.latitude = currentPosition.latitude;
  p1.longitude = currentPosition.longitude;
  p1.timestamp = currentPosition.timestamp;
}

bool checkDeviceCoordinates()
{
  if ((sendLAT && sendLONG) == true)
    return true;
  else
    return false;
}

void getGPS()
{

  if (modem.getGPS(&currentPosition.latitude, &currentPosition.longitude))
  { // Se há latitude e longitude, faça:
    gettimeofday(&currentPosition.timestamp, NULL);
    Serial.printf("latitude:%f longitude:%f\n", currentPosition.latitude, currentPosition.longitude);
    httpPUTRequest();
    isGPSEnable = true;
    //savePoints();
  }
  else
  { // Se não, espere:
    Serial.print("getGPS ");
    Serial.println(millis());
    isGPSEnable = false;
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
