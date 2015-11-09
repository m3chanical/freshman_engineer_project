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

#define GaugePin A5


Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
RTC_DS1307 rtc;

// Strain Gauge/Inst Amp input pin:


unsigned long lastInput = 0; // Last button press, for timeout.

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// States for the state machine!
enum OperatingStates { 
        DISPLAY_TIME = 0, 
        SET_TIME, 
        SET_ALARM 
      }; 
OperatingStates opState = DISPLAY_TIME;

// Function prototypes, because I'm a C programmer:
void AlarmState(void);
void DisplayTime(void);
void SetTime(void); // on set time, display current time as well.
void SetAlarm(void);
float ReadGauge(void);
uint8_t ReadButtons(void);



void setup () {


  Serial.begin(57600); // for debugging
  lcd.begin(16, 2);
  lcd.print("Hello, World!");
  if (! rtc.begin()) {
    lcd.print("Couldn't find RTC");
    while (1);
  }
  
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}

void loop () {
    while(ReadButtons() != 0) {} // wait for button release before changing states
    lcd.clear();
    
    float sensorVoltage = ReadGauge();

    switch(opState) {
      case DISPLAY_TIME:
        break;
        
      case SET_TIME:
        break;
        
      case SET_ALARM:
        break;
    }
    
}

void AlarmState() {

}
void DisplayTime() {

}
void SetTime() { // on set time, display current time as well.
  //Get Current time, enable update of minute... hold button for advance of 10min (?)

}
void SetAlarm() {

}

float ReadGauge() {
    int sensorValue = analogRead(GaugePin);
    Serial.print("The sensor value is: ");
    Serial.println(sensorValue);
    float sensorVolts = sensorValue * (2.4 / 1023.0);
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
