#ifndef PRAYER_CLOCK_HH
#define PRAYER_CLOCK_HH

#include <TM1637.h>
#include <MD_MAX72XX.h>
#include <MD_Parola.h>
#include <SPI.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <time.h>
#include <string>

#include "secrets.hh"

// TM1637 pin definitions
#define CLK_ASR 22
#define DIO_ASR 23

#define CLK2 3
#define DIO2 1

#define CLK3 2
#define DIO3 33

#define CLK4 22
#define DIO4 23

#define CLK5 18
#define DIO5 19

#define CLK6 2
#define DIO6 0

// MAX7219 definitions
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define DATA_PIN 17
#define CLK_PIN 18
#define CS_PIN 5

// Segment definition
extern const uint8_t SEGMENTS[] = {0b01111111, 0b01111111, 0b01111111, 0b01111111};;

// TM1637 display objects
extern TM1637 display1;
extern TM1637 display2;
extern TM1637 display3;
extern TM1637 display4;
extern TM1637 display5;
extern TM1637 display6;

// MD_MAX72XX object
extern MD_Parola max72;

extern String fajr, sunrise, dhuhr, asr, maghrib, isha;
extern const char* MONTHS[];

extern struct tm timeinfo;

extern uint32_t t1, t2;

// function definitions
void connect_wifi();
String prayer_times();
void parsePrayerTimes(const String& jsonString, uint8_t hours[], uint8_t minutes[]);
String get_date();
String get_time();

#endif 
