#include <SPI.h>
#include <SD.h>
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include "Adafruit_MAX31855.h"
#include <pt.h>
#include <Arduino.h>
#include <SparkFunDS3234RTC.h>

// DS3234 RTC & Micro-SD card module-----------------------------------------------
#define DS13074_CS_PIN 53 // DeadOn RTC Chip-select pin
File myFile; 
int pinCS = 49;
String file_name; // The file name is gonna be the current date
//---------------------------------------------------------------------------------

// Blynk cloud definitions --------------------------------------------------------
#define BLYNK_TEMPLATE_ID "TMPLeeSIRDjZ"
#define BLYNK_DEVICE_NAME "ESP8266 Arduino Mega Blynk"
#define BLYNK_AUTH_TOKEN "lV19Y5qjtKdlwqusw62XJnsGql8ns5Le"
//---------------------------------------------------------------------------------

//Thermocouples defines & includes & initialization -------------------------------
#define MAXDO1   22
#define MAXCS1   24
#define MAXCLK1  26
#define MAXDO2   28
#define MAXCS2   30
#define MAXCLK2  32
#define MAXDO3   23
#define MAXCS3   25
#define MAXCLK3  27
#define MAXDO4   29
#define MAXCS4   31
#define MAXCLK4  33
Adafruit_MAX31855 thermocouple1(MAXCLK1, MAXCS1, MAXDO1);
Adafruit_MAX31855 thermocouple2(MAXCLK2, MAXCS2, MAXDO2);
Adafruit_MAX31855 thermocouple3(MAXCLK3, MAXCS3, MAXDO3);
Adafruit_MAX31855 thermocouple4(MAXCLK4, MAXCS4, MAXDO4);
//----------------------------------------------------------------------------------

// Liquid level sensors ------------------------------------------------------------
int Liquid_Level;
int HIGH_Liquid_Level = 1;
int LOW_Liquid_Level = 0;
//----------------------------------------------------------------------------------

// Static pressure sensors --------------------------------------------
int rawValue; // A/D readings
int offset = 410; // zero pressure adjust
int fullScale = 9630; // max pressure (span) adjust
float pressure1; // final pressure
float pressure2; // final pressure
// --------------------------------------------------------------------

// Crrent - Voltage sensors -------------------------------------------
float average_current1;
float average_current2;
float average_current3;
float average_voltage1;
float average_voltage2;
float average_voltage3;
// --------------------------------------------------------------------

// Differntial pressure sensors & Flow Rate----------------------------
float DP1_Interpreted_Value;
float DP2_Interpreted_Value;
float Q11;
float Q12;
// --------------------------------------------------------------------

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "YairIOS";
char pass[] = "yair1234";

// ESP8266 Definition --------
#define EspSerial Serial1
#define ESP8266_BAUD 38400
ESP8266 wifi(&EspSerial);
// ---------------------------

// Define protothreads -----------------------------------------
static struct pt pt1, pt2, pt3, pt4, pt5, pt6, pt7, pt8, pt9, pt10, pt11, pt12, pt13, pt14, pt15, pt16, pt17, pt18;
// -------------------------------------------------------------

