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
 
int length = 15; // the number of notes
char notes[] = "ccggaagffeeddc "; // a space represents a rest
int beats[] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 4 };
int tempo = 300; 

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
  alarmTime = rtc.now();
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
  
  if(!alarming){
    alarming = CheckAlarm(); // Checks the current time (hours/minutes) against the alarm time, sets alarming to true if they're equivalent.
  }
  
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
  

  
  if( alarming ) {
    MakeNoise();
  }

  delay(250);
  ReadGauge();  
}
 
void MainDisplay(){
  uint8_t buttons = ReadButtons();
  DisplayTime(0, 0, now);
  lcd.setBacklight(TEAL);
  lcd.setCursor(8, 1);
  lcd.print("Alarm: ");
  if(buttons & BUTTON_UP){
    alarmOn = !alarmOn;
  }
  if(alarmOn){
    lcd.print("Y");
  } else {
    lcd.print("N");
    alarming = false;
  }
  
  if(buttons & BUTTON_RIGHT){
    acState = SET_TIME;
    lcd.clear();
  }
  if(buttons & BUTTON_LEFT){
    acState = SET_ALARM;
    lcd.clear();
  }
  if(buttons & BUTTON_SELECT) {
    for (int i = 0; i < length; i++) {
    if (notes[i] == ' ') {
      delay(beats[i] * tempo); // rest
    } else {
      playNote(notes[i], beats[i] * tempo);
    }
    
    // pause between notes
    delay(tempo / 2); 
    }
  }
}

void SetTime() {
  
  boolean bEditHours = 1; // 0 if editing minutes
  boolean canChangeTime = true; 
  unsigned long lastTimeChange;
  DateTime newTime = rtc.now();
  lcd.setBacklight(VIOLET);


  uint8_t buttons = 0;
  while(true){
    buttons = ReadButtons();
    lcd.setCursor(0, 0);
    lcd.print("Cur: ");
    DisplayTime(5, 0, rtc.now());
    lcd.setCursor(0, 1);
    lcd.print("New: ");
    DisplayTime(5, 1, newTime);
    
    if (millis() - lastTimeChange > 250){
     canChangeTime = true;
    }
     
    if(buttons & BUTTON_SELECT){
      bEditHours = !bEditHours;
    }
     
    if(buttons & BUTTON_UP){
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
     }
     if(buttons & BUTTON_DOWN){
       rtc.adjust(newTime);
       acState = MAIN_DISPLAY;
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("NEW TIME SET");
       delay(500);
       lcd.clear();
       return;
     }
     if(buttons & BUTTON_RIGHT){
       acState = SET_ALARM;
       lcd.clear();
       return;
     }
     if(buttons & BUTTON_LEFT){
       acState = MAIN_DISPLAY;
       lcd.clear();
       return;
     }
     if( (millis() - lastInput) > 3000){
       acState = MAIN_DISPLAY;
       lcd.clear();
       return;
     }
  }
}

void SetAlarm(){
  boolean bEditHours = 1; // 0 if editing minutes
  boolean canChangeTime = true; 
  unsigned long lastTimeChange;
  DateTime newAlarm = alarmTime;
  lcd.setBacklight(BLUE);
  uint8_t buttons = ReadButtons();
  
  while(true){
    buttons = ReadButtons();
    lcd.setCursor(0, 0);
    lcd.print("Alarm: ");
    DisplayTime(7, 0, alarmTime);
    lcd.setCursor(0, 1);
    lcd.print("New: ");
    DisplayTime(5, 1, newAlarm);
    
    if (millis() - lastTimeChange > 250){
     canChangeTime = true;
    }
     
    if(buttons & BUTTON_SELECT){
      bEditHours = !bEditHours;
    }
     
    if(buttons & BUTTON_UP){
      if ( bEditHours ) {
        if(canChangeTime){
          newAlarm = newAlarm + 3600; // operator+ adds seconds to DateTime object
          canChangeTime = false; 
          lastTimeChange = millis();
        }
      } else {
        if(canChangeTime){
          newAlarm = newAlarm + 60;
          canChangeTime = false;
          lastTimeChange = millis();
        }
      }
     }
     if(buttons & BUTTON_DOWN){
       alarmTime = newAlarm;
       acState = MAIN_DISPLAY;
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("NEW ALARM SET");
       delay(500);
       lcd.clear();
       return;
     }
     if( (millis() - lastInput) > 3000 ){
        acState = MAIN_DISPLAY;
        lcd.clear();
        return;
     }
     if(buttons & BUTTON_RIGHT){
        acState = MAIN_DISPLAY;
        lcd.clear();
        return;
     }
     if(buttons & BUTTON_LEFT){
        acState = SET_TIME;
        lcd.clear();
        return;
     }
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



void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(SpeakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(SpeakerPin, LOW);
    delayMicroseconds(tone);
  }
}

void playNote(char note, int duration) {
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };
  
  // play the tone corresponding to the note name
  for (int i = 0; i < 8; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}

