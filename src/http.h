
/* HTTP*/
void httpGETRequest(); // Realiza uma requisição GET
void httpPUTRequest(); // Realiza uma requisição PUT
void httpShowStatusCode(); // Recupera código do status da requisição feita
void httpDisconnect(); // Encerra a conexão http

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
