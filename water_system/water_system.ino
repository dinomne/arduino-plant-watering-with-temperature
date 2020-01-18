#include <SFE_BMP180.h>
#include <cactus_io_AM2302.h>
#include <Wire.h>
#define AM2302_PIN 51 // what pin on the arduino is the AM2302 data line connected to
#define ALTITUDE 295.0 // Altitude of SparkFun's HQ in Boulder, CO. in meters  2UL*60UL*
//#define d_second 1000 
unsigned long previousMillisPump = 0;
unsigned long previousMillisSystem = 0;   
const unsigned long intervalPump =  2UL*60UL*60UL*1000UL;   
const unsigned long intervalSystem =   60UL*1000UL;   
AM2302 dht(AM2302_PIN);

const int pumpOn =  2; 
const int redButtonPin = 1;     
int redButtonState = 0;
int flag=0;
int drySoilLevel = 600;
int temp = 0;
int bars = 0;

// You will need to create an SFE_BMP180 object, here called "pressure":
SFE_BMP180 pressure;


void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(redButtonPin, INPUT);   
  pinMode(pumpOn, OUTPUT);
  
  Serial.begin(9600); // open serial port, set the baud rate as 9600 bps
  Serial.println("Starting");
  
  // Initialize the wether sensor (it is important to get calibration values stored on the device).
  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.
    Serial.println("BMP180 init fail\n\n");
    //while(1); // Pause forever.
  }
}


void enginePulse()
{
  digitalWrite(pumpOn, HIGH);
  delay(2000);
  digitalWrite(pumpOn, LOW);
}

boolean needsWatering()
{
  int moisture = analogRead(0); //connect sensor to Analog 0
  Serial.println(moisture); //print the value to serial port
  
  if ( moisture < drySoilLevel && isItTime(previousMillisPump, intervalPump))
  {
        previousMillisPump = millis();
        return true;
  }
  else 
  {
  return false;
  }
}

boolean isItTime(long lastTime, long interval)
{
  if (millis() - lastTime >= interval )
  {
    return true;
  } else {
  return false;
  }
}


void mesureEnv()
{
   char status;
  double T,P,p0,a;
  // If you want sea-level-compensated pressure, as used in weather reports,
  // you will need to know the altitude at which your measurements are taken.
  // We're using a constant called ALTITUDE in this sketch:
  Serial.println();
  Serial.print("provided altitude: ");
  Serial.print(ALTITUDE,0);
  Serial.print(" meters, ");
  status = pressure.startTemperature();
  if (status != 0)
  {
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Print out the measurement:
      Serial.print("temperature: ");
      Serial.print(T,2);
      Serial.print(" deg C, ");
      temp = T;
      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);
               status = pressure.getPressure(P,T);
        if (status != 0)
        {
          // Print out the measurement:
          Serial.print("absolute pressure: ");
          Serial.print(P,2);
          Serial.print(" mb, ");
          Serial.print(P*0.0295333727,2);
          Serial.println(" inHg");
          bars = P;
          p0 = pressure.sealevel(P,ALTITUDE); // we're at 1655 meters (Boulder, CO)
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(p0,2);
          Serial.print(" mb, ");
          Serial.print(p0*0.0295333727,2);
          Serial.println(" inHg");
           a = pressure.altitude(P,p0);
          Serial.print("computed altitude: ");
          Serial.print(a,0);
          Serial.print(" meters, ");
          Serial.print(a*3.28084,0);
          Serial.println(" feet");
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
}

void loop()
{
  if ( isItTime(previousMillisSystem, intervalSystem))
  {
   mesureEnv();
   if (needsWatering())
   {
        Serial.print("needs water! ");
        enginePulse();
   }
   previousMillisSystem = millis();
   Serial.println(millis());
  }
    delay(1000); //Small delay
}