// LCDs Definition ----------------------------------------------------
LiquidCrystal_I2C lcd1(0x20,20,4);  // LCD1 - Temperature monitoring
LiquidCrystal_I2C lcd2(0x21,20,4);  // LCD2 - Liquid level
LiquidCrystal_I2C lcd3(0x22,20,4);  // LCD3 - TBD
LiquidCrystal_I2C lcd4(0x23,20,4);  // LCD4 - TBD
LiquidCrystal_I2C lcd5(0x24,20,4);  // LCD5 - TBD
// --------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  Serial.println("Start with setup() function");

  // Blynk Initialization --------------------------------
  EspSerial.begin(ESP8266_BAUD);
  Blynk.begin(auth, wifi, ssid, pass, "blynk.cloud", 80);
  // Wait for Blynk connection to be established
  while (!Blynk.connected()) 
  {
    delay(500);
  }
  Serial.println("Connected to Blynk cloud !");
  // -----------------------------------------------------

  // LCD Initialization ----------------------------------
  Serial.println("Start with LCD initialization");
  lcd1.init();                      // initialize the lcd1 
  lcd2.init();                      // initialize the lcd2
  lcd3.init();                      // initialize the lcd3 
  lcd4.init();                      // initialize the lcd4 
  lcd5.init();                      // initialize the lcd5
  lcd1.backlight();
  lcd2.backlight();
  lcd3.backlight();
  lcd4.backlight();
  lcd5.backlight();
  Serial.println("Finish with LCD initialization");
  // -----------------------------------------------------

  // Pointers --------------------------------------------
  PT_INIT(&pt1);
  PT_INIT(&pt2);
  PT_INIT(&pt3);
  PT_INIT(&pt4);
  PT_INIT(&pt5);
  PT_INIT(&pt6);
  PT_INIT(&pt7);
  PT_INIT(&pt8);
  PT_INIT(&pt9);
  PT_INIT(&pt10);
  PT_INIT(&pt11);
  PT_INIT(&pt12);
  PT_INIT(&pt13);
  PT_INIT(&pt14);
  PT_INIT(&pt15);
  PT_INIT(&pt16);
  PT_INIT(&pt17);
  PT_INIT(&pt18);
  // -----------------------------------------------------
  Serial.println("Finish with setup() function");

  // SD Card ---------------------------------------------
  pinMode(pinCS, OUTPUT);
  pinMode(48, OUTPUT); // For debuging the opening of the text file.
  if (SD.begin())
  {
    Serial.println("SD card is ready to use");
  }
  else
  {
    Serial.println("SD card initialization failed");
    return;
  }
  delay(1000);
  // -----------------------------------------------------

  // DS3234 RTC Module -----------------------------------
  rtc.begin(DS13074_CS_PIN);
  rtc.autoTime();
  rtc.update();
  rtc.enableAlarmInterrupt();
  rtc.setAlarm1(30);
  rtc.setAlarm2(rtc.minute() + 1);
  file_name = String(rtc.date()) + "-" + String(rtc.month()) + "-" + String(rtc.year()) + ".txt"; // Define the file name which is gonna be opened in the SD card.
  // -----------------------------------------------------
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------

