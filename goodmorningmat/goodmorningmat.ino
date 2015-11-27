// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#include <utility/Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
 
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

#define TRIGGERVOLTS 2.0
 
#define GaugePin A0
#define SpeakerPin 9
 
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
RTC_DS1307 rtc;
 
unsigned long lastInput = 0; // Last button press, for timeout.
float sensorValue = 0;
float sensorVolts = 0;
bool alarming = false;
bool alarmOn = false;
// Methods hour() and minute() may not work if alarmTime is not initialized
DateTime alarmTime; // 0 hrs 0 minutes
DateTime now;

// States for state machine: 
enum alarmClockStates {MAIN_DISPLAY = 0, SET_TIME, SET_ALARM};
alarmClockStates acState = MAIN_DISPLAY;
 
// Function prototypes
void MainDisplay();
void SetTime(); 
void SetAlarm();
void DisplayTime(int, int, DateTime);
float ReadGauge(void);
uint8_t ReadButtons(void);
void MakeNoise(void);
bool CheckAlarm(void);
 

 
void setup () {
  
  Serial.begin(57600); // Debug info here.
  lcd.begin(16, 2);
  pinMode(SpeakerPin, OUTPUT);
  
  //lcd.print("Hello, World!");
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  if (! rtc.begin()) {
    lcd.print("Couldn't find RTC");
    while (1);
  }
 
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    
  }
  
}
 
void loop () {
  now = rtc.now();
  alarmTime = rtc.now();
  
  switch(acState) {
    case MAIN_DISPLAY:
      MainDisplay();
      break;
    case SET_TIME:
      SetTime();
      break;
    case SET_ALARM:
      SetAlarm();
      break;
  }
  
  // compare minutes and hours because operator '==' not overloaded for DateTime objects
  if (alarming){
    MakeNoise(); // this function loops until load is detected
  }

  delay(250);
  ReadGauge();  
}
 
void MainDisplay(){
  
  DisplayTime(0, 0, now);
  lcd.setBacklight(TEAL);
  lcd.setCursor(8, 1);
  lcd.print("Alarm: ");
  if(ReadButtons() & BUTTON_UP){
    alarmOn = !alarmOn;
  }
  if(alarmOn){
    lcd.print("Y");
  } else {
    lcd.print("N");
  }
  
  if(ReadButtons() & BUTTON_RIGHT){
    acState = SET_TIME;
    lcd.clear();
  }
}

void SetTime() {
  
  boolean bEditHours = 1; // 0 if editing minutes
  boolean canChangeTime = true; 
  unsigned long lastTimeChange;
  DateTime newTime = rtc.now();
  lcd.setBacklight(VIOLET);

  lcd.setCursor(0, 0);
  lcd.print("Current: ");
  DisplayTime(9, 0, now);
  lcd.setCursor(0, 1);
  lcd.print("New: ");
  DisplayTime(5, 1, newTime);
  
  if (millis() - lastTimeChange > 250){
     canChangeTime = true;
  }
  switch ( ReadButtons() ) {
  case BUTTON_SELECT: // toggle editing hours/minutes
    bEditHours = !bEditHours;
    break;
  case BUTTON_UP:
    // hard-coding date for simplicity...
    if ( bEditHours ) {
      if(canChangeTime){
        newTime = newTime + 3600; // operator+ adds seconds to DateTime object
        canChangeTime = false; 
        lastTimeChange = millis();
      }
    } else {
      if(canChangeTime){
        newTime = newTime + 60;
        canChangeTime = false;
        lastTimeChange = millis();
      }
    }
    break;
  case BUTTON_RIGHT:
    acState = SET_ALARM;
    lcd.clear();
    break;
  case BUTTON_DOWN:
    rtc.adjust(newTime);
    break;
  }
  if( (millis() - lastInput) > 3000){
    acState = MAIN_DISPLAY;
    lcd.clear();
  }

  alarming = CheckAlarm;
}

void SetAlarm(){
  
  lcd.setBacklight(BLUE);
  //do cool shit;
  if( (millis() - lastInput) > 3000 ){
    acState = MAIN_DISPLAY;
  }
  if(ReadButtons() & BUTTON_RIGHT){
    acState = MAIN_DISPLAY;
    lcd.clear();
  }
}
    
    
void DisplayTime(int column, int line, DateTime time) {
  
  lcd.setCursor(column, line);
  if(time.hour() < 10){
   lcd.print(0); 
  }
  lcd.print(time.hour(), DEC );
  lcd.print(":");
  if(time.minute() < 10){
   lcd.print(0); 
  }
  lcd.print(time.minute(), DEC );
  lcd.print(":");
  if(time.second() < 10){
   lcd.print(0); 
  }
  lcd.print(time.second(), DEC);
  
  alarming = CheckAlarm();
  
  
}
 
float ReadGauge() {
    sensorValue = analogRead(GaugePin);
    Serial.print("The sensor value is: ");
    Serial.println(sensorValue);
    sensorVolts = sensorValue * (4.76 / 1023.0);
    Serial.print("The sensor voltage is: ");
    Serial.println(sensorVolts);
   
    return sensorVolts;
}
 
uint8_t ReadButtons() {
  uint8_t buttons = lcd.readButtons();
  if (buttons != 0) {
    lastInput = millis();
  }
  return buttons;
}

bool CheckAlarm(){
  Serial.print("Alarming: ");
  Serial.println(alarmOn && (rtc.now().hour() == alarmTime.hour() && rtc.now().minute() == alarmTime.minute()));
  return alarmOn && (rtc.now().hour() == alarmTime.hour() && rtc.now().minute() == alarmTime.minute());
}
 
void MakeNoise(){
  lcd.setBacklight(RED);
  delay(100);
  lcd.setBacklight(GREEN);
  //make noise.

}

