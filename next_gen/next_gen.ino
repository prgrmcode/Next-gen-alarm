/*
 * Project:   Next-Generation Digital Alarm Clock
 * Authors:   Adrian & Harun
 * Hardware:  Arduino Nano Every / PIR motion sensor / RTC DS3231 / 
 *            Grove-4-Digit / Crowtail Speaker
 * 
 */

 

#include <Wire.h>   // Library of wire in supoort of RTC library
//#include <RTClib.h> 
//#include <DS3231.h> // Library for the DS3231 real-time clock (RTC).
#include <DS1307RTC.h>                // Library of Real Time Clock (RTC)
#include <TimerOne.h> // Timer1 library
#include <Time.h> // Library of Time
#include "pitches.h" // Library to use with crowtail speaker
#include <TM1637Display.h> // Library for 4-digit display- Display TM1637 (I2C)

#include <UltrasonicSensor.h>


#define DS3231_I2C_ADDRESS 0x68       // This is the I2C address (RTC)


UltrasonicSensor ultrasonic(6, 5);

//joystick pins:
#define joyX A0 // X axis of the Joystick connected to Analog pin
#define joyY A1 // Y axis of the Joystick connected to Analog pin
#define joyButton 4 // The switch button output of the Joystick is connected to digital Pin 4
int xVal, yVal;
int buttonState = 0; // The beginning value of buttonstate of joystick
int buttonState1 = 0; // The initial value of buttonstate1 of joystick


/*
// define variables
int lcd_key = 0;
int adc_key_in = 0;
int lastDay = 0;
int lastMonth = 0;
int lastYear = 0;
int lastHour = 0;
int lastMinute = 0;
int movementTimer = 0;
int menuOptions = 3;
int menuOption = 0;
int alarmHours = 0;
int alarmMinutes = 0;
bool alarmPM = 0;
bool alarmSet = 0;
bool backLightOn = 1;
// Fixed default day and hour settings on set date/time
bool resetClock = false;

// define constants
const int backLight = 10; 
const int pirPin = 16;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
#define beeper A1
#define shortBeep 100
#define longBeep  500

*/

int menu =0;  // menu for alarm adjustments
int hora, minuto, hh, mm, temp, umid, tpo, dezHora, uniHora, horaAlarme, minutoAlarme;
byte statusPIR, statusAlarme, statusTempo, statusGatilho;
int distance2; // ultrasonic sensor

#define DIO 2       // DIO of display module of TM1637
#define CLK 3       // CLK

#define PIR 7       // PIR connected to pin7
#define speaker 8   // crowtail speaker connected to pin8
#define BUZ 11
// notes in the melody:
int melody[] = {

  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {

  4, 8, 8, 4, 4, 4, 4, 4
};

TM1637Display display(CLK, DIO);  // beginning of display

// Definition of special characters to be used with the display

const uint8_t Grau[] = {
  SEG_A | SEG_B | SEG_F | SEG_G ,                   // 
};
const uint8_t UR[] = {
  SEG_C | SEG_D | SEG_E | SEG_G ,                   // %
};
const uint8_t Celsius[] = {
  SEG_A | SEG_D | SEG_E | SEG_F ,                   // C
};
const uint8_t Fahrenheit[] = {
  SEG_A | SEG_E | SEG_F | SEG_G ,                   // F
};
const uint8_t letraA[] = {
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,    // A
};
const uint8_t letraL[] = {
  SEG_D | SEG_E | SEG_F ,                           // L
};
const uint8_t letraT[] = {
  SEG_D | SEG_E | SEG_F | SEG_G,                    // t
};
const uint8_t letraI[] = {
  SEG_E,                                            // i
};
const uint8_t letraS[] = {
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,            // S
};
const uint8_t letraO[] = {
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,    // O
};
const uint8_t letraE[] = {
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,            // E
};
const uint8_t letraN[] = {
  SEG_C | SEG_E | SEG_G,                            // n
};
const uint8_t letraF[] = {
  SEG_A | SEG_E | SEG_F | SEG_G ,                   // F
};

// To be shown with colon in the display:
uint8_t dataWithColon[] = {
  0b10111111,
  0b10000110,
  0b11011011,
  0b11001111,
  0b11100110,
  0b11101101,
  0b11111101,
  0b10000111,
  0b11111111,
  0b11101111,
  0b00000000,
};

  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // Ultrasonic:
  int temperature = 23;
  ultrasonic.setTemperature(temperature);

  //RTC:
  setSyncProvider(RTC.get);   // Updating time with RTC data
  setSyncInterval(250);       // interval of seconds btw re-sync

  //TM1637:
  display.setBrightness (0x0a);
  // max brightness of the display is 0x0f

  //joystick:
  pinMode(joyButton,INPUT); // Pin 2 on which the switch of the joystick is, is defined as input
  digitalWrite(joyButton, HIGH); // Initial value of joystick button is High

        
  pinMode(speaker, OUTPUT);
  pinMode(BUZ, OUTPUT);
  pinMode(PIR, INPUT);  
  Wire.begin();

  //TimerOne initialization
  Timer1.initialize(1000);                                  // Set Timer1 for 1000 microseconds
  Timer1.attachInterrupt(joystick);                        // Timer1 reads the joystick

  /*
   * 
   
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    //RTC.adjust(DateTime(__DATE__, __TIME__));
  }
   * 
   */

  // Read data of Alarm recorded in RTC
  Wire.beginTransmission(DS3231_I2C_ADDRESS);               // Open I2C line in write mode
  Wire.write((byte)0x08);                                   // Set the register pointer to (0x08)
  Wire.endTransmission();                                   // End Write Transmission
  Wire.requestFrom(DS3231_I2C_ADDRESS, 3);                  // In this case read only 3 bytes
  horaAlarme = Wire.read();                                 // Read the hour of Alarm stored at RTC
  minutoAlarme = Wire.read();                               // Read the minute of Alarm stored at RTC
  statusAlarme = Wire.read();                               // Read the status of Alarm stored at RTC

  // Initial values of variables
  hh = 23;
  mm = 59;
  statusGatilho = LOW;
  statusTempo = LOW;

}