void loop()
{
  // Blynk cloud -------------------------------
  Blynk.run();
  // -------------------------------------------

  // Thermocouples ------------------------------
  double c1 = thermocouple1.readCelsius();
  double c2 = thermocouple2.readCelsius();
  double c3 = thermocouple3.readCelsius(); 
  double c4 = thermocouple4.readCelsius();
  Blynk.virtualWrite(V0, c1);
  Blynk.virtualWrite(V1, c2);
  Blynk.virtualWrite(V13, c3);
  Blynk.virtualWrite(V14, c4);
  // --------------------------------------------

  // LCDS Display --------------------------------
  protothreadTempLCD(&pt1, c1, c2, c3, c4);
  protothreadLiquidLevelLCD(&pt2, Liquid_Level);
  protothreadDPLCD3(&pt3, DP1_Interpreted_Value, DP2_Interpreted_Value, Q11, Q12);
  protothreadSPLCD4(&pt4, pressure1, pressure2);
  protothreadCurrentVoltageLCD5(&pt5, average_current1, average_current2, average_current3, average_voltage1, average_voltage2, average_voltage3);
  // ---------------------------------------------

  // Liquid level ------------------------------
  // protothreadLiquidLevel(&pt6, Liquid_Level, LOW_Liquid_Level, HIGH_Liquid_Level);
  liquid_level();
  if (Liquid_Level < 500)
  {
    Blynk.logEvent("low_liquid_level"); // Enable the warning event of low liquid level
    Blynk.virtualWrite(V2, LOW_Liquid_Level);
  }
  else
  {
    Blynk.virtualWrite(V2, HIGH_Liquid_Level);  
  }
  // --------------------------------------------

  // Static pressure sensors ---------------------
  protothreadStaticPressure1(&pt7);
  protothreadStaticPressure2(&pt8);
  Blynk.virtualWrite(V5, pressure1);
  Blynk.virtualWrite(V6, pressure2);
  // ---------------------------------------------

  // DP Sensors ---------------------------------
  protothreadDifferntialPressure1(&pt9, DP1_Interpreted_Value);
  protothreadDifferntialPressure2(&pt10, DP2_Interpreted_Value);
  Blynk.virtualWrite(V3, DP1_Interpreted_Value);
  Blynk.virtualWrite(V4, DP2_Interpreted_Value);
  Blynk.virtualWrite(V15, Q11);
  Blynk.virtualWrite(V16, Q12);
  // ---------------------------------------------

  // DS3234 RTC Module ---------------------------
  protothread_RTC(&pt11);
  // ---------------------------------------------

  // Current - Voltage sensors -------------------
  protothreadcurrent1(&pt13);
  protothreadcurrent2(&pt14);
  protothreadcurrent3(&pt15);
  protothreadvoltage1(&pt16, average_current1);
  protothreadvoltage2(&pt17, average_current2);
  protothreadvoltage3(&pt18, average_current3);
  // ---------------------------------------------

  // SD Card module ------------------------------
  protothread_SD_Card(&pt12, c1, c2, c3, c4, DP1_Interpreted_Value, DP2_Interpreted_Value, pressure1, pressure2, Liquid_Level, file_name, 
  average_current1, average_current2, average_current3, average_voltage1, average_voltage2, average_voltage3, Q11, Q12);
  // ---------------------------------------------
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// 1 thread - Temperature LCD ----------------------------------
static int protothreadTempLCD(struct pt *pt, double c1, double c2, double c3, double c4)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
  {
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 5000);
    lcd1.setCursor(0,0);
    lcd1.print("Temperature 1:");
    lcd1.setCursor(0,1);
    lcd1.print(c1);
    lcd1.print(" Deg.Celsius");
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 5000);
    lcd1.setCursor(0,0);
    lcd1.print("Temperature 2:");
    lcd1.setCursor(0,1);
    lcd1.print(c2);
    lcd1.print(" Deg.Celsius");
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 5000);
    lcd1.setCursor(0,0);
    lcd1.print("Temperature 3:");
    lcd1.setCursor(0,1);
    lcd1.print(c3);
    lcd1.print(" Deg.Celsius");
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 5000);
    lcd1.setCursor(0,0);
    lcd1.print("Temperature 4:");
    lcd1.setCursor(0,1);
    lcd1.print(c4);
    lcd1.print(" Deg.Celsius");
  }
  PT_END(pt);
}
// -------------------------------------------------------------

// 2 thread - Liquid Level LCD ----------------------------
static int protothreadLiquidLevelLCD(struct pt *pt, double Liquid_Level)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
    {
      lastTimeDisplayed = millis();
      PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 1000);
      if (Liquid_Level < 500)
      {
        lcd2.setCursor(0,0);
        lcd2.print("Warning: Liquid");
        lcd2.setCursor(0,1);
        lcd2.print("level is too low");
      }
      else
      {
        lcd2.clear();
        lcd2.setCursor(2,0);
        lcd2.print("Liquid level");
        lcd2.setCursor(5,1);
        lcd2.print("is OK");
      }
    }
  PT_END(pt);
}
// -------------------------------------------------------------

// 3 thread - Differential Pressure LCD ------------------------
static int protothreadDPLCD3(struct pt *pt, double DP1_Interpreted_Value, double DP2_Interpreted_Value, float Q11, float Q12)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
    {
      lastTimeDisplayed = millis();
      PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 2750);
      lcd3.setCursor(0,0);
      lcd3.print("DP1: ");
      lcd3.print(DP1_Interpreted_Value);
      lcd3.setCursor(12,0);
      lcd3.print("[Pa]");
      lcd3.setCursor(0,1);
      lcd3.print("FR1: ");
      lcd3.print(Q11);
      lcd3.setCursor(10,1);
      lcd3.print("[m3/s]");
      lastTimeDisplayed = millis();
      PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 2750);
      lcd3.setCursor(0,0);
      lcd3.print("DP2: ");
      lcd3.print(DP2_Interpreted_Value);
      lcd3.setCursor(12,0);
      lcd3.print("[Pa]");
      lcd3.setCursor(0,1);
      lcd3.print("FR2: ");
      lcd3.print(Q12);
      lcd3.setCursor(10,1);
      lcd3.print("[m3/s]");
    }
  PT_END(pt);
}
// -------------------------------------------------------------

