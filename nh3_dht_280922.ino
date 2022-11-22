//FOR OTA************************************************
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <DHT.h>

//WIFI***********************************************
const char* ssid = "KFG_EMS_WIFI";
const char* password = "Kfg#2!EMS@Passw0rd";

//NH3************************************************
int sensorValue;
int digitalValue;
float ppm1;
float ppm2;
float ppml;
float ppmlow;
float multi3;

#define RL 47
#define m 0.253
#define b 0.42
#define Ro 20
//DHT************************************************
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//SERVER*********************************************
#define INFLUX "http://172.16.253.155:8086/write?db=kfg_ems"
#define DELAY 10000

String metrics;
void sendReadings(String post_data) {
  String dburl = INFLUX;
  HTTPClient http;
  http.begin(dburl);
  http.POST(post_data);
  http.end();
}
WebServer server(80);

/*
   setup function
*/
void setup(void) {
  pinMode(21, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(2, OUTPUT);
  // Connect to WiFi network
  Serial.begin(115200);
  digitalWrite(21, HIGH);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(2, HIGH);
  digitalWrite(21, LOW);
  digitalWrite(25, LOW);

  digitalWrite(25, HIGH);
  Serial.begin(115200);
  Serial.println(F("DHTxx test!"));
  digitalWrite(5, LOW);
  dht.begin();


}

void loop(void) {
  //DHT******************************************************

  float humi = dht.readHumidity();

  float tempC = dht.readTemperature();

  float tempF = dht.readTemperature(true);


  if ( isnan(tempC) || isnan(tempF) || isnan(humi)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(humi);
    Serial.print("%");

    Serial.print("  |  ");

    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.print("°C  ~  ");
    Serial.print(tempF);
    Serial.println("°F");

  }
  delay(2000);
  if ( humi > 0) {
    digitalWrite(5, HIGH);
    String metrics = "NH3_humi1,device=EMS1,sensor=Sensor66 value="    + String(humi, 2);
    sendReadings(metrics);
    Serial.println(metrics);
    delay(2000);
    digitalWrite(5, LOW);
  } else {
    Serial.println("Failed to read from DHT sensor!");
    digitalWrite(5, HIGH);
  }
  if ( tempC > 0) {
    digitalWrite(21, HIGH);
    String metrics = "NH3_temp1,device=EMS1,sensor=Sensor66 value="    + String(tempC, 2);
    sendReadings(metrics);
    Serial.println(metrics);
    delay(2000);
    digitalWrite(5, LOW);
  }else{
    Serial.println("Failed to read from DHT sensor!");
    digitalWrite(21, HIGH);
  }

  //NH3***************************************

  sensorValue = analogRead(39); // read analog input pin 0
  digitalValue = digitalRead(34);
  if (sensorValue > 400)
  {
    digitalWrite(25, HIGH);
    delay(500);
    digitalWrite(25, LOW);
    delay(500);
    digitalWrite(25, HIGH);
    delay(500);
    digitalWrite(25, LOW);
    delay(500);
    
    float analogIn =   sensorValue;
    float multi1 = analogIn * (3.3 / 4095.0);
    float multi2 = ((5 * RL) / multi1) - RL;
    float multi3 = multi2 / Ro;
    ppm1 = pow(10, ((log10 (multi3) - b) / m));
    Serial.print(ppm1);
    Serial.println("  ppm");
    delay(2000);
    String metrics = "NH31,device=EMS1,sensor=Sensor55 value="    + String(ppm1, 2);
    sendReadings(metrics);
    Serial.println(metrics);
    delay(2000);
    digitalWrite(25, LOW);
  }
  else if (sensorValue < 400) {
    digitalWrite(25, HIGH);
    delay(5000);
    digitalWrite(25, LOW);
    delay(500);
    digitalWrite(25, HIGH);
    float analogIn =   sensorValue;
    float multi1 = analogIn * (3.3 / 4095.0);
    float multi2 = ((5 * RL) / multi1) - RL;
    multi3 = multi2 / Ro;
    ppml = pow(10, ((log10 (multi3) - b) / m));
    Serial.println(ppml);
    Serial.print(multi3);
    Serial.println("  ppm");
    delay(2000);
    String metrics = "NH31,device=EMS1,sensor=Sensor55 value="    + String(multi3, 2);
    sendReadings(metrics);
    Serial.println(metrics);
    delay(5000);
    digitalWrite(25, LOW);
  } else {
    digitalWrite(25, LOW);
    delay(500);
    digitalWrite(25, HIGH);
    delay(500);
    digitalWrite(25, LOW);
    delay(500);
    digitalWrite(5, HIGH);
    //    Serial.print(sensorValue); // prints the value read
    //    Serial.println("...Sensor valu");
    //    delay(2000);
    //    Serial.print(digitalValue);
    //    Serial.println("...digital valu");
    //    delay(1000);
    ppmlow = (multi3 * 1);
    delay(5000);
    String metrics = "NH3_2,device=EMS1,sensor=Sensor02 value="    + String(ppmlow, 2);
    sendReadings(metrics);
    Serial.println(metrics);
    delay(2000);
    digitalWrite(25, LOW);
  }
}
