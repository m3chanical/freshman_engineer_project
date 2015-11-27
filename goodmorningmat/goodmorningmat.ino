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

int length = 15; // the number of notes
char notes[] = "ccggaagffeeddc "; // a space represents a rest
int beats[] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 4 };
int tempo = 300;

// methods hour() and minute() may not work if alarmTime is not initialized
DateTime alarmTime = DateTime(2015, 1, 1, 0, 0); // 0 hrs 0 minutes
 
// Function prototypes, because I'm a C programmer:
void DisplayTime(DateTime oTime);
DateTime SetTime(DateTime oTime, bool tabToSetAlarm = 1); // used for both alarm and current time
float ReadGauge(void);
uint8_t ReadButtons(void);
void MakeNoise(void);
 

 
void setup () {
 
 
  Serial.begin(57600); // for debugging
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
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
}
 
void loop () {

    DisplayTime(rtc.now());
    
    // compare minutes and hours because operator== not overloaded for DateTime objects
    if ( rtc.now().hour() == alarmTime.hour() && rtc.now().minute() == alarmTime.minute() )
      //MakeNoise(); // this function loops until load is detected
      
    if (ReadButtons() & BUTTON_SELECT)
      rtc.adjust( SetTime(rtc.now()) );
        // SetTime() loops until BUTTON_SELECT is pressed, or timeout 
    delay(250);
    ReadGauge();  
    
    if (ReadButtons() & BUTTON_LEFT){
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
 
void DisplayTime(DateTime oTime) {
  //lcd.clear();
  lcd.setCursor(0, 0);
  if(oTime.hour() < 10){
   lcd.print(0); 
  }
  lcd.print( oTime.hour(), DEC );
  lcd.print(":");
  if(oTime.minute() < 10){
   lcd.print(0); 
  }
  lcd.print( oTime.minute(), DEC );
  lcd.print(":");
  if(oTime.second() < 10){
   lcd.print(0); 
  }
  lcd.print(oTime.second(), DEC);
}

DateTime SetTime(DateTime oTime, bool tabToSetAlarm) {
  // Tab from hours to minutes with right button

  boolean bEditHours = 1; // 0 if editing minutes
  boolean canChangeTime = true; 
  unsigned long lastTimeChange;
  while ( (millis() - lastInput) < 3000 ) { // timeout condition
    DisplayTime(oTime);
    if (millis() - lastTimeChange > 250){
       canChangeTime = true;
    }
      switch ( ReadButtons() ) {
      case BUTTON_RIGHT: // toggle editing hours/minutes
        bEditHours = !bEditHours;
        break;
        
      case BUTTON_UP:
        // hard-coding date for simplicity...
        if ( bEditHours ) {
          if(canChangeTime){
            oTime = oTime + 3600; // operator+ adds seconds to DateTime object
            canChangeTime = false; 
            lastTimeChange = millis();
          }
        } else {
          if(canChangeTime){
            oTime = oTime + 60;
            canChangeTime = false;
            lastTimeChange = millis();
          }
        }
        break;

      case BUTTON_SELECT:
        if (tabToSetAlarm)
          alarmTime = SetTime(alarmTime, 0);
        return oTime;
        break;
    }

  }
  return oTime;  
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
