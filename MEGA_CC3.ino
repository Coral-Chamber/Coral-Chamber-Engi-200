 /*******
ENGI 200 Coral Chamber Code for Arduino Mega

Written by: John Reko

Has these sections:
1. Has a section that uses 6 analog inputs to control 24 LED NEOPIXELS
2. Has a section that controls 2 digital output pins according to a digital input
from a temperature sensor
3. Has a section that controls a LCD display with different inputs
4. Has a seciton that controls a valve that can stop water flow
depending on the readings of a water level sensor

Version 1.0: Ultrasonic sensor is latest addition

Version 1.1: Adding knob to control what temperature the system is run at and a LCD 1602 display
and watter level sensors for the flow chamber and the supply bin and a start button. Maybe format
so each ringlight emits the same color. Allows use arduino uno for whole project

Version 1.2: Using Arduino MEGA with more GPIO pins to have LCD display and external temperature setting
Will use switch on the power after being plugged in. Smoothed input from Ultrasonic Sensor

Version 1.3: Added reset button, two LED light that are controlled by pins, use water level sensors instead
of ultrasonic sensors. Act like water based switch.

Version 1.4: Added 12V DC valve and two water level sensors to monitor water supply.

Version 1.5: Set lights to turn off when the potentiometer reading is low in order to eliminate low
level flicker. 
******/

#include <Adafruit_NeoPixel.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
//For Display
const int rs = 22, en = 24, d4 = 26, d5 = 28, d6 = 46, d7 = 48;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Led Pins, LED_PIN1 --> Ring Light with inputs A0 - A2
//LED_PIN2 --> Ringlight with inputs A3 - A5
#define LED_PIN1  37 //Reassign both for MEGA
#define LED_PIN2  38

//Data input pin for digital Temperature Sensor
#define ONE_WIRE_BUS 33

//Pins that are used to turn on relays for pump, fan, and heater
#define pump_relay 30
#define fan_relay 31
#define heater_relay 32

//failsafes
#define start_pin 53
#define failsafe_supply A6
#define failsafe_flow A7
#define tempset A8
#define valve_pin 40
#define speaker_pin 42

//LED Indicators
#define on_pin 34
#define warning2 39

//Define both NEOPIXEL strips to write to
Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, LED_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(24, LED_PIN2, NEO_GRB + NEO_KHZ800);

//Initialize variables

//Used to time pump, may be changed with ultrasonic sensor
int count = 0;
int delay_time = 100;

//For light smoothing to reduce the flicker in the lights
const int numReadings = 10;
const int numReadings2 = 3;
int redArr[numReadings];
int redArr2[numReadings];
int greArr[numReadings];
int greArr2[numReadings];
int bluArr[numReadings];
int bluArr2[numReadings];
int redtotal, redtotal2, redave, redave2, redIndex, redIndex2 = 0;
int greenTotal, greenTotal2, greenAve, greenAve2, greenIndex, greenIndex2 = 0;
int bluTotal, bluTotal2, bluAve, bluAve2, bluIndex, bluIndex2 = 0;

//Temperature control
int maxTemp = 30;
int minTemp = 20;
int diff = maxTemp - minTemp;


// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

void setArray(int Arr1[], int Arr_size){
  /*
    Sets all values of array to zero

    Inputs:
      Arr1[] --- the array that is being set to zero
      Arr_size --- number of items in Arr1

    Output:
      Nothing
  */
    for (int i = 0; i < Arr_size; i++){
        Arr1[i] = 0;
      }
  }

