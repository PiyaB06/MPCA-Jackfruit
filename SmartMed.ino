#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

#define DAYS 7
#define ALARMS_PER_DAY 3

//pins
int buzzer = 8; //D8
int led = 2;
int btnMode = 7; //D7
int btnAction = 9; //D9

//data
int med_hour[DAYS][ALARMS_PER_DAY] = {0};
int med_min[DAYS][ALARMS_PER_DAY] = {0};

const char* days[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

// states
bool alarmActive = false;
bool alarmTriggered[DAYS][ALARMS_PER_DAY] = {false};
bool settingMode = false;

// Nav
int currentDay = 0;
int currentAlarm = 0;
int settingField = 0;

// Button
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

//Buttons
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
        settingField = (settingField + 1) % 4;
      }
    }
    pressStart = 0;
    longPressHandled = false;
  }

  if (digitalRead(btnAction) == LOW) {
    if (settingMode) {
      if (settingField == 0)
        currentDay = (currentDay + 1) % 7;

      else if (settingField == 1)
        currentAlarm = (currentAlarm + 1) % ALARMS_PER_DAY;

      else if (settingField == 2)
        med_hour[currentDay][currentAlarm] =
          (med_hour[currentDay][currentAlarm] + 1) % 24;

      else if (settingField == 3)
        med_min[currentDay][currentAlarm] =
          (med_min[currentDay][currentAlarm] + 5) % 60;

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

// Alarm
void handleAlarm(DateTime now) {
  int today = now.dayOfTheWeek();

  for (int a = 0; a < ALARMS_PER_DAY; a++) {
    if (!settingMode) {
      if (now.hour() == med_hour[today][a] &&
          now.minute() == med_min[today][a] &&
          !alarmTriggered[today][a]) {

        alarmActive = true;
        alarmTriggered[today][a] = true;
      }

      if (now.minute() != med_min[today][a]) {
        alarmTriggered[today][a] = false;
      }
    }
  }

  if (alarmActive) {
    digitalWrite(buzzer, HIGH);
    digitalWrite(led, HIGH);
  }
}

// Display
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
      lcd.print(days[currentDay]);
      lcd.print(" A");
      lcd.print(currentAlarm+1);
      lcd.print(" ");

      print2(med_hour[currentDay][currentAlarm]);
      lcd.print(":");
      print2(med_min[currentDay][currentAlarm]);
    }
  }
  else {
    lcd.print("Set ");
    lcd.print(days[currentDay]);
    lcd.print(" A");
    lcd.print(currentAlarm+1);

    lcd.setCursor(0,1);

    if (settingField == 0) lcd.print("*");
    else lcd.print(" ");

    lcd.print(days[currentDay]);
    lcd.print(" ");

    if (settingField == 1) lcd.print("*");
    else lcd.print(" ");

    lcd.print("A");
    lcd.print(currentAlarm+1);

    lcd.print(" ");

    if (settingField == 2) lcd.print("*");
    else lcd.print(" ");

    print2(med_hour[currentDay][currentAlarm]);
    lcd.print(":");

    if (settingField == 3) lcd.print("*");
    else lcd.print(" ");

    print2(med_min[currentDay][currentAlarm]);
  }
}

//Helper
void print2(int val) {
  if (val < 10) lcd.print("0");
  lcd.print(val);
}
