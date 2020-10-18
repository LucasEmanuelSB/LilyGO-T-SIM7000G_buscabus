#define TINY_GSM_MODEM_SIM7000 // MODEM TYPE

#include <Arduino.h>
#include <ArduinoJson.h>
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define RXPin 3 // RX2
#define TXPin 1 // TX2
#define MAX_SIZE 600

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
#define CHARACTERISTIC_UUID_BUS "beb5483e-36e1-4688-b7f5-ea07361b26a1"
#define CHARACTERISTIC_UUID_BUS_DRIVER "beb5483e-36e1-4688-b7f5-ea07361b26a2"
#define CHARACTERISTIC_UUID_CALENDAR "beb5483e-36e1-4688-b7f5-ea07361b26a3"
bool LTE_M_Connected = false;
bool BLE_deviceConnected = false;
bool sendJSONFlag = true;
bool updateJSON = false; // indicador de atualização do JSON
const char server[] = "34.95.187.30";
const char busId[] = "/api/buses/1";
const char globalPosition[] = "/api/globalPositions/1";
const int port = 80;
const char apn[] = "zap.vivo.com.br";
const char gprsUser[] = "vivo";
const char gprsPass[] = "vivo";
const char *content_type = "application/json; charset=utf-8";
String responseBody = "{}"; // resposta da requisicao GET http
const int capacity = 12480;

String pieces[24], input;
int counter, lastIndex, numberOfPieces = 24;

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
HttpClient http(client, server, port);

class MyServerCallbacks : public BLEServerCallbacks
{

    void onConnect(BLEServer *pServer)
    {
        BLE_deviceConnected = true;
        sendJSONFlag = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        BLE_deviceConnected = false;
    };
};

struct Bus
{
    int id;
    int line;
    bool isAvailable;
    BusDriver busDriver;
    Itinerary itinerary;
    Calendar calendar;
    GlobalPosition currentPosition;
};

struct BusDriver
{
    int id;
    String name;
    String averageRate; // nao é double para satisfazer a deserelizacao no cliente
};

struct Calendar
{
    int id;
    String weeks[100];
    String weekendsHolidays[100];
};

struct GlobalPosition
{
    int id;
    float latitude;
    float longitude;
};

struct Itinerary
{
    int id;
    int busId;
    int routeId;
    int calendarId;
};

struct Route
{
    int id;
};

struct BusStop
{
    int id;
    bool isTerminal;
    int adressId;
};

struct Adress
{
    int id;
    String country;
    String uf;
    String city;
    String neighborhood;
    String street;
    String cep;
    String number;
    int globalPositionId;                        
};

Bus bus;
BusDriver busDriver;
Calendar calendar;
GlobalPosition currentPosition;
BLECharacteristic *pCharacteristic_Bus;
BLECharacteristic *pCharacteristic_BusDriver;
BLECharacteristic *pCharacteristic_Calendar;
