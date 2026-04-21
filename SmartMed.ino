#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

#define MAX_ALARMS 3

// PINS
int buzzer = 8;
int led = 2;
int btnMode = 7;
int btnAction = 9;

// ALARM DATA
int med_hour[MAX_ALARMS] = {8, 14, 20};
int med_min[MAX_ALARMS]  = {0, 0, 0};
int med_day[MAX_ALARMS]  = {1, 1, 1};

// DAY NAMES
const char* days[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

// STATES
bool alarmActive = false;
bool alarmTriggered[MAX_ALARMS] = {false, false, false};
bool settingMode = false;

int currentAlarm = 0;
int settingField = 0;

// BUTTON
unsigned long pressStart = 0;
bool longPressHandled = false;

void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(btnMode, INPUT_PULLUP);
  pinMode(btnAction, INPUT_PULLUP);

  rtc.begin();
}

void loop() {
  DateTime now = rtc.now();

  handleButtons();
  handleAlarm(now);
  display(now);

  delay(200);
}

// ---------------- BUTTON LOGIC ----------------
void handleButtons() {
  if (digitalRead(btnMode) == LOW) {
    if (pressStart == 0) pressStart = millis();

    if (millis() - pressStart > 800 && !longPressHandled) {
      longPressHandled = true;
      settingMode = !settingMode;
    }
  } else {
    if (pressStart != 0 && !longPressHandled) {
      if (settingMode) {
        settingField++;
        if (settingField > 2) {
          settingField = 0;
          currentAlarm = (currentAlarm + 1) % MAX_ALARMS;
        }
      }
    }
    pressStart = 0;
    longPressHandled = false;
  }

  if (digitalRead(btnAction) == LOW) {
    if (settingMode) {
      if (settingField == 0)
        med_hour[currentAlarm] = (med_hour[currentAlarm] + 1) % 24;

      else if (settingField == 1)
        med_min[currentAlarm] = (med_min[currentAlarm] + 5) % 60;

      else if (settingField == 2)
        med_day[currentAlarm] = (med_day[currentAlarm] + 1) % 7;

      delay(200);
    }
    else if (alarmActive) {
      alarmActive = false;
      digitalWrite(buzzer, LOW);
      digitalWrite(led, LOW);

      lcd.setCursor(0,1);
      lcd.print("                ");
      delay(300);
    }
  }
}

// ---------------- ALARM LOGIC ----------------
void handleAlarm(DateTime now) {
  int today = now.dayOfTheWeek();

  for (int i = 0; i < MAX_ALARMS; i++) {
    if (!settingMode) {
      if (now.hour() == med_hour[i] &&
          now.minute() == med_min[i] &&
          today == med_day[i] &&
          !alarmTriggered[i]) {

        alarmActive = true;
        alarmTriggered[i] = true;
      }

      if (now.minute() != med_min[i]) {
        alarmTriggered[i] = false;
      }
    }
  }

  if (alarmActive) {
    digitalWrite(buzzer, HIGH);
    digitalWrite(led, HIGH);
  }
}

// ---------------- DISPLAY ----------------
void display(DateTime now) {
  lcd.setCursor(0,0);

  if (!settingMode) {
    lcd.print("Time: ");
    print2(now.hour());
    lcd.print(":");
    print2(now.minute());
    lcd.print(":");
    print2(now.second());

    lcd.setCursor(0,1);

    if (alarmActive) {
      lcd.print("TAKE MEDICINE!");
    } else {
      lcd.print("A");
      lcd.print(currentAlarm+1);
      lcd.print(" ");

      print2(med_hour[currentAlarm]);
      lcd.print(":");
      print2(med_min[currentAlarm]);

      lcd.print(" ");
      lcd.print(days[med_day[currentAlarm]]);
      lcd.print(" ");
    }
  } 
  else {
    lcd.print("Set A");
    lcd.print(currentAlarm+1);
    lcd.print("      ");

    lcd.setCursor(0,1);

    if (settingField == 0) lcd.print("H:");
    else lcd.print("  ");

    print2(med_hour[currentAlarm]);
    lcd.print(":");

    if (settingField == 1) lcd.print("M:");
    else lcd.print("  ");

    print2(med_min[currentAlarm]);

    lcd.print(" ");

    if (settingField == 2) lcd.print("*");
    else lcd.print(" ");

    lcd.print(days[med_day[currentAlarm]]);
    lcd.print(" ");
  }
}

// ---------------- HELPER ----------------
void print2(int val) {
  if (val < 10) lcd.print("0");
  lcd.print(val);
}