void buzzer() {               // Routine for Alarm/Buzzer

  display.showNumberDec(dezHora, true, 1, 0);
  display.setSegments(dataWithColon + uniHora, 1, 1);
  display.showNumberDec(minuto, true, 2, 2);

  
  tone(BUZ, 880, 300);
  delay(300);
  tone(BUZ, 523, 200);
  delay(200);
  

  // iterate over the notes of the melody:

  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.

    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.

    int noteDuration = 1000 / noteDurations[thisNote];

    tone(speaker, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.

    // the note's duration + 30% seems to work well:

    int pauseBetweenNotes = noteDuration * 1.30;

    delay(pauseBetweenNotes);

    // stop the tone playing:

    noTone(speaker);

  }

  display.setSegments(dataWithColon + 10, 1, 0);            // Clear digit #1
  display.setSegments(dataWithColon + 10, 1, 1);            // Clear digit #2
  display.setSegments(dataWithColon + 10, 1, 2);            // Clear digit #3
  display.setSegments(dataWithColon + 10, 1, 3);            // Clear digit #4
  delay(100);
  
}

void joystick() {

  // press on pushbutton switch of the joystick:
  buttonState = digitalRead(joyButton); // Reading the value of joystick button on pin 2
  // If it is pressed display will show set
  if (buttonState == LOW)
  {
    menu=menu+1;
    if (menu <= 6) {  
      statusPIR = LOW;
      display.setSegments(letraS, 1, 0);                    // Show the symbol of word S
      display.setSegments(letraE, 1, 1);                    // Show the symbol of word E
      display.setSegments(letraT, 1, 2);                    // Show the symbol of word T
      display.setSegments(dataWithColon + 10, 1, 3);        // Clear digit #4
      delay(500);
    }
    else{
      menu = 0;
      statusPIR = HIGH;
    }
  }

  switch (menu) {
    
    case 0: //Display the time

      DisplayDateTime(); // void DisplayDateTime
      // Alarm(); // Alarm control

    case 1: //Set the Hours of Alarm

      DisplaySetHour();
      
    case 2: //Set the Minutes of Alarm

      DisplaySetMinute();

    case 3: //Set status of Alarm (On-Off)

      StatusAlarm();    

    case 4: //Set the Hour of Time

      SetHourTime();

    case 5: //Set the Minutes of Time

      SetMinuteTime();

    case 6: //Set Time Adjustment

      SetTimeAdjust();
      menu=0; //??
  }
  
}

void DisplayDateTime ()
{
  // We show the hour and minutes current time
  // when menu is 0; button not pressed
  //Display Hour and Minutes
  dezHora = hora / 10;
  uniHora = hora % 10;
  display.showNumberDec(dezHora, true, 1, 0);
  display.setSegments(dataWithColon + uniHora, 1, 1);
  display.showNumberDec(minuto, true, 2, 2);
  delay(2000);

  /*
  DateTime now = RTC.now();  
  if (now.hour()<=9)
  {
    lcd.print("0");
  }
  lcd.print(now.hour(), DEC);
  hourupg=now.hour();
  lcd.print(":");
  if (now.minute()<=9)
  {
    lcd.print("0");
  }
  lcd.print(now.minute(), DEC);
  minupg=now.minute();
  lcd.print(":");
  if (now.second()<=9)
  {
    lcd.print("0");
  }
  lcd.print(now.second(), DEC);

  
  lcd.setCursor(0, 1);
  lcd.print("Date : ");
  if (now.day()<=9)
  {
    lcd.print("0");
  }
  lcd.print(now.day(), DEC);
  dayupg=now.day();
  lcd.print("/");
  if (now.month()<=9)
  {
    lcd.print("0");
  }
  lcd.print(now.month(), DEC);
  monthupg=now.month();
  lcd.print("/");
  lcd.print(now.year(), DEC);
  yearupg=now.year();
 
  char DOW[][10]={"Sunday   ","Monday   ","Tuesday  ","Wednesday","Thursday ","Friday   ","Saturday "};
  lcd.setCursor(0, 0); 
  lcd.print("Day  : ");
  lcd.print(DOW[now.dayOfTheWeek()]); // if it appears error in the code, enter the code given below
  //lcd.print(DOW[now.dayOfWeek()]);
  */
  
}

