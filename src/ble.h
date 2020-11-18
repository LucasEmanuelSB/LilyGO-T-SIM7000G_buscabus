/* BLE */
void configurateBLE(); // Configura rede BLE
void configurateBeaconBLE(); // Configura Beacon do dispositivo
void searchDevicesBLE(); // Busca por dispositivos na cobertura do rede BLE
void sendJSONBLE(); // / Envia JSON pela rede BLE

void configurateBLE()
{
  Serial.println("Fazendo inicializacao do beacon...");

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

void searchDevicesBLE()
{
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5);
  /*
  pBLEScan->setInterval(mSeconds);
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
       if (strcmp(device.getName(), "Parking") == 0)
    {
      Serial.print("We found a device named \"Parking\"");
    } 
  } 
  */
  realTimeData.nDevices = adressesDevicesDetected.size();
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

void sendJSONBLE()
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
    delay(10);
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
    delay(10);
  }

  pCharacteristic_TX->setValue("OK!");
  pCharacteristic_TX->notify();
  delay(10);
}