#include <Arduino.h>
#include <RTClib.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>

void displayUnlocked();
void displayLocked();
void displayLocking();
void openGate();
void alarm();
int readButtons();

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET 4        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x 64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RTC_DS3231 rtc;
Bounce bounce = Bounce(); // instantiate bounce objects for all 5 buttons/pins being used...

int days = 0;
int hours = 0;

void setup()
{
  Serial.begin(9600);

  // initializing the rtc
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC!");
    Serial.flush();
    abort();
  }
  rtc.disable32K();                //we don't need the 32K Pin, so disable it
  // rtc.clearAlarm(1);   CLEARS THE ALARM ON RESTART (DELETE THIS)            
  rtc.clearAlarm(2);               // if not done, this easily leads to problems, as both register aren't reset on reboot/recompile
  rtc.writeSqwPinMode(DS3231_OFF); // stop oscillating signals at SQW Pin, otherwise setAlarm1 will fail
  rtc.disableAlarm(2);             // turn off alarm 2 (in case not off already) again, this isn't done at reboot, so previously set alarm could go overlooked

  // Start the OLED display
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();

  // setup input buttons
  pinMode(8, INPUT_PULLUP); // set Button
  pinMode(9, INPUT_PULLUP); // day PlusButton
  pinMode(10, INPUT_PULLUP); // day MinusButton
  pinMode(11, INPUT_PULLUP); // hour PlusButton
  pinMode(12, INPUT_PULLUP); // hour MinusButton
}

void loop()
{
  display.clearDisplay(); // need this or else it fills up screen w dots
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0); // this prevents the text from scrolling off screen

  Serial.print("Alarm Fired:");
  Serial.println(rtc.alarmFired(1));

  if (rtc.alarmFired(1))
  {
    displayUnlocked();
    if (readButtons())
    {
      displayLocking();
    }
  }
  else
  {
    displayLocked();
  }
  delay(200);
}

// Read day and hour buttons, and return 1 if set button is pressed
int readButtons()
{
  int setButton = digitalRead(8);
  if (setButton) {
    return 1; // return 1 if the set button is pressed to start the lock cycle
  }
  int dayPlusButton = digitalRead(9);
  int dayMinusButton = digitalRead(10);
  int hourPlusButton = digitalRead(11);
  int hourMinusButton = digitalRead(12);
  if (dayPlusButton) 
  {
    days++;
    Serial.println("days++");
  }
  if (dayMinusButton) 
  {
    days--;
    Serial.println("days--");
  }
  if (hourPlusButton) 
  {
    hours++;
    Serial.println("hours++");
  }
  if (hourMinusButton) 
  {
    hours--;
    Serial.println("hours--");
  }
  return 0;
}

void displayUnlocked()
{
  display.println("~~ UNLOCKED ~~");
  display.println("Set Lock Time:");
  display.print("Days:");
  display.println(days);
  display.print("Hours:");
  display.println(hours);
  display.display();

}

void displayLocking()
{
  for (int i = 5; i > 0; i--) 
  {
    display.clearDisplay();
    display.setCursor(0, 0); // this prevents the text from scrolling off screen
    display.println("-~ LOCKING ~-");
    display.print("In ");
    display.print(i);
    display.println(" seconds");
    display.display();
    delay(1000);
  }
  Serial.print("Status");
  Serial.println(rtc.alarmFired(1));
  rtc.clearAlarm(1);
  Serial.print("Status");
  Serial.println(rtc.alarmFired(1));
  rtc.setAlarm1(rtc.now() + TimeSpan(hours), DS3231_A1_Second);
  Serial.print("Status");
  Serial.println(rtc.alarmFired(1));
  Serial.println("TURN LOCK");
  Serial.print("Unlock in ");
  Serial.print(hours); // TODO update later...
  Serial.println(" seconds!");
}

void displayLocked()
{
  // TODO: Retrieve the days and hours left for display from RTC (need to convert)
  display.println("-- LOCKED --");
  display.println("Time Remaining:");
  display.print("Days:");
  display.println("??");
  display.print("Hours:");
  display.println("??");
  display.display();
}
