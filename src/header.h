#define TINY_GSM_MODEM_SIM7000 // Tipo do modem 

#include <Arduino.h> 
#include <ArduinoJson.h> // Serialização e deserealização de json's
#include <TinyGsmClient.h> // Conexão com a rede LTE-M
#include <ArduinoHttpClient.h> // Requisições HTTP
#include <BLEDevice.h> // (Dispositivos) Bluetooth Low Energy 
#include <BLEUtils.h> // (Utilitários) Bluetooth Low Energy 
#include <BLEScan.h> // (Scanner) Bluetooth Low Energy 
#include <BLEAdvertisedDevice.h> // (Anunciamento) Bluetooth Low Energy 
#include <BLEServer.h> // (Servidor) Bluetooth Low Energy 
#include <BLE2902.h> // (Descritor) Bluetooth Low Energy 
#include <BLEBeacon.h> // (Beacon) Bluetooth Low Energy 
#include <sys/time.h> // Recuperação do tempo atual
#include <freertos/FreeRTOS.h> // (RTOS) Multiprocessamento
#include <freertos/task.h> // (Tarefas) Multiprocessamento
#define MAX_SIZE 500 // Tamanho máximo para o particionamento do JSON
#define SerialAT Serial1 // Comandos AT
#define UART_BAUD 9600 // Baud Rate
#define PIN_DTR 25
#define PIN_TX 27  // transmissão de dados
#define PIN_RX 26  // recepcção de dados
#define PWR_PIN 4  // energia
#define GSM_PIN "8486" // GSM PIN
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b" 
#define CHARACTERISTIC_UUID_TX "beb5483e-36e1-4688-b7f5-ea07361b26a1"
#define CHARACTERISTIC_UUID_RX_LAT "68dadf0a-1323-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_RX_LONG "7ac3dc76-1323-11eb-adc1-0242ac120002"
#define BEACON_UUID "ce936957-6db7-48f3-a3c5-515d51ad4933"
#define MAJOR_BEACON 1
#define MINOR_BEACON 1
#define ID_FABRICANTE_BEACON 0x4C00
#define BEACON_DATA ""
#define BEACON_DATA_SIZE 26
#define BEACON_DATA_TYPE 0xFF
bool isLTEMConnected = false;
bool isBLEDeviceConnected = false;
bool sendJSON = true;
bool sendLAT = false;
bool sendLONG = false;
bool sendRealTimeData = false;
bool updateJSON = false; // indicador de atualização do JSON
const char server[] = "35.199.104.230";
const char busId[] = "/api/buses/1";
const char urlRealTimeData[] = "/api/realTimeData/";
char urlPUTRequest[25];
const int port = 80;
const char apn[] = "zap.vivo.com.br";
const char gprsUser[] = "vivo";
const char gprsPass[] = "vivo";
const char *content_type = "application/json; charset=utf-8";
String responseBody = "{}"; // resposta da requisicao GET http
const int scanTime = 7;
const int mSeconds = 100;
const int minRSSI = -80;
bool newDeviceDetected = false;
std::vector<BLEAdvertisedDevice> adressesDevicesDetected;
BLEAdvertising *pAdvertising;
int countSearchDevice = 0;
const int capacity = 200;
bool isGPSEnable = false;
bool isGPSOn = false;
bool ready = false;
StaticJsonDocument<100> docRealTime;
String pieces[24], input;
int counter, lastIndex, numberOfPieces = 24;
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
HttpClient http(client, server, port);

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

struct GlobalPosition {
    float latitude;
    float longitude;
    struct timeval timestamp;
};

struct RealTimeData
{
    int id;
    float velocity;
    int nDevices;
    GlobalPosition currentPosition;
};

struct Bus
{
    int id;
    int line;
    bool isAvailable;
};

Bus bus;
RealTimeData realTimeData;
GlobalPosition p1;
GlobalPosition p2;
BLECharacteristic *pCharacteristic_TX;
BLECharacteristic *pCharacteristic_RX_LAT;
BLECharacteristic *pCharacteristic_RX_LONG;

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        isBLEDeviceConnected = true;
        sendJSON = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        isBLEDeviceConnected = false;
        sendJSON = false;
    };
};

class CharacteristicCallbacks_LAT : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        if (!isGPSEnable) // se GPS nao estiver disponível
        {
            std::string rxValue = characteristic->getValue();
            realTimeData.currentPosition.latitude = ::atof(rxValue.c_str());
            
            p2.latitude = p1.latitude;
            p1.latitude = realTimeData.currentPosition.latitude;
            sendLAT = true;
            delay(10);
        }

    } //onWrite
};

class CharacteristicCallbacks_LONG : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        if (!isGPSEnable) // se GPS nao estiver disponível
        {
            std::string rxValue = characteristic->getValue();
            realTimeData.currentPosition.longitude = ::atof(rxValue.c_str());
            p2.longitude = p1.longitude;
            p1.longitude = realTimeData.currentPosition.longitude;
            sendLONG = true;
            delay(10);
        }

    } //onWrite
};

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        //const char * adress = advertisedDevice.getAddress().toString().c_str();
        //Serial.print("Device found -> ");
        //Serial.println(adress);
        if(advertisedDevice.getRSSI() > minRSSI)
            adressesDevicesDetected.push_back(advertisedDevice);
    }
};