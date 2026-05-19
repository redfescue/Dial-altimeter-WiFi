// 5/19/26 notes
//add copilot code and see if works
//first see if openWeatherMap has albany oregon

//use this version to try to get barometirc pressure from OpenWeatherMap.org
//plan - remove everything but altimeter functions - OK
//remove LCD from setup - OK
//plan - see if this will work on R4 WIFI
//it does!  - comment out linear slider to speed up
//adjust zero on dial -ok
//next, start adding copilot code

//added 5/18/26
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

const char* ssid = "Stillnet";
const char* password = "Lds3706#";
const char* apiKey = "7474bc92d324dfdf2b907155f8513389";
const char* city = "Albany,Oregon,US";
const char* weatherHost = "api.openweathermap.org";



//ENTER CURRENT BAROMETRIC PRESSURE HERE
//#define BAR_PRES  30.24
//units are in-Hg was 30.15

#define ALT 9  //PWM for dial altimeter
#define LIN 10 //PWM for linear slider

float barometricPressure = 30.24; //will be updated from API (in-Hg)

//added 2/20/21
float LinMin = 820; //pulse width for zero feet on slider
float LinMax = 2200; //pulse witdth for 500 feet on slider

int pulseAlt = 2443; //calibrated pulse width for altimeter servo 10/3
int i = 0;  //added 9/14

//added from BMP280

Adafruit_BMP280 bmp; // I2C

//#define CUR_PRES 100597 //units are in bar
int HEIGHT;
float R;
int LinPW; 

//WiFi variables
unsigned long lastWeatherUpdate = 0;
const unsigned long WEATHER_UPDATE_INTERVAL = 600000; // 10 minutes




 

void setup()
{
  pinMode(ALT,OUTPUT); //added 9/14 altimeter ouput pin
 pinMode(LIN,OUTPUT); //added 2/11/21 linear slider output
 
 digitalWrite(ALT, LOW);  //added for troubleshooting 10/1
 digitalWrite(LIN, LOW); //added 2/11/21 - not sure this is needed

Serial.begin(9600);
Serial.println(F("BMP280 test"));
  
  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
//added 9/11
 bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

 pinMode(LED_BUILTIN, OUTPUT); //added 9/1/20

//move to here

connectToWiFi();



}


 
void loop()
{

//Fetch updated barometric pressure every 10 minutes
  if (millis() - lastWeatherUpdate > WEATHER_UPDATE_INTERVAL) {
    fetchWeatherData();
    lastWeatherUpdate = millis();
  }


//connectToWiFi();

 Serial.print(F("Pressure = "));
 Serial.print(bmp.readPressure());
 Serial.println(" Pa");
 
 Serial.print(F("Barometric Pressure (from API) = "));
  Serial.print(barometricPressure);
  Serial.println(" in-Hg");

 
 
 
 Serial.print(F("Approx altitude = "));
 Serial.print(bmp.readAltitude(barometricPressure*33.86)); //added 9/13
 Serial.println(" m");
 Serial.println();
 delay(2000);
 
HEIGHT =(bmp.readAltitude(barometricPressure*33.86))*3.2808; //height in feet

//added 10/8 to take dial to zero feet before displaying altitude
for (i= 0; i < 150; i++) { 
digitalWrite(ALT, HIGH); //start servo pulse
delayMicroseconds(2370);  //use to calibrate 10/3 2443
digitalWrite(ALT, LOW);
delay(20);
}

delay(1500);

for (i= 0; i < 300; i++) { 
digitalWrite(ALT, HIGH); //start servo pulse
delayMicroseconds(pulseAlt-HEIGHT*4.026);  //calibrated 10/3
digitalWrite(ALT, LOW);
delay(20);
}

Serial.println(pulseAlt-HEIGHT*4.073);
Serial.println();
Serial.println();


/*
//drive for linear slider
for (i= 0; i < 400; i++) { 
digitalWrite(LIN, HIGH); //start servo pulse
delayMicroseconds(LinMin);  //use to calibrate for zero feet
digitalWrite(LIN, LOW);
delay(20);
}

delay(1500);

for (i= 0; i < 400; i++) { 
digitalWrite(LIN, HIGH); //start servo pulse
delayMicroseconds(LinMax);  //use to calibrate for 500 feet
digitalWrite(LIN, LOW);
delay(20);
}

delay(1500);

R= (LinMax-LinMin)/500;
LinPW = HEIGHT*R + LinMin;

delay(3000);

//slider position for altitude
for (i= 0; i < 400; i++) { 
digitalWrite(LIN, HIGH); //start servo pulse
delayMicroseconds(LinPW);  //use to calibrate for zero feet
digitalWrite(LIN, LOW);
delay(20);
}
*/
delay(1500);

}

//****************************************************** functions
void connectToWiFi()
{
  Serial.print(F("Connecting to WiFi: "));
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("WiFi connected!"));
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    Serial.println(F("Failed to connect to WiFi"));
  }
}


void fetchWeatherData()
{
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("WiFi not connected, skipping weather update"));
    return;
  }

  Serial.println(F("Fetching weather data from OpenWeatherMap..."));

  WiFiClient client;
  
  if (!client.connect(weatherHost, 80)) {
    Serial.println(F("Failed to connect to weather server"));
    return;
  }

  //Build the request URL
  String url = "/data/2.5/weather?q=" + String(city) + "&appid=" + String(apiKey) + "&units=metric";

  //Send HTTP request
  client.print("GET " + url + " HTTP/1.1\r\n");
  client.print("Host: " + String(weatherHost) + "\r\n");
  client.print("Connection: close\r\n\r\n");

  //Read response
  String response = "";
  unsigned long timeout = millis() + 5000; // 5 second timeout
  
  while (client.connected() || client.available()) {
    if (client.available()) {
      char c = client.read();
      response += c;
    }
    if (millis() > timeout) break;
  }

  client.stop();

  //Extract JSON from response (skip HTTP headers)
  int jsonStart = response.indexOf('{');
  if (jsonStart != -1) {
    String jsonData = response.substring(jsonStart);
    parseWeatherResponse(jsonData);
  } else {
    Serial.println(F("No JSON found in response"));
  }
}


void parseWeatherResponse(String response)
{
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.print(F("JSON parse error: "));
    Serial.println(error.f_str());
    return;
  }

  //Extract barometric pressure (main.pressure is in hPa, convert to in-Hg)
  if (doc.containsKey("main") && doc["main"].containsKey("pressure")) {
    float pressureHpa = doc["main"]["pressure"];
    barometricPressure = pressureHpa * 0.02953; // Convert hPa to in-Hg
    
    Serial.print(F("Updated barometric pressure: "));
    Serial.print(barometricPressure);
    Serial.println(F(" in-Hg"));
  } else {
    Serial.println(F("Pressure data not found in response"));
  }
}











































