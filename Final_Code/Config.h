#include <PZEM004Tv30.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "uRTCLib.h"
#include "EmonLib.h"  // Include Emon Library
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
const char* ssid = "LMS";
const char* password = "123456789";

// Insert Firebase project API Key
#define API_KEY "AIzaSyARqudZHpoDhe6_cSoapP4NkwlBLah5Ouc"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://homeautomation-67727-default-rtdb.firebaseio.com/"

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

uint32_t sendDataPrevMillis = 0;
uint32_t readDataPrevMillis = 0;
bool signupOK = false;
volatile bool dataChanged = false;



EnergyMonitor emon1;  // Create an instance
EnergyMonitor emon2;  // Create an instance
EnergyMonitor emon3;  // Create an instance

// uRTCLib rtc;
uRTCLib rtc(0x68);

#define AmpSensor1 36
#define AmpSensor2 39  //VN
#define AmpSensor3 34  //VP
#define SSR1 27
#define SSR2 25
#define SSR3 32
#define Button1 13
#define Button2 14
#define Button3 33

#define RXD2 16  // to TX of Pzem module
#define TXD2 17  // to RX of Pzem module
PZEM004Tv30 pzem(Serial2, RXD2, TXD2);
LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

String Mode = "0";        // 0 -> Manual, 1 -> Auto
String unitsReset = "0";  // 0 -> Not Reset, 1 -> Reset units
String btn1 = "0", btn2 = "0", btn3 = "0";
String load1 = "OFF", load2 = "OFF", load3 = "OFF";

// Parameters
float PowerT = 0.0, Power1 = 0.0, Power2 = 0.0, Power3 = 0.0;
float AmpsT = 0.0, Amps1 = 0.0, Amps2 = 0.0, Amps3 = 0.0;
float voltage = 0.0, pf = 0.0, freq = 0.0, units = 0.0;
String unitsSet = "0";
bool swapScreen = false;
// String time1start, time1end; // load 1 in hours only
// String time2start, time2end; // load 2 in hours only
// String time3start, time3end; // load 3 in hours only

void initFirebase() {
  // Firebase Initialization
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Wifi Connected !!");
  delay(1000);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase Connected !!");
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Internet Connected");
}

void sentdata() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 4000)) {
    Serial.println("Data Sent");

    // Writing values to Firebase Realtime database
    Firebase.RTDB.setString(&fbdo, "/Data/load1state", load1);
    Firebase.RTDB.setString(&fbdo, "/Data/load2state", load2);
    Firebase.RTDB.setString(&fbdo, "/Data/load3state", load3);
    Firebase.RTDB.setString(&fbdo, "/Data/power", String(PowerT, 1));
    Firebase.RTDB.setString(&fbdo, "/Data/current", String(AmpsT, 1));
    Firebase.RTDB.setString(&fbdo, "/Data/pow1", String(Power1, 1));
    Firebase.RTDB.setString(&fbdo, "/Data/pow2", String(Power2, 1));
    Firebase.RTDB.setString(&fbdo, "/Data/pow3", String(Power3, 1));
    Firebase.RTDB.setString(&fbdo, "/Data/amp1", String(Amps1, 1));
    Firebase.RTDB.setString(&fbdo, "/Data/amp2", String(Amps2, 1));
    Firebase.RTDB.setString(&fbdo, "/Data/amp3", String(Amps3, 1));
    Firebase.RTDB.setString(&fbdo, "/Data/units", String(units, 2));
    Firebase.RTDB.setString(&fbdo, "/Data/pf", String(pf, 2));
    Firebase.RTDB.setString(&fbdo, "/Data/freq", String(freq, 1));
    Firebase.RTDB.setString(&fbdo, "/Data/voltage", String(voltage, 1));
    sendDataPrevMillis = millis();
  }
}