void DisplaySetHour() {
  
  // joystick:
  xVal = analogRead(joyX); // Reading x value of joy X pin-A0
  yVal = analogRead(joyY); // Reading y value of joy Y pin-A1

  // joystick movement in right +x direction:
  if (xVal>=950)
  {
    horaAlarme++;
  }
  else if(xVal<=120){
  // joystick movement in left -x direction
    horaAlarme--;
  }

  horaAlarme = constrain(horaAlarme, 0, 23);
  display.showNumberDec(horaAlarme, true, 2, 0);
  display.setSegments(letraA, 1, 2);                    // Show the symbol of word A
  display.setSegments(letraL, 1, 3);                  // Show the symbol of word L

}

void DisplaySetMinute() {
  
  // joystick:
  xVal = analogRead(joyX); // Reading x value of joy X pin-A0
  yVal = analogRead(joyY); // Reading y value of joy Y pin-A1

  // joystick movement in right +x direction:
  if (xVal>=950)
  {
    minutoAlarme++;
  }
  else if(xVal<=120){
  // joystick movement in left -x direction
    minutoAlarme--;
  }

  minutoAlarme = constrain(minutoAlarme, 0, 59);
  display.setSegments(letraA, 1, 0);                    // Show the symbol of word A
  display.setSegments(letraL, 1, 1);                    // Show the symbol of word L
  display.showNumberDec(minutoAlarme, true, 2, 2);
}

void StatusAlarm() {
  
  // joystick:
  xVal = analogRead(joyX); // Reading x value of joy X pin-A0
  yVal = analogRead(joyY); // Reading y value of joy Y pin-A1

  // joystick movement in up -y direction:
  if (yVal<=120)
  {
    statusAlarme = HIGH;
  }
  else if(yVal>=950){
  // joystick movement in down +y direction:
    statusAlarme = LOW;
  }

  if (statusAlarme == LOW) {
    display.setSegments(letraO, 1, 0);                // Show the symbol of word O
    display.setSegments(letraF, 1, 1);                // Show the symbol of word F
    display.setSegments(letraF, 1, 2);                // Show the symbol of word F
    display.setSegments(dataWithColon + 10, 1, 3);    // Clear digit #4
  }
  else if (statusAlarme == HIGH) {
    //display.showNumberDec(1111, true, 4, 0);
    display.setSegments(letraO, 1, 0);                // Show the symbol of word O
    display.setSegments(letraN, 1, 1);                // Show the symbol of word N
    display.setSegments(dataWithColon + 10, 1, 2);    // Clear digit #3
    display.setSegments(dataWithColon + 10, 1, 3);    // Clear digit #4
  }
  statusGatilho = HIGH;
}

void SetHourTime() {       
  // joystick:
  xVal = analogRead(joyX); // Reading x value of joy X pin-A0
  yVal = analogRead(joyY); // Reading y value of joy Y pin-A1

  // joystick movement in right +x direction:
  if (xVal>=950)
  {
    hh++;
  }
  else if(xVal<=120){
  // joystick movement in left -x direction
    hh--;
  }

  hh = constrain(hh, 0, 23);
  display.showNumberDec(hh, true, 2, 0);
  display.setSegments(letraT, 1, 2);               // Show the symbol of word T
  display.setSegments(letraI, 1, 3);               // Show the symbol of word I
}

void SetMinuteTime() {    
  // joystick:
  xVal = analogRead(joyX); // Reading x value of joy X pin-A0
  yVal = analogRead(joyY); // Reading y value of joy Y pin-A1

  // joystick movement in right +x direction:
  if (xVal>=950)
  {
    mm++;
  }
  else if(xVal<=120){
  // joystick movement in left -x direction
    mm--;
  }

  mm = constrain(mm, 0, 59);
  display.setSegments(letraT, 1, 0);
  display.setSegments(letraI, 1, 1);                    // Show the symbol of word T
  display.showNumberDec(mm, true, 2, 2);                // Show the symbol of word I
}