// 4 thread - Static Pressure LCD ------------------------------
static int protothreadSPLCD4(struct pt *pt, float pressure1, float pressure2)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
    {
      PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 3000);
      lcd4.setCursor(0,0);
      lcd4.print("SP1: ");
      lcd4.print(pressure1);
      lcd4.setCursor(11,0);
      lcd4.print("[kPa]");
      lastTimeDisplayed = millis();
      PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 3000);
      lcd4.setCursor(0,1);
      lcd4.print("SP2: ");
      lcd4.print(pressure2);
      lcd4.setCursor(11,1);
      lcd4.print("[kPa]");
    }
  PT_END(pt);
}
// -------------------------------------------------------------

// 5 thread - TBD LCD ------------------------------------------
static int protothreadCurrentVoltageLCD5(struct pt *pt, float average_current1, float  average_current2, float  average_current3, float  average_voltage1, 
float  average_voltage2, float  average_voltage3)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
    {
      lastTimeDisplayed = millis();
      PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 3000);
      lcd5.setCursor(0,0);
      lcd5.print("Current1: ");
      lcd5.print(average_current1);
      lcd5.setCursor(14,0);
      lcd5.print("mA");
      lcd5.setCursor(0,1);
      lcd5.print("Voltage1: ");
      lcd5.print(average_voltage1);
      lcd5.setCursor(15,1);
      lcd5.print("V");
      lastTimeDisplayed = millis();
      PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 3000);
      lcd5.setCursor(0,0);
      lcd5.print("Current2: ");
      lcd5.print(average_current2);
      lcd5.setCursor(14,0);
      lcd5.print("mA");
      lcd5.setCursor(0,1);
      lcd5.print("Voltage2: ");
      lcd5.print(average_voltage2);
      lcd5.setCursor(15,1);
      lcd5.print("V");
      lastTimeDisplayed = millis();
      PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 3000);
      lcd5.setCursor(0,0);
      lcd5.print("Current3: ");
      lcd5.print(average_current3);
      lcd5.setCursor(14,0);
      lcd5.print("mA");
      lcd5.setCursor(0,1);
      lcd5.print("Voltage3: ");
      lcd5.print(average_voltage3);
      lcd5.setCursor(15,1);
      lcd5.print("V");
    }
  PT_END(pt);
}
// -------------------------------------------------------------

// Liquid level sensor read funtion-----------------------------
int liquid_level()
{
  Liquid_Level = analogRead(A0);
  if (Liquid_Level < 500)
  {
    return LOW_Liquid_Level;
  }
  else
  {
    return HIGH_Liquid_Level;
  }
}
// -------------------------------------------------------------

// 6 thread -  Liquid level sensor reading ---------------------
static int protothreadLiquidLevel(struct pt *pt, double Liquid_Level, int LOW_Liquid_Level, int HIGH_Liquid_Level)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
  {
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 110);
    if (Liquid_Level < 500)
    {
      Blynk.logEvent("low_liquid_level"); // Enable the warning event of low liquid level
      Blynk.virtualWrite(V2, LOW_Liquid_Level);
    }
    else
    {
      Blynk.virtualWrite(V2, HIGH_Liquid_Level);  
    }
  }
  PT_END(pt);
}
// -------------------------------------------------------------

// Static Pressure1 sensors read funtion -----------------------

float static_pressure1()
{
  rawValue = 0;
  for (int x = 0; x < 10; x++) rawValue = rawValue + analogRead(A3);
  pressure1 = (rawValue - offset) * 700.0 / (fullScale - offset); // pressure conversion
  return pressure1;
}
float static_pressure2()
{
  rawValue = 0;
  for (int x = 0; x < 10; x++) rawValue = rawValue + analogRead(A4);
  pressure2 = (rawValue - offset) * 700.0 / (fullScale - offset); // pressure conversion
  return pressure2;
}
// -------------------------------------------------------------