float adjustFloat(float x){
  /*
  Takes a floating point number and shortens it to one decimal place

  Doesn't round, just cuts off hundredth's place digit To enhance UI of display

  Inputs --- float x, a floating point number of an arbitrary number of digits
  Output --- z, a floating point number that is x shortened to one decimal point
      
  */
    float z;
    int y = (int) (x * 10);
    int firstDigit = y / 100;
    int secondDigit = (y - firstDigit * 100) / 10;
    int lastDigit = (y - firstDigit * 100 - secondDigit * 10);
    z = firstDigit * 10 + secondDigit + (float) lastDigit / 10.0;
    return z;
  }
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(pump_relay, OUTPUT);
  pinMode(fan_relay, OUTPUT);
  pinMode(heater_relay, OUTPUT);
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(start_pin, INPUT);
  pinMode(on_pin, OUTPUT);
  pinMode(warning2, OUTPUT);
  pinMode(valve_pin, OUTPUT);

  //Light Smoothing
  setArray(redArr, numReadings);
  setArray(redArr2, numReadings);
  setArray(greArr, numReadings);
  setArray(greArr2, numReadings);
  setArray(bluArr, numReadings);
  setArray(bluArr2, numReadings);
  
  //Allows strips to turn on
  strip.begin();
  strip.show();
  strip1.begin();
  strip1.show();
  sensors.begin(); 
}



