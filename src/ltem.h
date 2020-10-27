
/* LTE-M */
void initModem(); // inicializa modem
void restartModem(); // reinicializa modem
void unlockSIM(); // Desbloqueia o cartão SIM
void setNetworkMode(); // Configura o modo da rede
void setPreferredMode(); // Configura a preferência do modo
void getModemName(); // Recupera nome do modem
void getModemInfo(); // Recupera outras informações do modem
void connectLTEM(); // Conecta-se a rede LTE-M
void waitConnectionLTEM(); // Aguarda pela conexão LTE-M
void disconnectLTEM(); // Desconecta-se da rede LTE-M

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

void unlockSIM()
{ // Unlock your SIM card with a PIN if needed
  if (GSM_PIN && modem.getSimStatus() != 3)
  {
    modem.simUnlock(GSM_PIN);
  }
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

void connectLTEM()
{
  Serial.println("\n--- Conexão LTE-M ---\n");
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
  Serial.println("\n------------\n");
}

void waitConnectionLTEM()
{
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
  isLTEMConnected = true;
}

void disconnectLTEM()
{
  modem.gprsDisconnect();
  if (!modem.isGprsConnected())
  {
    Serial.println("LTE-M disconnected");
  }
  else
  {
    Serial.println("LTE-M disconnected: Failed.");
  }
  isLTEMConnected = false;
}

void serialATdecode(){

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
}


