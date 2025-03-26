#include "Config.h"
// #include "Firebase.h"

int refreshCount = 0;
void setup() {
  Serial.begin(115200);
  initPins();
  initFirebase();
  read_data();
  Serial.println("OK");
  bool success = PzemDisplay();
  lcdUpdate2();
  for (int i = 1; i <= 5; i++) {
    Amps1 = emon3.calcIrms(1480);  // Calculate Irms only
    Amps2 = emon2.calcIrms(1480);  // Calculate Irms only
    Amps3 = emon1.calcIrms(1480);  // Calculate Irms only
    delay(200);
  }

  /*
    // Following line sets the RTC with an explicit date & time once
    rtc.set(0, 45, 12, 6, 11, 5, 24);
  */
}

unsigned long pmillis = 0, previousMillis = 0;
void loop() {
  read_data();
  if (Mode == "0") {
    scanButtons();
  } else if (Mode == "1") {  // Auto Mode
    if (units >= unitsSet.toFloat()) {
      LoadAuto(0);
    } else {
      LoadAuto(1);
    }
  }

  if (millis() - pmillis >= 2000) {
    if (PzemDisplay()) {
      scanAmps();
      Power1 = Amps1 * voltage;
      Power2 = Amps2 * voltage;
      Power3 = Amps3 * voltage;
    }
    lcdUpdate();
    sentdata();
    // rtc.refresh();
    // Serial.println(String(rtc.hour()));
    pmillis = millis();
  }

  // if WiFi is down, try reconnecting
  // if ((WiFi.status() != WL_CONNECTED) && (millis() - previousMillis >= 10000)) {
  //   Serial.println("Reconnecting to WiFi...");
  //   WiFi.disconnect();
  //   WiFi.reconnect();
  //   Serial.println("Reconnected");
  //   previousMillis = millis();
  // }
}


void LoadAuto(int i) {
  if (i == 1) {
    load1 = "ON";
    load2 = "ON";
    load3 = "ON";
    digitalWrite(SSR1, HIGH);
    digitalWrite(SSR2, HIGH);
    digitalWrite(SSR3, HIGH);
  } else {
    load1 = "OFF";
    load2 = "OFF";
    load3 = "OFF";
    digitalWrite(SSR1, LOW);
    digitalWrite(SSR2, LOW);
    digitalWrite(SSR3, LOW);
  }
}

void lcdUpdate() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  if (swapScreen) {
    lcdUpdate2();
    swapScreen = false;
    refreshCount++;
  } else {
    // if (refreshCount > 1) {  // due to initializing time of ACS712 sensors
      lcdUpdate1();
    // }
    swapScreen = true;
  }
}
