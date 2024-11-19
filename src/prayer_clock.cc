#include "prayer_clock.hh"

TM1637 display1;
TM1637 display2;
TM1637 display3;
TM1637 display4;
TM1637 display5;
TM1637 display6;


MD_Parola max72 = MD_Parola(
  HARDWARE_TYPE,
  DATA_PIN,
  CLK_PIN,
  CS_PIN,
  MAX_DEVICES
);

// MD_Parola max72 = MD_Parola(
//   HARDWARE_TYPE,
//   CS_PIN,
//   MAX_DEVICES
// );

const long INTERVAL = 20000; 

const uint8_t SEGMENTS[] = {0b01111111, 0b01111111, 0b01111111, 0b01111111};
const char* MONTHS[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

struct tm timeinfo;

void connect_wifi() {

  Serial.print("connecting to ");
  Serial.print(WIFI_SSID);
  Serial.print(" with identity ");
  Serial.print(EAP_IDENTITY);
  Serial.print(": ");
  WiFi.disconnect(true);  //disconnect from wifi to set new wifi connection
  WiFi.mode(WIFI_STA);    //init wifi mode
  WiFi.begin(WIFI_SSID, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    i++;
    if (i >= 60) {  //after timeout - reset board
      ESP.restart();
      return;
    }
  }
  Serial.println();
  Serial.println("connected!");
  Serial.print("ip address: ");
  Serial.println(WiFi.localIP());
}

String prayer_times() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin("http://nasif.ca/prayer-times");
    uint16_t resp_code = http.GET();

    if (resp_code > 0) {
      String response = http.getString();
      return response;
      // Serial.println(response);

    }
    else {
      Serial.print("error in GET request: ");
      Serial.println(resp_code);
      return "";
    }

    http.end();  
  }
}

void parsePrayerTimes(const String& jsonString, uint8_t hours[], uint8_t minutes[]) {
  const char* keys[] = {"Fajr", "Sunrise", "Dhuhr", "Asr", "Maghrib", "Isha"};
  const uint8_t numKeys = 6;

  for (int i = 0; i < numKeys; ++i) {
    String key = "\"" + String(keys[i]) + "\":\""; // search pattern
    uint8_t startIndex = jsonString.indexOf(key);

    if (startIndex != -1) {
      startIndex += key.length(); 
      uint8_t endIndex = jsonString.indexOf("\"", startIndex);

      if (endIndex != -1) {
        String timeString = jsonString.substring(startIndex, endIndex);

        uint8_t hour = timeString.substring(0, timeString.indexOf(":")).toInt();
        uint8_t minute = timeString.substring(timeString.indexOf(":") + 1, timeString.indexOf(" ")).toInt();

        
        if (timeString.endsWith("PM") && hour != 12) {
          hour += 0;  // 12 hour
          // hour += 12; // 24 hour
        }
        else if (timeString.endsWith("AM") && hour == 12) {
          hour = 0; // midnight case
        }

        hours[i] = hour;
        minutes[i] = minute;
      }
    }
  }
}

String get_date() {
  struct tm timeinfo;
  char strftime_buf[64];

  if (getLocalTime(&timeinfo)) {
    strftime(strftime_buf, sizeof(strftime_buf), "%H:%M", &timeinfo);
    return String(strftime_buf);
  }
  else {
    return "failed to obtain time";
  }
}

String get_time() {
  struct tm timeinfo;
  char strftime_buf[64];

  if (getLocalTime(&timeinfo)) {
    strftime(strftime_buf, sizeof(strftime_buf), "%H:%M", &timeinfo);
    return String(strftime_buf);
  }
  else {
    return "failed to obtain time";
  }
}

void setup() {
  Serial.begin(9600);
  delay(10);
  // TM1637
  display1.begin(CLK_ASR, DIO_ASR, 4);
  display2.begin(CLK2, DIO2, 4);
  display3.begin(CLK3, DIO3, 4);
  display4.begin(CLK4, DIO4, 4);
  display5.begin(CLK5, DIO5, 4);
  display6.begin(CLK6, DIO6, 4);

  display1.setBrightness(7);
  display2.setBrightness(7);
  display3.setBrightness(7);
  display4.setBrightness(7);
  display5.setBrightness(7);
  display6.setBrightness(7);

  // fetchPrayerTimes();
  display1.displayTime(6, 18, 1);
  display2.displayTime(8, 15, 1);
  display3.displayTime(1, 19, 1);
  display4.displayTime(3, 49, 1);
  display5.displayTime(6, 22, 1);
  display6.displayTime(8, 13, 1);

  // MAX7219
  max72.begin(2);
  max72.setInvert(false);
  delay(100);
  max72.setZone(0, MAX_DEVICES - 4, MAX_DEVICES - 1);
  max72.setZone(1, MAX_DEVICES - 4, MAX_DEVICES - 1);
  max72.displayZoneText(1, "...", PA_CENTER, 75, 20000, PA_FADE);

  connect_wifi();

  max72.setIntensity(15);

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  // Wait until time is synchronized
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("waiting for time...");
    delay(1000);
  }

  uint32_t t1 = millis();
}

void loop() {

  if (max72.displayAnimate()) {
    uint32_t t2 = millis();
    if (t2 - t1 >= interval) {
      t1 = t2;
      String json = prayer_times();
      Serial.println(json);
      if (json != "") {
        uint8_t hours[6];
        uint8_t minutes[6];
        parsePrayerTimes(json, hours, minutes);

        Serial.println("fajr: " + String(hours[0]) + ":" + String(minutes[0]));
        Serial.println("sunrise: " + String(hours[1]) + ":" + String(minutes[1]));
        Serial.println("dhuhr: " + String(hours[2]) + ":" + String(minutes[2]));
        Serial.println("asr: " + String(hours[3]) + ":" + String(minutes[3]));
        Serial.println("maghrib: " + String(hours[4]) + ":" + String(minutes[4]));
        Serial.println("isha: " + String(hours[5]) + ":" + String(minutes[5]));

        display1.displayTime(hours[0], minutes[0], 1);
        display2.displayTime(hours[1], minutes[1], 1);
        display3.displayTime(hours[2], minutes[2], 1);
        display4.displayTime(hours[3], minutes[3], 1);
        display5.displayTime(hours[4], minutes[4], 1);
        display6.displayTime(hours[5], minutes[5], 1);
      }
      else {
        Serial.println("failed to fetch prayer times");
      }

      // tm time = get_time();

      getLocalTime(&timeinfo);
      char time[6];
      sprintf(time, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
      char date[7];
      sprintf(date, "%s %02d", MONTHS[timeinfo.tm_mon], timeinfo.tm_mday);

      static bool state;
      if (state) {
        max72.displayZoneText(1, time, PA_CENTER, 75, 20000, PA_PRINT);
        Serial.println(time);
      }
      else {
        max72.displayZoneText(0, date, PA_CENTER, 75, 20000, PA_PRINT);
        Serial.println(date);
      }
      state = !state;
      }
    }
  }

