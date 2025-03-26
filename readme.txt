#include "Config.h"
#include "Firebase.h"

int refreshCount = 0;
void setup() {
  Serial.begin(115200);
  Serial.println("OK");
  initPins();
  initFirebase();
  digitalWrite(2, HIGH);
  read_data();

  /*
    // Following line sets the RTC with an explicit date & time once
    rtc.set(0, 45, 12, 6, 11, 5, 24);
  */
  delay(5000);
}

uint32_t pmillis = 0, previousMillis = 0;
bool swapScreen = false;
void loop() {
  if (Mode == "0") {
    scanButtons();
  }

  if (PzemDisplay() && millis() - pmillis > 3000) {
    pmillis = millis();
    read_data();
    scanAmps();
    Power1 = Amps1 * voltage;
    Power2 = Amps2 * voltage;
    Power3 = Amps3 * voltage;
    lcdUpdate();

    rtc.refresh();
    Serial.println(String(rtc.hour()));
    //    if (String(rtc.hour()) == "13") {
    //      Serial.println("Success");
    //    }
  } else {
    digitalWrite(2, LOW);
  }


  // if WiFi is down, try reconnecting
  if ((WiFi.status() != WL_CONNECTED) && (millis() - previousMillis >= 10000)) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    Serial.println("Reconnected");
    previousMillis = millis();
  }
}

  void lcdUpdate() {
    if (swapScreen) {
      lcdUpdate2();
      swapScreen = false;
      refreshCount++;
    } else {
      if (refreshCount > 5) {  // due to initializing time of ACS712 sensors
        lcdUpdate1();
      }
      swapScreen = true;
    }
  }

void scanButtons() {
  if (digitalRead(Button1) == 0) {
    digitalWrite(SSR1, HIGH);
  } else {
    digitalWrite(SSR1, LOW);
  }
  if (digitalRead(Button2) == 0) {
    digitalWrite(SSR2, HIGH);
  } else {
    digitalWrite(SSR2, LOW);
  }
  if (digitalRead(Button3) == 0) {
    digitalWrite(SSR3, HIGH);
  } else {
    digitalWrite(SSR3, LOW);
  }
}




#include <Arduino.h>
#include <PZEM004Tv30.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "uRTCLib.h"
#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance
EnergyMonitor emon2;                   // Create an instance
EnergyMonitor emon3;                   // Create an instance

// uRTCLib rtc;
uRTCLib rtc(0x68);

#define AmpSensor1 36
#define AmpSensor2 39 //VN
#define AmpSensor3 34 //VP
#define SSR1 27
#define SSR2 25
#define SSR3 32
#define Button1 13
#define Button2 14
#define Button3 33

#define RXD2 16 // to TX of Pzem module
#define TXD2 17 // to RX of Pzem module
PZEM004Tv30 pzem(Serial2, RXD2, TXD2);
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

String Mode = "0"; // 0 -> Manual, 1 -> Auto
String state1 = "OFF", state2 = "OFF", state3 = "OFF";
String load1 = "OFF", load2 = "OFF", load3 = "OFF";
// Parameters
float PowerT = 0.0, Power1 = 0.0, Power2 = 0.0, Power3 = 0.0;
float AmpsT = 0.0, Amps1 = 0.0, Amps2 = 0.0, Amps3 = 0.0;
float voltage = 0.0, pf = 0.0, freq = 0.0, units = 0.0;
float unitsSet = 0.0;
String time1start, time1end; // load 1 in hours only
String time2start, time2end; // load 2 in hours only
String time3start, time3end; // load 3 in hours only

void initPins() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("ECC Controls");
  lcd.setCursor(0, 1);
  lcd.print("And Automation");

  pinMode(Button1, INPUT_PULLUP);
  pinMode(Button2, INPUT_PULLUP);
  pinMode(Button3, INPUT_PULLUP);

  pinMode(SSR1, OUTPUT);
  digitalWrite(SSR1, LOW);
  pinMode(SSR2, OUTPUT);
  digitalWrite(SSR2, LOW);
  pinMode(SSR3, OUTPUT);
  digitalWrite(SSR3, LOW);

  URTCLIB_WIRE.begin();
  emon1.current(AmpSensor1, 3.15);             // Current: input pin, calibration.
  emon2.current(AmpSensor2, 3.15);             // Current: input pin, calibration.
  emon3.current(AmpSensor3, 3.15);             // Current: input pin, calibration.

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  delay(1000);
}

bool PzemDisplay() {
  bool success = true;

  // Read the data from the sensor
  voltage = pzem.voltage();
  AmpsT = pzem.current();
  PowerT = pzem.power();
  units = pzem.energy();
  freq = pzem.frequency();
  pf = pzem.pf();

  // Check if the data is valid
  if (isnan(voltage)) {
    success = false;
  } 
  return success;
}

void scanAmps() {
  Amps1 = emon1.calcIrms(1480);  // Calculate Irms only
  Amps2 = emon2.calcIrms(1480);  // Calculate Irms only
  Amps3 = emon3.calcIrms(1480);  // Calculate Irms only
  if (Amps1 <= 0.2) {
    Amps1 = 0.0;
  }
  if (Amps2 <= 0.2) {
    Amps2 = 0.0;
  }
  if (Amps3 <= 0.2) {
    Amps3 = 0.0;
  }
}

void lcdUpdate2() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Voltage = ");
  lcd.print(voltage, 1);
  lcd.print(" V");
  lcd.setCursor(0, 1);
  lcd.print("Current = ");
  lcd.print(AmpsT, 2);
  lcd.print(" A");
  lcd.setCursor(0, 2);
  lcd.print("Power = ");
  lcd.print(PowerT, 2);
  lcd.print(" W");
  lcd.setCursor(0, 3);
  lcd.print(" Units = ");
  lcd.print(units, 2);
  lcd.print(" KWh");
}

void lcdUpdate1() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("I= ");
  lcd.print(Amps1, 2);
  lcd.print("A  ");
  lcd.print("P=");
  lcd.print(Power1, 1);
  lcd.print("W");

  lcd.setCursor(0, 1);
  lcd.print("I= ");
  lcd.print(Amps2, 2);
  lcd.print("A  ");
  lcd.print("P=");
  lcd.print(Power2, 1);
  lcd.print("W");

  lcd.setCursor(0, 2);
  lcd.print("I= ");
  lcd.print(Amps3, 2);
  lcd.print("A  ");
  lcd.print("P=");
  lcd.print(Power3, 1);
  lcd.print("W");

  lcd.setCursor(0, 3);
  lcd.print("PF= ");
  lcd.print(pf, 2);
  lcd.print("   Freq= ");
  lcd.print(freq, 1);
  lcd.print(" Hz");
}
