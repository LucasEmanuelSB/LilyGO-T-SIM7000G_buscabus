#include "header.h"
#include "gps.h"
#include "ble.h"
#include "ltem.h"
#include "http.h"

/* OUTRAS FUNÇÕES */
void configurate(); // Configura Serial e pinagem da placa
void deserializableJSON();   // Deserializa JSON
void setUrlGlobalPosition(); // define url para requisições http
void savePoints(); // Salva os pontos de latitude e longitude em p2 e p1, respectivamente
bool checkDeviceCoordinates(); // Verifica se as coordenadas recuperados de um dispositivo por conexão BLE já estão disponíveis
double calculateVelocity(); // Calcula a velocidade do ônibus utilizando distância e tempo dos pontos p1 e p2
double differenceTimestamp(double timestamp1, double timestamp2); // Aplica a diferença dos tempos de p1 e p2
double distanceOnGeoid(double lat1, double lon1, double lat2, double lon2); // calcula a distância no globo utilizando as coordenadas

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

void deserializableJSON()
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

void setUrlGlobalPosition()
{
  strcpy(urlPUTRequest, urlGlobalPosition);
  Serial.print("current-Position-id: ");
  Serial.println(currentPosition.id);
  char id[10];
  sprintf(id, "%d", currentPosition.id);
  strcat(urlPUTRequest, id);
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

double calculateVelocity()
{
  if((p1.latitude == 0) || (p1.longitude == 0) || (p1.timestamp.tv_sec == 0))
    return 0;
  if((p2.latitude == 0) || (p2.longitude == 0) || (p2.timestamp.tv_sec == 0))
    return 0;

  double distance = distanceOnGeoid(p1.latitude, p1.longitude, p2.latitude, p2.longitude);
  double time = differenceTimestamp(p1.timestamp.tv_sec, p2.timestamp.tv_sec);
  double speed_mps = distance / time;
  return speed_mps;
}

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

