#define TINY_GSM_MODEM_SIM7000 // MODEM TYPE

#include <Arduino.h>
#include <ArduinoJson.h>
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <sys/time.h>

#define RXPin 3 // RX2
#define TXPin 1 // TX2
#define MAX_SIZE 100

#define SerialAT Serial1
#define UART_BAUD 9600
#define PIN_DTR 25
#define PIN_TX 27                 // transmissão de dados
#define PIN_RX 26                 // recepcção de dados
#define PWR_PIN 4                 // energia
#define GSM_PIN "8486"            // GSM PIN
#define uS_TO_S_FACTOR 1000000ULL // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 60          // Time ESP32 will go to sleep (in seconds)
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_TX "beb5483e-36e1-4688-b7f5-ea07361b26a1"
#define CHARACTERISTIC_UUID_RX_LAT "68dadf0a-1323-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_RX_LONG "7ac3dc76-1323-11eb-adc1-0242ac120002"
bool LTE_M_Connected = false;
bool BLE_deviceConnected = false;
bool sendJSON = true;
bool sendLAT = false;
bool sendLONG = false;
bool updateJSON = false; // indicador de atualização do JSON
const char server[] = "34.95.187.30";
const char busId[] = "/api/buses/1";
const char urlGlobalPosition[] = "/api/globalPositions/";
char urlPUTRequest[25];
const int port = 80;
const char apn[] = "zap.vivo.com.br";
const char gprsUser[] = "vivo";
const char gprsPass[] = "vivo";
const char *content_type = "application/json; charset=utf-8";
String responseBody = "{}"; // resposta da requisicao GET http
const int capacity = 200;
bool isGPSEnable = false;
bool isGPS_ON = false;
bool ready = false;
StaticJsonDocument<50> docGPS;
String pieces[24], input;
int counter, lastIndex, numberOfPieces = 24;

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
HttpClient http(client, server, port);

struct GlobalPosition
{
    int id;
    float latitude;
    float longitude;
    struct timeval timestamp;
};
struct Bus
{
    int id;
    int line;
    bool isAvailable;
};

Bus bus;
GlobalPosition currentPosition;
GlobalPosition p1;
GlobalPosition p2;
BLECharacteristic *pCharacteristic_TX;
BLECharacteristic *pCharacteristic_RX_LAT;
BLECharacteristic *pCharacteristic_RX_LONG;

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        BLE_deviceConnected = true;
        sendJSON = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        BLE_deviceConnected = false;
        sendJSON = false;
    };
};

//callback para eventos das características
class CharacteristicCallbacks_LAT : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        if (!isGPSEnable) // se GPS nao estiver disponível
        {
            std::string rxValue = characteristic->getValue();
            currentPosition.latitude = ::atof(rxValue.c_str());
            sendLAT = true;
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
            currentPosition.longitude = ::atof(rxValue.c_str());
            sendLONG = true;
        }
            
    } //onWrite
};

