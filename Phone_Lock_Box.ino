#include <LiquidCrystal.h>
#include <Servo.h>
#include <string.h>

// button pins
const int TIME_BUTTON  = 4;
const int START_BUTTON = 7;


//   led pins
const int RED_PIN   = 3;
const int GREEN_PIN = 6;
const int BLUE_PIN  = 5;

// servo pins
const int SERVO_PIN = 2;


// lcd pins
const int LCD_RS = 13;
const int LCD_EN = 12;
const int LCD_D4 = 11;
const int LCD_D5 = 10;
const int LCD_D6 = 9;
const int LCD_D7 = 8;

// object declaration
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
Servo myServo;



enum State {     //states are basically setting up the mode of the device. We can add some if needed but these are the basics
  IDLE,   //mode when nothing has been set and the box is unlocked
  SET_TIME,   // mode for when setting the time for the machine. Mode where the buttons can be pressed to cycle the time
  LOCKED,   //mode when the timer is ticking down after being set. This is tracking the time elapsed and also sending occasional messages across the lcd. Sets mode to unlocked when complete
  UNLOCKED    // after timer has run out, briefly gives congrats and resets to idle mode
};
unsigned long selectedMinutes = 5;     //variable for minutes set, is cycled by button
unsigned long lockDurationMs = 0;    // time that the lock is engaged, ticks down over time when in locked mode
unsigned long startTime = 0;   
unsigned long lastScrollTime = 0;

State currentState = IDLE;    //sets mode to idle

const int LOCK_POS = 50; //locked position of the servo
const int UNLOCK_POS = 100; //unlocked and idle position of the servo, change after we know the box

bool buttonPressed(int pin) {
  static bool wasPressed[20];  // track previous state

  bool isPressed = (digitalRead(pin) == LOW);

  if (isPressed && !wasPressed[pin]) {
    wasPressed[pin] = true;
    return true;          // trigger  once
  }

  if (!isPressed) {
    wasPressed[pin] = false;  // reset when released 
  }

  return false;
}

void printTime(unsigned long ms) {
  unsigned long totalSeconds = ms / 1000;
  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;

  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);
}
void scrollMessage(const char* msg, bool reset = false) {
  static int index = 0;
  int len = strlen(msg);

  if (reset) {
    index = 0;
  }

  lcd.setCursor(0, 1);

  for (int i = 0; i < 16; i++) {
    int charIndex = (index + i) % len;
    lcd.print(msg[charIndex]);
  }

  index = (index + 1) % len;
}

void setup() {
  Serial.begin(9600);

  // LCD
  lcd.begin(16, 2);
  lcd.clear();

  // Buttons
  pinMode(TIME_BUTTON, INPUT_PULLUP);
  pinMode(START_BUTTON, INPUT_PULLUP);

  // RGB LED
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Servo
  myServo.attach(SERVO_PIN);
  myServo.write(100); // unlocked position

  lcd.print("Ready");
}


void loop() {

  switch (currentState) {

    case IDLE:
      setLED(100,100,100); 
      myServo.write(UNLOCK_POS);

      lcd.setCursor(0,0);
      lcd.print("Set Time:");
      lcd.setCursor(0,1);
      lcd.print(selectedMinutes);
      lcd.print(" min   ");

      if (buttonPressed(TIME_BUTTON)) {
        selectedMinutes += 5;
        if (selectedMinutes > 60) selectedMinutes = 5;
      }

      if (buttonPressed(START_BUTTON)) {
        lockDurationMs = selectedMinutes * 60UL * 1000UL;
        startTime = millis();
        currentState = LOCKED;
        myServo.write(LOCK_POS);
        setLED(0,100,0); 
        delay(300);
      }
      break;

    case LOCKED: {
  setLED(100,100,0);

  unsigned long elapsed = millis() - startTime;

  lcd.setCursor(0,0);
  lcd.print("Time Left:");
  lcd.setCursor(0,1);
 unsigned long remaining = 0;
if (elapsed < lockDurationMs) {
  remaining = lockDurationMs - elapsed;
}
printTime(remaining);
lcd.print("  ");


  if (millis() - lastScrollTime > 200) {
    scrollMessage("                                            Stay focused!                                     You got this!                                            Keep it up!                           Lock in!                         No distractions!                         You're doing great!                              ");
    lastScrollTime = millis(); 
  }

  if (elapsed >= lockDurationMs) {
    currentState = UNLOCKED;
    myServo.write(UNLOCK_POS);
  }
}
break;

    case UNLOCKED:
      setLED(100,0,0);
      lcd.clear();
      lcd.print("Unlocked!");

      delay(3000);
      selectedMinutes = 5;
      currentState = IDLE;
      break;
  }
}

int brightness = 60;

void setLED(int r, int g, int b) {
  analogWrite(RED_PIN,   (r * brightness) / 255);
  analogWrite(GREEN_PIN, (g * brightness) / 255);
  analogWrite(BLUE_PIN,  (b * brightness) / 255);
}