// 7 thread - Static Pressure sensors reading ------------------
static int protothreadStaticPressure1(struct pt *pt)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
  {
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 150);
    static_pressure1();
  }
  PT_END(pt);
}
// -------------------------------------------------------------

// 8 thread - Static Pressure sensors reading ------------------
static int protothreadStaticPressure2(struct pt *pt)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
  {
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 170);
    static_pressure2();
  }
  PT_END(pt);
}
// -------------------------------------------------------------

// Differential Pressure sensors read funtion ----
float differntial_pressure1()
{
  float DP1_Sample = analogRead(A1); // First sensor is connected to pinout A1
  DP1_Interpreted_Value = DP1_Sample - 497;
  return DP1_Interpreted_Value;
}
float differntial_pressure2()
{
  float DP2_Sample = analogRead(A2); // Second sensor is connected to pinout A2
  DP2_Interpreted_Value = DP2_Sample - 499;
  return DP2_Interpreted_Value;
}
// -----------------------------------------------

// Flow Rate calculation -------------------------
float flow_rate1(float DP1_Interpreted_Value)
{
  float r11 = 15; // Please enter the radius inlet at A1 (mm)
  float r21 = 5; // Please enter the radius inlet at A2 (mm)
  float A11 = 3.14 * (pow(r11, 2)); // Calculation of area 1 (mm^2)
  float A21 = 3.14 * (pow(r21, 2)); // Calculation of area 2 (mm^2)
  float q11 = 2120; // Typical KOH liquid density (kg/(m^3))
  Q11 = sqrt((2 * DP1_Interpreted_Value)/(q11 * ((1/pow(A21, 2)) - (1/pow(A11, 2))))); // Flow rate via the first pipe
  return Q11;
}
float flow_rate2(float DP2_Interpreted_Value)
{
  float r12 = 15; // Please enter the radius inlet at A1 (mm)
  float r22 = 5; // Please enter the radius inlet at A2 (mm)
  float A12 = 3.14 * (pow(r12, 2)); // Calculation of area 1 (mm^2)
  float A22 = 3.14 * (pow(r22, 2)); // Calculation of area 2 (mm^2)
  float q12 = 2120; // Typical KOH liquid density (kg/(m^3))
  Q12 = sqrt((2 * DP1_Interpreted_Value)/(q12 * ((1/pow(A22, 2)) - (1/pow(A12, 2))))); // Flow rate via the first pipe
  return Q12; 
}
// -----------------------------------------------

// 9 thread - Differential Pressure sensors reading ------------
static int protothreadDifferntialPressure1(struct pt *pt, float DP1_Interpreted_Value)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
  {
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 50);
    differntial_pressure1();
    flow_rate1(DP1_Interpreted_Value);
  }
  PT_END(pt);
}
// -------------------------------------------------------------

// 10 thread - Differential Pressure sensors reading -----------
static int protothreadDifferntialPressure2(struct pt *pt, float DP2_Interpreted_Value)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
  {
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 70);
    differntial_pressure2();
    flow_rate2(DP2_Interpreted_Value);
  }
  PT_END(pt);
}
// -------------------------------------------------------------

