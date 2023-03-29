#include <DHT.h>
#include <EEPROM.h>
#include "GravityTDS.h"
 
GravityTDS gravityTds;
 
float tdsValue = 0;

int dhtPin=23;
DHT dht(dhtPin,DHT11);
float humidity;
float temperature;

int turbidityPin=36;
float volt;
float ntu;
float turbidity;
float ph;

int co2Pin=34;
int tdsPin=35;
int phPin=39;

float calibration_value = 21.34;
int phval = 0; 
unsigned long int avgval; 
int buffer_arr[10],temp;

void bsp() {
  Serial.begin(9600);
  pinMode(turbidityPin,INPUT);
  pinMode(co2Pin,INPUT);
  pinMode(tdsPin,INPUT);
  dht.begin();

  gravityTds.setPin(tdsPin);
  gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(4096);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();  //initialization
}

int readCo2Value() {
  return analogRead(co2Pin);
}

int readTdsValue() {
  return analogRead(tdsPin);
}

float readTurbidityValue() {
  volt = 0;
  for(int i=0; i<800; i++){
    volt += ((float)analogRead(turbidityPin)/1023)*5;
  }
  volt = volt/800;
  volt = round_to_dp(volt,2);
  if(volt < 2.5){
    ntu = 3000;
  }else{
    ntu = -1120.4*sq(volt)+5742.3*volt-4353.8; 
  }
  return ntu;
}

float round_to_dp( float in_value, int decimal_place )
{
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}

float readPhValue() {
 for(int i=0;i<10;i++) 
 { 
  buffer_arr[i]=analogRead(phPin);
  delay(30);
 }
 for(int i=0;i<9;i++)
 {
  for(int j=i+1;j<10;j++)
  {
    if(buffer_arr[i]>buffer_arr[j])
    {
      temp=buffer_arr[i];
      buffer_arr[i]=buffer_arr[j];
      buffer_arr[j]=temp;
    }
  }
 }
 avgval=0;
 for(int i=2;i<8;i++)
 avgval+=buffer_arr[i];
 float volt=(float)avgval*5.0/4096/6;
 float ph_act = -5.70 * volt + calibration_value;
 return ph_act;
}


void setup() {
  bsp();
}

void loop() {
  turbidity=readTurbidityValue();
  
  humidity=dht.readHumidity();
  temperature=dht.readTemperature();
  
  gravityTds.setTemperature(temperature);  
  gravityTds.update();  
  tdsValue = gravityTds.getTdsValue(); 

  ph=readPhValue();

  if(isnan(humidity) || isnan(temperature)) 
    return;
    
  Serial.print("Turbidity Value: ");
  Serial.print(turbidity);
  Serial.print(", Humidity Value: ");
  Serial.print(humidity);
  Serial.print(", Temperature Value: ");
  Serial.print(temperature);
  Serial.print(", TDS Value: ");
  Serial.print(tdsValue);
  Serial.print(", PH Value: ");
  Serial.print(ph);
  Serial.println();
  delay(4000);
}