void run_cc(bool pump_on) {
  //Version 1.2 for relay control, using count in seconds, not millis
  if(digitalRead(start_pin) == 1){
    
    digitalWrite(valve_pin, HIGH);
    count += 1;
    // Light control
    //Smoothing Looks much better than with no smoothing
    //Set up for 2 NEOPIXELS
  
    //Red color for first ringlight
    redtotal = redtotal - redArr[redIndex];
    redArr[redIndex] = analogRead(A0);
    redtotal = redtotal + redArr[redIndex];
    redIndex += 1;
    if(redIndex >= numReadings){
        redIndex = 0;
      }
    redave = redtotal / numReadings;
  
    //Red control for second ring light
    redtotal2 = redtotal2 - redArr2[redIndex];
    redArr2[redIndex2] = analogRead(A3);
    redtotal2 = redtotal2 + redArr2[redIndex2];
    redIndex2 += 1;
    if(redIndex2 >= numReadings){
        redIndex2 = 0;
      }
    
    redave2 = redtotal2 / numReadings;
  
    //Green color control for first ring light
    greenTotal = greenTotal - greArr[greenIndex];
    greArr[greenIndex] = analogRead(A1);
    greenTotal = greenTotal + greArr[greenIndex];
    greenIndex += 1;
    if(greenIndex >= numReadings){
        greenIndex = 0;
      }
    greenAve = greenTotal / numReadings;
  
  
    //Green color control for second ring light
    greenTotal2 = greenTotal2 - greArr2[greenIndex2];
    greArr2[greenIndex2] = analogRead(A4);
    greenTotal2 = greenTotal2 + greArr2[greenIndex2];
    greenIndex2 += 1;
    if(greenIndex2 >= numReadings){
        greenIndex2 = 0;
      }
    greenAve2 = greenTotal2 / numReadings;
  
    //Blue color control for first ring light
    bluTotal = bluTotal - bluArr[bluIndex];
    bluArr[bluIndex] = analogRead(A2);
    bluTotal = bluTotal + bluArr[bluIndex];
    bluIndex += 1;
    if(bluIndex >= numReadings){
        bluIndex = 0;
      }
    bluAve = bluTotal / numReadings;
    
    //Blue color control for second ring light
    bluTotal2 = bluTotal2 - bluArr2[bluIndex2];
    bluArr2[bluIndex2] = analogRead(A5);
    bluTotal2 = bluTotal2 + bluArr2[bluIndex2];
    bluIndex2 += 1;
    if(bluIndex2 >= numReadings){
        bluIndex2 = 0;
      }
    bluAve2 = bluTotal2 / numReadings;
    //Serial.println(bluAve);

    int color_array[] =  {redave, redave2, greenAve, greenAve2, bluAve, bluAve2};
    for(int i = 0; i < 6; i++){
        if(color_array[i] / 4 < 10){
            color_array[i] = 0;
          }
      }
    uint32_t ring_color1 = strip.Color((color_array[0] / 4), (color_array[2] / 4), (color_array[4] / 4));
    uint32_t ring_color2 = strip1.Color((color_array[1] / 4), (color_array[3] / 4), (color_array[5] / 4));
    strip.fill(ring_color1, 0, 23);
    strip1.fill(ring_color2, 0, 23);
  
  
    //Serial.println(bluAve2);
    sensors.requestTemperatures(); // Send the command to get temperature readings
    //Serial.print(sensors.getTempCByIndex(0));
  
    // add a reading to change temperature
    float temperature = sensors.getTempCByIndex(0);
   // running_temp += temperature;
    //average_temp = running_temp / count;

  
    int temp_read = analogRead(tempset);
    float set_temp = (float) (minTemp + diff * temp_read / 1024.0);

    //12/9 changed heat to start at 1.5 c below set
    //12/9 changed fan to start at 1c below set
       
    if( temperature <= set_temp - 1.5){
        digitalWrite(heater_relay, HIGH);
        digitalWrite(fan_relay, LOW);
        delay(100);
      }
    if(temperature >= set_temp - 1){
      delay(100);
      digitalWrite(heater_relay, LOW);
      digitalWrite(fan_relay, HIGH);
      }
    
    if(pump_on){
        digitalWrite(pump_relay, HIGH);
        digitalWrite(on_pin, HIGH);
        digitalWrite(warning2, LOW);
      }
    else{
        digitalWrite(pump_relay, LOW);
        digitalWrite(on_pin, LOW);
        digitalWrite(warning2, HIGH);
      }

      
    if(analogRead(failsafe_flow) > 400){
        uint32_t red_warning1 = strip.Color(125, 0, 0);
        uint32_t red_warning2 = strip1.Color(125, 0, 0);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("FLOW CHAMBER IS");
        lcd.setCursor(0, 1);
        lcd.print("BLOCKED. RESET!");
        digitalWrite(on_pin, LOW);
        digitalWrite(pump_relay, LOW);
        digitalWrite(fan_relay, LOW);
        digitalWrite(heater_relay, LOW);
        digitalWrite(valve_pin, LOW);
        while(true){
            digitalWrite(warning2, HIGH);
            tone(speaker_pin, 500, 500);
            strip.fill(red_warning1);
            strip1.fill(red_warning2);
            strip.show();
            strip1.show();
            delay(1000);
            digitalWrite(warning2, LOW);
            noTone(speaker_pin);
            strip.clear();
            strip1.clear();  
            strip.show();
            strip1.show();       
            delay(1000);
          }
      }
  
    //Timer for display
    //run_time = millis();
    //float seconds = (float) run_time / 1000.0;
    //float minutes = seconds / 60.0;
    //float hours = minutes / 60.0;
    //LCD Display in this section
    //float final_temp = adjustFloat(average_temp);

    //LCD Display
    lcd.setCursor(0,0);
    lcd.print("Temp Set: ");
    String s1 = (String) (adjustFloat(set_temp));
    lcd.print(s1);
    lcd.setCursor(0, 1);
    lcd.print("Temp: ");
    lcd.print(temperature);
    //End of LCD display section
  
  
    //Displays colors on pixels
    strip.show();
    strip1.show();
  }
  else{
      digitalWrite(pump_relay, LOW);
      digitalWrite(heater_relay, LOW);
      digitalWrite(fan_relay, LOW);
    }

  delay(delay_time);
}

void loop(){
  if(digitalRead(start_pin) == 1){
      if(analogRead(failsafe_supply) > 400){
          digitalWrite(warning2, HIGH);
          digitalWrite(on_pin, LOW);
          for(int i = 0; i < 10; i++){
              run_cc(false);
            }
        }
       
      else{
          run_cc(true);
          digitalWrite(on_pin, HIGH);
        }
     }
   else{

      //When on switch is OFF
      digitalWrite(pump_relay, LOW);
      digitalWrite(fan_relay, LOW);
      digitalWrite(heater_relay, LOW);
      digitalWrite(on_pin, LOW);
      digitalWrite(warning2, LOW);
      digitalWrite(valve_pin, LOW);
      strip.clear();
      strip1.clear();
      strip.show();
      strip1.show();
      lcd.clear();
    }
  }

  
