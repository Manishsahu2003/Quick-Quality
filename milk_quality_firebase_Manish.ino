#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <DFRobot_ESP_EC.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#define sensor_pin 2
#define ONE_WIRE_BUS 27                          //D2 pin of nodemcu
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);  
int laststate = LOW;
int currentstate=0;
DFRobot_ESP_EC ec;
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using I2C
#define OLED_RESET     -1 // Reset pin
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//firebase credential
#include "addons/TokenHelper.h"//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"// Insert your network credentials
#define WIFI_SSID "innovation"
#define WIFI_PASSWORD "latifwifi"
// Insert Firebase project API Key
#define API_KEY "AIzaSyDr2ABUwsWquPC_aGtlEMNxa9D_9r3ztVY"
#define DATABASE_URL "https://cross-68664-default-rtdb.firebaseio.com/" // Insert RTDB URLefine the RTDB URL */
const int potPin=34;
float ph;
float Value=0;
float urea=25;
 
float voltage, ecValue, temperature = 25;
int tdsvalue=0;
//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
#define FIREBASE_HOST "cross-68664-default-rtdb.firebaseio.com"  
#define FIREBASE_AUTH "5FO0t3USbit2IwyjQkKvwYPzCxV4WmAGgLVH8XKQ"  

void setup()
{
    pinMode(potPin,INPUT);
  Serial.begin(115200);
  EEPROM.begin(32);//needed EEPROM.begin to store calibration k in eeprom
  ec.begin();
  WiFi.mode(WIFI_STA);
  // initialize the OLED object
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
    }

  // Clear the buffer.
  display.clearDisplay();

  // Display Text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,4);
  display.println("Milk-Quality");
  display.display();
  delay(1000);
  //display.clearDisplay();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  //   ThingSpeak.begin(client);
}

void loop()
{
  tdsvalue=0;
  ph=0;
  temperature=0;
  voltage = analogRead(A0); // A0 is the gpio 36
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);  // read your temperature sensor to execute temperature compensation
  ecValue = ec.readEC(voltage, temperature); // convert voltage to EC with temperature compensation
  tdsvalue=ecValue*135;
  Serial.print("Temperature:");
  Serial.print(temperature, 2);
  Serial.println("ºC");
  Value= analogRead(potPin);
  // Serial.print(Value);
  // Serial.print(" | ");
  float voltageph=Value*(3.3/4095.0);
  Serial.print("pH:=");
  ph=(3.3*voltageph);
  ph=voltageph;
  Serial.println(ph);
  delay(500);
  // Serial.print("EC:");
  // Serial.println(ecValue, 2);
  Serial.print("TDS:");
  Serial.println(tdsvalue,0);
  display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,20);
      display.println("Urea:");
      display.setCursor(60,20);
      display.println(10,0);
      display.setCursor(90,20);
      display.println("%");
      display.setCursor(0,30);
      display.println("TDS Value:");
      display.setCursor(60,30);
      display.println(tdsvalue,0);
      display.setCursor(90,30);
      display.println("PPM");
      display.setCursor(0,40);
      display.println("Temprature:");
      display.setCursor(70,40);
      display.println(temperature);
      display.setCursor(110,40);
      display.println("ºC");
      display.setCursor(0,50);
      display.println("pH Value:");
      display.setCursor(70,50);
      display.println(ph,1);
      display.setCursor(110,50);
      display.display();
      delay(2000);
  display.clearDisplay();
  ec.calibration(voltage, temperature); // calibration process by Serail CMD
  firebasedata();
}
void firebasedata()
  {

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an float number on the database path test/int
    if (Firebase.RTDB.setFloat(&fbdo, "Milk_Quality/Urea",urea )){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
     if (Firebase.RTDB.setFloat(&fbdo, "Milk_Quality/Temprature",temperature)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
      if (Firebase.RTDB.setFloat(&fbdo, "Milk_Quality/ph_Value",ph)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
  else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  if (Firebase.RTDB.setInt(&fbdo, "Milk_Quality/Tds_Value",tdsvalue)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
  else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
   
}
  }