void read_data() {
  if (Firebase.ready() && signupOK && (millis() - readDataPrevMillis > 2000)) {
    digitalWrite(2, HIGH);
    // if (Firebase.RTDB.getFloat(&fbdo, "/Data/units")) {
    //   if (fbdo.dataType() == "float") {
    //     units = fbdo.floatData();
    //     Serial.println("units => " + units);
    //   }
    // }
    if (Firebase.RTDB.getString(&fbdo, "/UserInput/mode")) {
      if (fbdo.dataType() == "string") {
        Mode = fbdo.stringData();
        Serial.println("Mode => " + Mode);
      }
    }
    if (Firebase.RTDB.getString(&fbdo, "/UserInput/units")) {
      if (fbdo.dataType() == "string") {
        unitsSet = fbdo.stringData();
        Serial.println("Units Set => " + fbdo.stringData());
      }
    }
    if (Firebase.RTDB.getString(&fbdo, "/UserInput/load1")) {
      if (fbdo.dataType() == "string") {
        btn1 = fbdo.stringData();
        // Serial.println(load1);
      }
    }
    if (Firebase.RTDB.getString(&fbdo, "/UserInput/load2")) {
      if (fbdo.dataType() == "string") {
        btn2 = fbdo.stringData();
        // Serial.println(load2);
      }
    }
    if (Firebase.RTDB.getString(&fbdo, "/UserInput/load3")) {
      if (fbdo.dataType() == "string") {
        btn3 = fbdo.stringData();
        // Serial.println(load3);
      }
    }
    if (Firebase.RTDB.getString(&fbdo, "/UserInput/reset")) {
      if (fbdo.dataType() == "string") {
        unitsReset = fbdo.stringData();
        if (unitsReset == "1") {
          units = 0;
          pzem.resetEnergy();
          delay(1000);
          Firebase.RTDB.setString(&fbdo, "/UserInput/reset", "0");
        }
      }
    }
    // if (Firebase.RTDB.getString(&fbdo, "/UserInput/time1")) {
    //   if (fbdo.dataType() == "string") {
    //     time1start = fbdo.stringData();
    //     Serial.println(time1start);
    //   }
    // }
    // if (Firebase.RTDB.getString(&fbdo, "/UserInput/time11")) {
    //   if (fbdo.dataType() == "string") {
    //     time1end = fbdo.stringData();
    //     Serial.println(time1end);
    //   }
    // }
    // if (Firebase.RTDB.getString(&fbdo, "/UserInput/time2")) {
    //   if (fbdo.dataType() == "string") {
    //     time2start = fbdo.stringData();
    //     Serial.println(time2start);
    //   }
    // }
    // if (Firebase.RTDB.getString(&fbdo, "/UserInput/time22")) {
    //   if (fbdo.dataType() == "string") {
    //     time2end = fbdo.stringData();
    //     Serial.println(time2end);
    //   }
    // }
    // if (Firebase.RTDB.getString(&fbdo, "/UserInput/time3")) {
    //   if (fbdo.dataType() == "string") {
    //     time3start = fbdo.stringData();
    //     Serial.println(time3start);
    //   }
    // }
    // if (Firebase.RTDB.getString(&fbdo, "/UserInput/time33")) {
    //   if (fbdo.dataType() == "string") {
    //     time3end = fbdo.stringData();
    //     Serial.println(time3end);
    //   }
    // }
    digitalWrite(2, LOW);
    readDataPrevMillis = millis();
  }
}
void initPins() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  // lcd.setCursor(0, 0);
  // lcd.print("ECC Controls");
  // lcd.setCursor(0, 1);
  // lcd.print("And Automation");

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
  emon1.current(AmpSensor1, 3.15);  // Current: input pin, calibration.
  emon2.current(AmpSensor2, 3.15);  // Current: input pin, calibration.
  emon3.current(AmpSensor3, 3.15);  // Current: input pin, calibration.

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  delay(1000);
}

void scanButtons() {
  if (digitalRead(Button1) == 0 || btn1 == "ON") {
    delay(100);
    if (digitalRead(Button1) == 0 || btn1 == "ON") {
      load1 = "ON";
      digitalWrite(SSR1, HIGH);
    }
  } else {
    load1 = "OFF";
    digitalWrite(SSR1, LOW);
  }
  if (digitalRead(Button2) == 0 || btn2 == "ON") {
    delay(100);
    if (digitalRead(Button2) == 0 || btn2 == "ON") {
      load2 = "ON";
      digitalWrite(SSR2, HIGH);
    }
  } else {
    load2 = "OFF";
    digitalWrite(SSR2, LOW);
  }
  if (digitalRead(Button3) == 0 || btn3 == "ON") {
    delay(100);
    if (digitalRead(Button3) == 0 || btn3 == "ON") {
      load3 = "ON";
      digitalWrite(SSR3, HIGH);
    }
  } else {
    load3 = "OFF";
    digitalWrite(SSR3, LOW);
  }
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
    voltage = 0;
    success = false;
  }
  return success;
}

void scanAmps() {
  Amps1 = emon1.calcIrms(1480);  // Calculate Irms only
  Amps2 = emon2.calcIrms(1480);  // Calculate Irms only
  Amps3 = emon3.calcIrms(1480);  // Calculate Irms only
  if (Amps1 < 0.2) {
    Amps1 = 0.0;
  }
  if (Amps2 < 0.2) {
    Amps2 = 0.0;
  }
  if (Amps3 < 0.2) {
    Amps3 = 0.0;
  }
}

void lcdUpdate2() {
  // lcd.clear();
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
  lcd.print(PowerT, 1);
  lcd.print(" W");
  lcd.setCursor(0, 3);
  lcd.print("Units = ");
  lcd.print(units, 2);
  lcd.print(" KWh");
}

void lcdUpdate1() {
  // lcd.clear();
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
  // lcd.print("  Freq= ");
  // lcd.print(freq, 1);
  // lcd.print(" Hz");
}