// DS3234 RTC Print function -----------------------------------
void printTime()
{
  Serial.print(String(rtc.hour()) + ":"); // Print hour
  if (rtc.minute() < 10)
    Serial.print('0'); // Print leading '0' for minute
  Serial.print(String(rtc.minute()) + ":"); // Print minute
  if (rtc.second() < 10)
    Serial.print('0'); // Print leading '0' for second
  Serial.print(String(rtc.second())); // Print second

  if (rtc.is12Hour()) // If we're in 12-hour mode
  {
    // Use rtc.pm() to read the AM/PM state of the hour
    if (rtc.pm()) Serial.print(" PM"); // Returns true if PM
    else Serial.print(" AM");
  }
  Serial.print(" | ");
  // Few options for printing the day, pick one:
  Serial.print(rtc.dayStr()); // Print day string
  //Serial.print(rtc.dayC()); // Print day character
  //Serial.print(rtc.day()); // Print day integer (1-7, Sun-Sat)
  Serial.print(" - ");
  #ifdef PRINT_USA_DATE
    Serial.print(String(rtc.month()) + "/" +   // Print month
                  String(rtc.date()) + "/");  // Print date
  #else
    Serial.print(String(rtc.date()) + "/" +    // (or) print date
                  String(rtc.month()) + "/"); // Print month
  #endif
    Serial.println(String(rtc.year()));        // Print year
}
// -------------------------------------------------------------

// 11 Thread - DS3234 RTC module -------------------------------
static int protothread_RTC(struct pt *pt)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
  {
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 700);
    static int8_t lastSecond = -1;
    rtc.update();
    if (rtc.second() != lastSecond) // If the second has changed
    {
      printTime(); // Print the new time
      lastSecond = rtc.second(); // Update lastSecond value
    }
  }
  PT_END(pt);
}
// -------------------------------------------------------------

// 12 Thread - SD Card module writing --------------------------
static int protothread_SD_Card(struct pt *pt, double c1, double c2, double c3, double c4, double DP1_Interpreted_Value, double DP2_Interpreted_Value, float pressure1, 
float pressure2, double Liquid_Level, String file_name, float average_current1, float  average_current2, float  average_current3, float  average_voltage1, 
float  average_voltage2, float  average_voltage3, float Q11, float Q12)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
  {
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 4200);
    myFile = SD.open(file_name, FILE_WRITE);
    if (myFile)
    {
      Serial.println("Writing to file ...");
      myFile.println(String(rtc.date()) + "/" + String(rtc.month()) + "/" + String(rtc.year()) + " | " + String(rtc.hour()) + ":" + String(rtc.minute()) + ":" + String(rtc.second()));
      delay(5);
      myFile.print("    Temperature 1 is: ");
      myFile.print(c1);
      myFile.println(" Celsius Degrees");
      delay(5);
      myFile.print("    Temperature 2 is: ");
      myFile.print(c2);-
      myFile.println(" Celsius Degrees");
      delay(5);
      myFile.print("    Temperature 3 is: ");
      myFile.print(c3);-
      myFile.println(" Celsius Degrees");
      delay(5);
      myFile.print("    Temperature 4 is: ");
      myFile.print(c4);-
      myFile.println(" Celsius Degrees");
      delay(5);
      myFile.print("    DP 1 is: ");
      myFile.print(DP1_Interpreted_Value);
      myFile.println(" [Pa]");
      delay(5);
      myFile.print("    Flow Rate 1 is: ");
      myFile.print(Q11);
      myFile.println(" [m^3/s]");
      delay(5);
      myFile.print("    DP 2 is: ");
      myFile.print(DP2_Interpreted_Value);
      myFile.println(" [Pa]");
      delay(5);
      myFile.print("    Flow Rate 2 is: ");
      myFile.print(Q12);
      myFile.println(" [m^3/s]");
      delay(5);
      myFile.print("    SP 1 is: ");
      myFile.print(pressure1);
      myFile.println(" [kPa]");
      delay(5);
      myFile.print("    SP 2 is: ");
      myFile.print(pressure2);
      myFile.println(" [kPa]");
      delay(5);
      myFile.print("    Current 1 is: ");
      myFile.print(average_current1);
      myFile.println(" [mA]");
      delay(5);
      myFile.print("    Voltage 1 is: ");
      myFile.print(average_voltage1);
      myFile.println(" [V]");
      delay(5);
      myFile.print("    Current 2 is: ");
      myFile.print(average_current2);
      myFile.println(" [mA]");
      delay(5);
      myFile.print("    Voltage 2 is: ");
      myFile.print(average_voltage2);
      myFile.println(" [V]");
      delay(5);
      myFile.print("    Current 3 is: ");
      myFile.print(average_current3);
      myFile.println(" [mA]");
      delay(5);
      myFile.print("    Voltage 3 is: ");
      myFile.print(average_voltage3);
      myFile.println(" [V]");
      delay(5);
      if (Liquid_Level < 500)
      {
        myFile.println("    Warning - Liquid level is too low !! ");
        delay(5);
      }
      else
      {
        myFile.println("    Liquid level is OK");
        delay(5);
      }
      myFile.close();
      Serial.println("Done.");
      digitalWrite(48, HIGH);
    }
    else
    {
      Serial.println("Eror opening " + file_name );
      digitalWrite(48, LOW);
    }
  }
  PT_END(pt);
}
// -------------------------------------------------------------

