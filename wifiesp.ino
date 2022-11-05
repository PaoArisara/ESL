#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

//=================================================================================================
WiFiClient   espClient;
PubSubClient client(espClient);             //สร้างออปเจ็ค สำหรับเชื่อมต่อ mqtt
//=================================================================================================
const char* ssid = "CE-ESL";               //wifi name
const char* password = "ceeslonly
";         //wifi password
//=================================================================================================
const char* mqtt_broker = "broker.hivemq.com";   //IP mqtt server
const int   mqtt_port = 1883;               //port mqtt server
//=================================================================================================

#define AOUT_PIN 36
#define DHTPIN 4
#define DHTTYPE DHT22
#define LIGHT_SENSOR_PIN 34

#define TFT_CS     12
#define TFT_RST    14
#define TFT_DC     13
#define TFT_SCLK   22
#define TFT_MOSI   21
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

int humidity;
float h ;
float t ;
float f ;
float hic;
int ldr;
int check;


char humi[100];
char temp[100];
char out[100];
char sun[100];
char ch[100];


DHT dht(DHTPIN, DHTTYPE);

void setup_wifi() {   //ฟังก์ชั่นเชื่อมต่อwifi
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);              //เลือกโหมดรับ wifi
  WiFi.begin(ssid, password);       //เชื่อมต่อ wifi
  while (WiFi.status() != WL_CONNECTED)     //รอจนกว่าจะเชื่อมต่อwifiสำเร็จ
  {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  check = 1;
}

void reconnect() {  //ฟังก์ชั่นเชื่อมต่อmqtt
  client.setServer(mqtt_broker, mqtt_port);   //เชื่อมต่อmqtt
  while (!client.connected()) //รอจนกว่าจะเชื่อมต่อmqttสำเร็จ
  {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str()))
      Serial.println("Public emqx mqtt broker connected");
    else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  setup_wifi(); //เชื่อมต่อwifi
  reconnect();  //เชื่อมต่อmqtt
  dht.begin();
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3); 
}

void loop()
{
  sprintf(ch,"%d",check);
  client.publish("aris/sta", ch);

  
  //เซนเซอร์วัดความชื่นในดิน
  humidity = analogRead(AOUT_PIN); //4095
  humidity = 100-humidity/40.95;
  sprintf(humi,"%d",humidity);
  client.publish("aris/soil", humi);
  
//================================================================================

//เซนเซอร์วัดอุณหภูมิ
  h = dht.readHumidity();
  t = dht.readTemperature();
  f = dht.readTemperature(true);
  hic = dht.computeHeatIndex(t, h, false);
  
  if (isnan(h) || isnan(t)) {
  Serial.println(F("Failed to read from DHT sensor!"));
  }
  else
  {
  sprintf(temp,"%.2f",t);
  Serial.print(temp);
  client.publish("aris/temp", temp);
  sprintf(out,"%.2f",h);
  Serial.print(out);
  client.publish("aris/out", out); 
  } 

 //LDR sensor.
  ldr = analogRead(LIGHT_SENSOR_PIN);
  ldr = 100-ldr/40.95;
  sprintf(sun,"%d",ldr);
  client.publish("aris/sun", sun);

  //เงื่อนไข
  //sad
  if (humidity <30 || ldr<50){
    show();
    sad();
  }
  //Smile
  else if ((humidity <=80 && humidity >= 30) && (ldr<=95 && ldr>=50)){
    show();
    smile();
  }
  //Normal
  else if(humidity > 80 || ldr >95){
    show();
    normal();
  }
  delay(5000);

}

void show(){
  tft.fillScreen(ST7735_BLACK);
  tft.drawLine(85, 0, 85, 128, ST77XX_RED);
  tft.setCursor(5, 20);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print("Slivyy");
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(0);
  tft.setCursor(2, 50);
  tft.print("humiSoil:");
  tft.print(humidity);
  tft.print("%");
  tft.setCursor(2, 65);
  tft.print("humiAir:");
  tft.print(out);
  tft.print("%");
  tft.setCursor(2, 80);
  tft.print("Temp:");
  tft.print(temp);
  tft.print(" C");
  tft.setCursor(2, 95);
  tft.print("Light:");
  tft.print(ldr);
  tft.print("%");
  
}
void smile(){
  tft.drawLine(110, 32, 110, 64, ST77XX_GREEN);
  tft.drawLine(136, 32, 136, 64, ST77XX_GREEN);
  tft.drawLine(104, 80, 144, 80, ST77XX_GREEN);
  tft.drawLine(104, 80, 109, 90, ST77XX_GREEN);
  tft.drawLine(144, 80, 139, 90, ST77XX_GREEN);
  tft.drawLine(109, 90, 139, 90, ST77XX_GREEN);
}

void normal(){
  tft.drawLine(110, 32, 110, 64, ST77XX_YELLOW);
  tft.drawLine(136, 32, 136, 64, ST77XX_YELLOW);
  tft.drawLine(104, 90, 144, 90, ST77XX_YELLOW);
}

void sad(){
  tft.drawLine(110, 32, 110, 64, ST77XX_RED);
  tft.drawLine(136, 32, 136, 64, ST77XX_RED);
  tft.drawLine(104, 80, 144, 80, ST77XX_RED);
  tft.drawLine(104, 80, 99,  90, ST77XX_RED);
  tft.drawLine(144, 80, 149, 90, ST77XX_RED);
}
