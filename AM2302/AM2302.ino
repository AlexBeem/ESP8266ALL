// Статья размещена здесь - https://radioprog.ru/post/869
// Оригинал - https://lastminuteengineers.com/esp8266-dht11-dht22-web-server-tutorial/
// В перевод статьи перетащены все ошибки из оригинала ))) в коде ошибки устранены


// V.003 08/02/2021
// - добавлено автообновление страницы раз в 2 секунды
// - исправлена инициализация датчика DHT22 (инит, а потом подаём питание с цифрового пина)

#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#include <Ticker.h>
Ticker flipper;

#define DEBUG

// Раскомментируйте одну из строк ниже в зависимости от того, какой датчик вы используете!
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define DHTVSS D5


/* Введите свои SSID и пароль */
const char* ssid = "ssid1";  // SSID
const char* password = "pass1"; // пароль
const char* host = "yandex.ru";

ESP8266WebServer server(80);

// датчик DHT
uint8_t DHTPin = 2;
bool ret;
bool reti;
const int interval = 1 * 60; // 1 минута
int tm = interval;

// инициализация датчика DHT.
DHT dht(DHTPin, DHTTYPE);

float Temperature;
float Humidity;

void flip() {
  tm--;
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float TempCstat,/*float TempFstat,*/float Humiditystat)
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  // ptr +="<meta http-equiv=\"refresh\" content=\"2; charset=UTF8\">\n";
  ptr += "<meta http-equiv=\"refresh\" content=\"60\"charset=\"UTF-8\">\n";
  ptr += "<link href=\"https://fonts.googleapis.com/css?family=Open+Sans:300,400,600\" rel=\"stylesheet\">\n";
  ptr += "<title>ESP8266 Weather Report</title>\n";
  ptr += "<style>html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #333333;}\n";
  ptr += "body{margin-top: 50px;}\n";
  ptr += "h1 {margin: 50px auto 30px;}\n";
  ptr += ".side-by-side{display: inline-block;vertical-align: middle;position: relative;}\n";
  ptr += ".humidity-icon{background-color: #3498db;width: 30px;height: 30px;border-radius: 50%;line-height: 36px;}\n";
  ptr += ".humidity-text{font-weight: 600;padding-left: 15px;font-size: 19px;width: 160px;text-align: left;}\n";
  ptr += ".humidity{font-weight: 300;font-size: 60px;color: #3498db;}\n";
  ptr += ".temperature-icon{background-color: #f39c12;width: 30px;height: 30px;border-radius: 50%;line-height: 40px;}\n";
  ptr += ".temperature-text{font-weight: 600;padding-left: 15px;font-size: 19px;width: 160px;text-align: left;}\n";
  ptr += ".temperature{font-weight: 300;font-size: 60px;color: #f39c12;}\n";
  ptr += ".superscript{font-size: 17px;font-weight: 600;position: absolute;right: -20px;top: 15px;}\n";
  ptr += ".data{padding: 10px;}\n";
  ptr += "</style>\n";

  // AJAX ADDED
  ptr += "<script>\n";
  ptr += "setInterval(loadDoc,1000);\n";
  ptr += "function loadDoc() {\n";
  ptr += "var xhttp = new XMLHttpRequest();\n";
  ptr += "xhttp.onreadystatechange = function() {\n";
  ptr += "if (this.readyState == 4 && this.status == 200) {\n";
  ptr += "document.getElementById(\"webpage\").innerHTML =this.responseText}\n";
  ptr += "};\n";
  ptr += "xhttp.open(\"GET\", \"/\", true);\n";
  ptr += "xhttp.send();\n";
  ptr += "}\n";
  ptr += "</script>\n";
  // END AJAX

  ptr += "</head>\n";
  ptr += "<body>\n";

  ptr += "<div id=\"webpage\">\n";

  ptr += "<h1>ESP8266 HOME Метео</h1>\n";
  ptr += "<div class=\"data\">\n";
  ptr += "<div class=\"side-by-side temperature-icon\">\n";
  ptr += "<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n";
  ptr += "width=\"9.915px\" height=\"22px\" viewBox=\"0 0 9.915 22\" enable-background=\"new 0 0 9.915 22\" xml:space=\"preserve\">\n";
  ptr += "<path fill=\"#FFFFFF\" d=\"M3.498,0.53c0.377-0.331,0.877-0.501,1.374-0.527C5.697-0.04,6.522,0.421,6.924,1.142\n";
  ptr += "c0.237,0.399,0.315,0.871,0.311,1.33C7.229,5.856,7.245,9.24,7.227,12.625c1.019,0.539,1.855,1.424,2.301,2.491\n";
  ptr += "c0.491,1.163,0.518,2.514,0.062,3.693c-0.414,1.102-1.24,2.038-2.276,2.594c-1.056,0.583-2.331,0.743-3.501,0.463\n";
  ptr += "c-1.417-0.323-2.659-1.314-3.3-2.617C0.014,18.26-0.115,17.104,0.1,16.022c0.296-1.443,1.274-2.717,2.58-3.394\n";
  ptr += "c0.013-3.44,0-6.881,0.007-10.322C2.674,1.634,2.974,0.955,3.498,0.53z\"/>\n";
  ptr += "</svg>\n";
  ptr += "</div>\n";
  ptr += "<div class=\"side-by-side temperature-text\">Температура</div>\n";
  ptr += "<div class=\"side-by-side temperature\">";
  ptr += (float)TempCstat;
  ptr += "<span class=\"superscript\">°C</span></div>\n";
  //    ptr +="<span class=\"superscript\">°C</span></div>\n";
  ptr += "</div>\n";
  ptr += "<div class=\"data\">\n";
  ptr += "<div class=\"side-by-side humidity-icon\">\n";
  ptr += "<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n\"; width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n";
  ptr += "<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
  ptr += "c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
  ptr += "</svg>\n";
  ptr += "</div>\n";
  ptr += "<div class=\"side-by-side humidity-text\">Влажность</div>\n";
  ptr += "<div class=\"side-by-side humidity\">";
  ptr += (float)Humiditystat;
  ptr += "<span class=\"superscript\">%</span></div>\n";
  ptr += "</div>\n";

  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

