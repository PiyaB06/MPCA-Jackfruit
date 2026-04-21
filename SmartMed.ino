#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

// PINS
int buzzer = 8;
int led = 2;
int btnMode = 7;   // D7
int btnAction = 9; // D9

// MED TIME
int med_hour = 0;
int med_min = 00;

// STATES
bool alarmActive = false;
bool alarmTriggered = false;
bool settingMode = false;
bool settingHour = true;

// BUTTON TIMING
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

  handleButtons(now);
  handleAlarm(now);
  display(now);

  delay(200);
}

void handleButtons(DateTime now) {
  // LONG PRESS DETECTION (D7)
  if (digitalRead(btnMode) == LOW) {
    if (pressStart == 0) pressStart = millis();

    if (millis() - pressStart > 800 && !longPressHandled) {
      longPressHandled = true;

      if (!settingMode) {
        settingMode = true;   // ENTER setting
      } else {
        settingMode = false;  // EXIT setting
      }
    }
  } else {
    // Button released
    if (pressStart != 0 && !longPressHandled) {
      // SHORT PRESS → switch field
      if (settingMode) {
        settingHour = !settingHour;
      }
    }

    pressStart = 0;
    longPressHandled = false;
  }

  // D9 BUTTON
  if (digitalRead(btnAction) == LOW) {
    if (settingMode) {
      // INCREMENT TIME
      if (settingHour) {
        med_hour = (med_hour + 1) % 24;
      } else {
        med_min = (med_min + 5) % 60;
      }
      delay(200);
    } else if (alarmActive) {
      // STOP ALARM
      alarmActive = false;
      digitalWrite(buzzer, LOW);
      digitalWrite(led, LOW);

      lcd.setCursor(0, 1);
      lcd.print("                ");
      delay(300);
    }
  }
}

void handleAlarm(DateTime now) {
  if (!settingMode) {
    // TRIGGER ONCE
    if (now.hour() == med_hour && now.minute() == med_min && !alarmTriggered) {
      alarmActive = true;
      alarmTriggered = true;
    }

    if (now.minute() != med_min) {
      alarmTriggered = false;
    }
  }

  if (alarmActive) {
    digitalWrite(buzzer, HIGH);
    digitalWrite(led, HIGH);
  }
}

void display(DateTime now) {
  lcd.setCursor(0, 0);

  if (!settingMode) {
    lcd.print("Time: ");
    print2(now.hour());
    lcd.print(":");
    print2(now.minute());
    lcd.print(":");
    print2(now.second());

    lcd.setCursor(0, 1);

    if (alarmActive) {
      lcd.print("TAKE MEDICINE! ");
    } else {
      lcd.print("Next: ");
      print2(med_hour);
      lcd.print(":");
      print2(med_min);
      lcd.print("       ");
    }
  } else {
    lcd.print("Set Time:");

    lcd.setCursor(0, 1);

    if (settingHour) lcd.print("H:");
    else lcd.print(" ");

    print2(med_hour);
    lcd.print(":");

    if (!settingHour) lcd.print("M:");
    else lcd.print(" ");

    print2(med_min);
    lcd.print("   ");
  }
}

void print2(int val) {
  if (val < 10) lcd.print("0");
  lcd.print(val);
}