void SetTimeAdjust() {
 
  // joystick:
  xVal = analogRead(joyX); // Reading x value of joy X pin-A0
  yVal = analogRead(joyY); // Reading y value of joy Y pin-A1

  // joystick movement in up -y direction:
  if (yVal<=120)
  {
    statusTempo = HIGH;
  }
  else if(yVal>=950){
  // joystick movement in down +y direction:
    statusTempo = LOW;
  }

  if (statusTempo == LOW) {
    display.setSegments(letraO, 1, 0);                // Show the symbol of word O
    display.setSegments(letraF, 1, 1);                // Show the symbol of word F
    display.setSegments(letraF, 1, 2);                // Show the symbol of word F
    display.setSegments(dataWithColon + 10, 1, 3);    // Clear digit #4
  }
  else if (statusTempo == HIGH) {
    //display.showNumberDec(1111, true, 4, 0);
    display.setSegments(letraO, 1, 0);                // Show the symbol of word O
    display.setSegments(letraN, 1, 1);                // Show the symbol of word N
    display.setSegments(dataWithColon + 10, 1, 2);    // Clear digit #3
    display.setSegments(dataWithColon + 10, 1, 3);    // Clear digit #4
  }
}


void loop() {
  // Ultrosonic:
  int distance = ultrasonic.distanceInCentimeters();
  Serial.print("Distance to person: ");
  Serial.print(distance);
  Serial.print(" cm");

  //Clear all digits of display
  display.setSegments(dataWithColon + 10, 1, 0);            // Clear digit #1
  display.setSegments(dataWithColon + 10, 1, 1);            // Clear digit #2
  display.setSegments(dataWithColon + 10, 1, 2);            // Clear digit #3
  display.setSegments(dataWithColon + 10, 1, 3);            // Clear digit #4

  //joystick(); 
   
  do {                                                      // Keep the display off and check the sensor PIR and the time of Alarm
    if (menu == 0) {
      statusPIR = digitalRead(PIR);
      // When the PIR sensor detects a movement, 
      // the output will be HIGH, otherwise, it will be LOW.
    }
    hora = hour();
    minuto = minute();
    if (hora == horaAlarme && minuto == minutoAlarme && statusAlarme == HIGH) {
      // If it is alarm time and alarm status is on:
      // ring the alarm:
      Serial.println("Speaker running!!!!");
      buzzer();
      delay(50); //for ultrasonic sensor to stabilize;
      distance2 = ultrasonic.distanceInCentimeters();
    }
  } while (statusPIR == LOW || distance2 == distance);
  // if there is a movement or a change in the distance,
  // then the alarm will stop. 
  // if there is a movement but no change im distance,
  // then the alarm will continue

  /*
   * 
   ????????*
  hora = hour();
  minuto = minute();
  if (hora == horaAlarme && minuto == minutoAlarme && statusAlarme == HIGH) {
    buzzer();
  }
  
  * 
  */

  //Set Time (Hours and Minutes)
  if (menu == 0 && statusTempo == HIGH) {
    setTime(hh, mm, 00, day(), month(), year());
    RTC.set(now());     // the current time
    setSyncProvider(RTC.get);
    statusTempo = LOW;
  }

  // Write data of Alarm into RTC
  if (menu == 0 && statusGatilho == HIGH) {
    Wire.beginTransmission(DS3231_I2C_ADDRESS);               // Open I2C line in write mode
    Wire.write((byte)0x08);                                   // Set the register pointer to (0x08)
    Wire.write(horaAlarme);                                   // Record at RTC memory the Hour of Alarm
    Wire.write(minutoAlarme);                                 // Record at RTC memory the Minute of Alarm
    Wire.write(statusAlarme);                                 // Record at RTC memory the Status of Alarm
    Wire.endTransmission();                                   // End Write Transmission
    statusGatilho == LOW;
  }

  if (menu == 0) {
    //Display Hour and Minutes
    dezHora = hora / 10;
    uniHora = hora % 10;
    display.showNumberDec(dezHora, true, 1, 0);
    display.setSegments(dataWithColon + uniHora, 1, 1);
    display.showNumberDec(minuto, true, 2, 2);
    delay(2000);
  }

  /*
  // joystick:
  xVal = analogRead(joyX); // Reading x value of joy X pin-A0
  yVal = analogRead(joyY); // Reading y value of joy Y pin-A1
  Serial.print("xVal: "); // Printing the values read from joystick
  Serial.print(xVal); // Printing the values read from joystick
  Serial.print("\n");
  Serial.print("yVal: ");
  Serial.print(yVal);
  Serial.print("\n");
  buttonState = digitalRead(joyButton); // Reading the value of joystick button on pin 2
  Serial.print("buttonState: ");
  Serial.print(buttonState);
  Serial.print("\n");
  */
 
}