void handle_OnConnect()
{
  Temperature = dht.readTemperature(); // получить значение температуры
  Humidity = dht.readHumidity();       // получить значение влажности

  if (isnan(Humidity) || isnan(Temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  server.send(200, "text/html", SendHTML(Temperature, Humidity));
  Serial.print(F("Humidity: "));
  Serial.print(Humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(Temperature);
  Serial.println(F("°C "));
}

void setup()
{
  Serial.begin(115200);
  delay(100);

  wifiMulti.addAP("ssid1", "1234");
  wifiMulti.addAP("ssid2", "1234");
 

  while (wifiMulti.run() != WL_CONNECTED) {
#ifdef DEBUG
    Serial.print(".");
#endif
    delay(500);

  } //

#ifdef DEBUG
  Serial.println();                       // отправляем в Serial данные о подключении
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.println();
#endif

  //  pinMode(DHTPin, INPUT_PULLUP);

  dht.begin();
  pinMode(DHTVSS, OUTPUT);
  delay(500);
  digitalWrite(DHTVSS, HIGH);
  // D0 - GPIO6
  // D1 - GPIO5  // SDA I2C
  // D2 - GPIO4  // SCL I2C
  // D3 - GPIO0
  // D4 - GPIO2

  Serial.println("Connecting to ");
  Serial.println(ssid);

  // подключаемся к локальной wi-fi сети
  // WiFi.begin(ssid, password);

  // проверить, подключился ли wi-fi модуль к wi-fi сети
  //  while (WiFi.status() != WL_CONNECTED)
  //  {
  //   delay(1000);
  //  Serial.print(".");
  // }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

  flipper.attach(1, flip);                 // запускаем таймер

}

void loop()
{
  server.handleClient();
  if (tm == 0) {                       // если таймер отработал
    flipper.detach();                 // выключаем
    tm = interval;
    // сбрасываем переменную таймера
    int j = 0;
    int k = 0;
    for (int i = 0; i < 3; i++) {
      ret = Ping.ping(WiFi.gatewayIP());
      if (ret) j++;
      reti = Ping.ping(host);
      if (reti) k++;
    }
    if (!j && !k)ESP.restart();
    flipper.attach(1, flip);          // включаем прерывание по таймеру
  }
}
