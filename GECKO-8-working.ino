#include "DHTesp.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_PRINT Serial

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif

DHTesp dht;

int BLYNKWaterButton = 0;
int ThresholdFromBLYNK = 0;
int SENDOK = 1;

// Set Relay Pins
int relayOne = 4; //D2

//set water sesnor pin
int relayOne = 5; //D1

//ThingSpeak
unsigned long myChannelNumber = 249402;
const char* MY_TSAPIWRITEKEY = "SFK73OUL1RORIH0B";
int counterForDelay = 0;  
int ThingSpeakDelay = 25;

// blynk auth
char auth[] = "5512cf7dba05467d8301730d92243ad6";

//wifi ---------------------
const char* MY_SSID = "VennMill"; 
const char* MY_PWD = "welcome1";
int status = WL_IDLE_STATUS;
WiFiServer server(80);
WiFiClient  client;

int DHTTemperature = 0;
int DHTHumidity = 0;

int DHTTemperatureSTORE = 0;
int DHTHumiditySTORE = 0;
int failCounter = 0;

String TextForScreen = "Old Text";

BLYNK_READ(V1)
{
  Blynk.virtualWrite(V1, DHTTemperature);
}
BLYNK_READ(V2)
{
  Blynk.virtualWrite(V2, DHTHumidity);
}
BLYNK_WRITE(V3)
{
  //int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  // You can also use:
  int i = param.asInt();;
  // double d = param.asDouble();
  Serial.print("V3 state is ");
  if (i == 1){
    BLYNKWaterButton = 1;
  }
  Serial.println(BLYNKWaterButton);
  
}
BLYNK_WRITE(V4)
{
  //int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  // You can also use:
  int z = param.asInt();;
  // double d = param.asDouble();
  Serial.print("Humidity Threshold is ");
  Serial.println(z);
  ThresholdFromBLYNK = z;
}
BLYNK_READ(V5)
{
  Blynk.virtualWrite(V5, TextForScreen);
}



void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(500);
  // Wait for serial to initialize.
  while(!Serial) { }
  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");
  String thisBoard= ARDUINO_BOARD;
  Serial.println(thisBoard);

  // Autodetect is not working reliable, don't use the following line
  // dht.setup(17);
  // use this instead: 
  dht.setup(2, DHTesp::DHT11); // Connect DHT sensor to GPIO 2


  
  Serial.println("I'm awake.");
  Serial.println("StartConnectWifi");
  connectWifi();
  delay(250);

  Serial.println("Connect to ThinkSpeak + Blynk");
  Blynk.begin(auth, MY_SSID, MY_PWD);
  delay(250);
  ThingSpeak.begin(client);
  delay(250);
  
  //Set relay pins to outputs
  pinMode(relayOne, OUTPUT);
  //digitalWrite(relayOne, HIGH);   // sets relayOne on
  //Serial.println("RELAY OFF");
  relayPump("OFF",100);
  delay(500);
  
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  if (isnan(humidity) || isnan(temperature)) {
      Serial.print("Failed"); 
      failCounter++;
    }
    else{
      DHTTemperature = temperature;
      DHTHumidity = humidity;
    }

  Blynk.run();
  delay(1000);
  
  if (BLYNKWaterButton==1){
    //Serial.println("WATER!!");
    relayPump("ON",6000);
    BLYNKWaterButton = 0;
  }
  
  Serial.println(); 
  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");


  if (counterForDelay >ThingSpeakDelay){

    if (DHTHumidity < ThresholdFromBLYNK){
      relayPump("ON",6000);
      Blynk.notify("AUTO WATER");
    }

    
      counterForDelay = 0;  
      ThingSpeak.setField(1,temperature);
      ThingSpeak.setField(2,humidity);
      ThingSpeak.setField(3,failCounter);
      ThingSpeak.writeFields(myChannelNumber, MY_TSAPIWRITEKEY);
      Serial.print(counterForDelay);
      Serial.println("Data sent to Thingstream");
      //Blynk.notify("Data sent to ThingStream");
      if (failCounter>10){
        TextForScreen = failCounter;
        delay(250);
        //ESP.restart();  
      }
      else {
        TextForScreen = failCounter;
        failCounter = 0;
      }
      
    
  }
    counterForDelay++;
    delay(1000); //wait a sec (recommended for DHT11) and 15 seconds for thing speak!
}

int relayPump(String setstate, int duration){
  if (setstate=="ON"){
    digitalWrite(relayOne, LOW);   // sets relayOne on
    Serial.println("Pump ON");
    delay(duration);
    digitalWrite(relayOne, HIGH);   // sets relayOne off
    Serial.println("Pump OFF");
  }
  else if (setstate=="OFF"){
    digitalWrite(relayOne, HIGH);   // sets relayOne off
    Serial.println("Pump OFF");
  }
  else {
    Serial.print(setstate);
    }
}

void connectWifi()
{
  Serial.print("Connecting to "+*MY_SSID);
  WiFi.begin(MY_SSID, MY_PWD);
  while (WiFi.status() != WL_CONNECTED) {
  digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
  delay(1000);
  digitalWrite(BUILTIN_LED, HIGH);   // Turn the LED on (Note that LOW is the voltage level
  Serial.println("Waiting");
  }
  Serial.println("");
  Serial.println("Connected");
  digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
  Serial.println(WiFi.localIP()); 
  Serial.println("");  
}//end connect


