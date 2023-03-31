#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <Servo.h>

#define BLYNK_TEMPLATE_ID "TMPL6cOOxOaPX"
#define BLYNK_TEMPLATE_NAME "FishFood"
#define BLYNK_AUTH_TOKEN "Hag-HVbatNTNiOcm9GaP9s-5A5oeEt1k"
#define servo 2
#define countof(a) (sizeof(a) / sizeof(a[0]))

bool button_state = false;
Servo myServo;
ThreeWire myWire(5, 4, 16);  // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

int _hour, _min, _second ;

void setup() {
  Serial.begin(9600);
  Serial.println();

  WiFi.begin("Rew52", "0865507910");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  Blynk.begin(BLYNK_AUTH_TOKEN, "Rew52", "0865507910", "blynk.cloud", 80);

  myServo.attach(servo);
  Serial.begin(9600);

  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();

  if (!Rtc.IsDateTimeValid()) {
    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected()) {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled) {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  } else if (now > compiled) {
    Serial.println("RTC is newer than compile time. (this is expected)");
  } else if (now == compiled) {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }
}

void loop() {
  Blynk.run();
  RtcDateTime now = Rtc.GetDateTime();
  String hourStr = String(now.Hour());
  if (hourStr.length() == 1) {
    hourStr = "0" + hourStr;
  }

  String minuteStr = String(now.Minute());
  if (minuteStr.length() == 1) {
    minuteStr = "0" + minuteStr;
  }

  String secondStr = String(now.Second());
  if (secondStr.length() == 1) {
    secondStr = "0" + secondStr;
  }

  String timeStr = hourStr + ":" + minuteStr + ":" + secondStr;
  Blynk.virtualWrite(V1, timeStr);
  Blynk.virtualWrite(V2, __DATE__);

  printDateTime(now);
  Serial.println();

  if (now.Hour() == _hour && now.Minute() == _min && now.Second() == _second) {
    runServo();
  }

  if (!now.IsValid()) {
    Serial.println("RTC lost confidence in the DateTime!");
  }

  if (button_state == 1) {
    runServo();
  } else {
    myServo.write(0);
  }
  Serial.println(String("Button (on-off): ") + button_state);
  delay(1000);
}

BLYNK_WRITE(V3) {
  button_state = param.asInt();
}
BLYNK_WRITE(V4) {
  TimeInputParam t(param);
  _hour = t.getStartHour() ? t.getStartHour() : 0;
  _min = t.getStartMinute() ? t.getStartMinute() : 0;
  _second = t.getStartSecond() ? t.getStartSecond() : 0;
}

BLYNK_WRITE(V6) {
  int servoAngle = param.asInt();
  myServo.write(servoAngle);
  delay(400);
}

void printDateTime(const RtcDateTime& dt) {
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second());
  Serial.print(datestring);
}

void runServo() {
  myServo.write(0);
  delay(500);

  myServo.write(90);
  delay(500);

  myServo.write(180);
  delay(1000);

  myServo.write(0);
  delay(500);
}