//use this version to try to get barometirc pressure from OpenWeatherMap.org
//plan - remove everything but altimeter functions - OK
//remove LCD from setup - OK
//plan - see if this will work on R4 WIFI
//it does!  - comment out linear slider to speed up
//adjust zero on dial -ok
//next, start adding copilot code

//added 5/18/26
#include <WiFi.h>
#include <HTTPClient.h>  //get error message - forge on
#include <ArduinoJson.h>

//ENTER CURRENT BAROMETRIC PRESSURE HERE
#define BAR_PRES  30.24
//units are in-Hg was 30.15

#define ALT 9  //PWM for dial altimeter
#define LIN 10 //PWM for linear slider

//added 2/20/21
float LinMin = 820; //pulse width for zero feet on slider
float LinMax = 2200; //pulse witdth for 500 feet on slider

int pulseAlt = 2443; //calibrated pulse width for altimeter servo 10/3
int i = 0;  //added 9/14

//added from BMP280
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp; // I2C

//#define CUR_PRES 100597 //units are in bar
int HEIGHT;
float R;
int LinPW; 

 

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

}


 
void loop()
{
 Serial.print(F("Pressure = "));
 Serial.print(bmp.readPressure());
 Serial.println(" Pa");
 Serial.print(F("Approx altitude = "));
 Serial.print(bmp.readAltitude(BAR_PRES*33.86)); //added 9/13
 Serial.println(" m");
 Serial.println();
 delay(2000);
 
HEIGHT =(bmp.readAltitude(BAR_PRES*33.86))*3.2808; //height in feet

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