// Current sensors + Voltage measurements ----------------------
float current1()
{
  average_current1 = 0;
  for(int i = 0; i < 10; i++)
  {
    average_current1 = average_current1 + (0.044 * analogRead(A5) - 3.78) / 1000; // Calculate the current in mA units
    delay(1);
  }
  return average_current1;
}
float current2()
{
  average_current2 = 0;
  for(int i = 0; i < 10; i++)
  {
    average_current2 = average_current2 + (0.044 * analogRead(A6) - 3.78) / 1000; // Calculate the current in mA units
    delay(1);
  }
  return average_current2;
}
float current3()
{
  average_current3 = 0;
  for(int i = 0; i < 10; i++)
  {
    average_current3 = average_current3 + (0.044 * analogRead(A7) - 3.78) / 1000; // Calculate the current in mA units
    delay(1);
  }
  return average_current3;
}
float voltage1(float average_current1)
{
  average_voltage1 = 0;
  average_voltage1 = (0.067 * average_current1) + 2.5; // Calculate the voltage in V units
  return average_voltage1;
}
float voltage2(float average_current2)
{
  average_voltage2 = 0;
  average_voltage2 = (0.067 * average_current2) + 2.5; // Calculate the voltage in V units
  return average_voltage2;
}
float voltage3(float average_current3)
{
  average_voltage3 = 0;
  average_voltage3 = (0.067 * average_current3) + 2.5; // Calculate the voltage in V units
  return average_voltage3;
}
// -------------------------------------------------------------

// 13 thread - Current1 ----------------------------------------
static int protothreadcurrent1(struct pt *pt)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
  {
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 6500);
    current1();
    Blynk.virtualWrite(V7, average_current1);
  }
  PT_END(pt);
}
// -------------------------------------------------------------

// 14 thread - Current2 ----------------------------------------
static int protothreadcurrent2(struct pt *pt)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
  {
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 6500);
    current2();
    Blynk.virtualWrite(V8, average_current2);
  }
  PT_END(pt);
}
// -------------------------------------------------------------

// 15 thread - Current3 ----------------------------------------
static int protothreadcurrent3(struct pt *pt)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
  {
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 6500);
    current3();
    Blynk.virtualWrite(V9, average_current3);
  }
  PT_END(pt);
}
// -------------------------------------------------------------

// 16 thread - Voltage1 ----------------------------------------
static int protothreadvoltage1(struct pt *pt, float average_current1)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
  {
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 7020);
    voltage1(average_current1);
    Blynk.virtualWrite(V10, average_voltage1);
  }
  PT_END(pt);
}
// -------------------------------------------------------------

// 17 thread - Voltage2 ----------------------------------------
static int protothreadvoltage2(struct pt *pt, float average_current2)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
  {
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 7020);
    voltage2(average_current2);
    Blynk.virtualWrite(V11, average_voltage2);
  }
  PT_END(pt);
}
// -------------------------------------------------------------

// 18 thread - Voltage3 ----------------------------------------
static int protothreadvoltage3(struct pt *pt, float average_current3)
{
  static unsigned long lastTimeDisplayed = 0;
  PT_BEGIN(pt);
  while(1)
  {
    lastTimeDisplayed = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeDisplayed > 7020);
    voltage3(average_current3);
    Blynk.virtualWrite(V12, average_voltage3);
  }
  PT_END(pt);
}
// -------------------------------------------------------------