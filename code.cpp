  
#include "thingProperties.h"
#include <Arduino.h>
#include <Bounce2.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

const String componentNames[3] = {"WATER PUMP", "SUCTION PUMP", "BLOWER"};
const String allOnButtonName = "START/STOP AUTO";

const int componentPins[3] = {27, 26, 25}; 
const int buttonPins[4] = {13, 12, 14, 33};
const int ledPins[3] = {18, 5, 15}; 
const int BUZZER_PIN = 4;

const unsigned long waterPumpOnTime = 15000;
const unsigned long suctionPumpDelay = 5000; 
const unsigned long suctionPumpOnTime = 25000;
const unsigned long blowerOnTime = 40000;
const unsigned long screenChangeInterval = 5000;

const unsigned long debounceDelay = 50;
Bounce debouncers[4]; 

bool componentStates[3] = {false, false, false}; 
bool automationRunning = false;
bool buttonPressed[4] = {false, false, false, false};

unsigned long automationStartTime = 0;
unsigned long lastScreenChangeTime = 0;
int currentComponentIndex = -1; 
int currentScreen = 0; 

String lastLine1 = ""; 
String lastLine2 = "";

void playToneSequence(const int frequencies[], int length, int duration, int delayBetween) {
  for (int i = 0; i < length; i++) {
    tone(BUZZER_PIN, frequencies[i], duration);
    delay(duration + delayBetween);
  }
  noTone(BUZZER_PIN);
}

void playStartupSound() {
  const int frequencies[] = {1200, 1800, 1600, 2000, 2400};
  playToneSequence(frequencies, 5, 150, 100);
}

void playOnOffButtonSound() {
  const int frequencies[] = {2440};
  playToneSequence(frequencies, 1, 50, 0);
}

void playAutomationToggleSound() {
  const int frequencies[] = {1800, 1600, 2400};
  playToneSequence(frequencies, 3, 100, 50);
}

void playAutomationCompletionSound() {
  const int frequencies[] = {2000, 2200, 2400, 2600};
  playToneSequence(frequencies, 4, 200, 50);
}

void playComponentSwitchingSound() {
  const int frequencies[] = {1800, 2000};
  playToneSequence(frequencies, 2, 100, 30);
}



void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
 initProperties();

  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  Serial.println("\n***** Welcome to the Automatic Sanitation System *****");
  Serial.println("Press the buttons or use the 'Start/Stop Automation' button to control the system.");

  for (int i = 0; i < 3; i++) {
    pinMode(componentPins[i], OUTPUT);
    digitalWrite(componentPins[i], HIGH); 
    pinMode(ledPins[i], OUTPUT); 
    digitalWrite(ledPins[i], LOW); 
  }

  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    debouncers[i].attach(buttonPins[i]);
    debouncers[i].interval(debounceDelay);
  }

  pinMode(BUZZER_PIN, OUTPUT);
  playStartupSound();

  displayCenteredText("WELCOME TO", "AUTO SANITATION");
  delay(2000);
  displayCenteredText("PRESS ANY", "BUTTON TO START");
  delay(2000);

  lastScreenChangeTime = millis(); 
  Serial.println("System ready.");
}





  void loop() {

      ArduinoCloud.update();

  for (int i = 0; i < 4; i++) {
    debouncers[i].update();
    bool pressed = debouncers[i].fell();

    if (pressed && !buttonPressed[i]) {
      buttonPressed[i] = true;
      handleButtonPress(i);
    } else if (!debouncers[i].fell() && buttonPressed[i]) {
      buttonPressed[i] = false;
    }
  }

  if (automationRunning) {
    handleAutomationSequence();
  } else {
    if (millis() - lastScreenChangeTime >= screenChangeInterval && !anyComponentRunning()) {
      lastScreenChangeTime = millis();
      currentScreen = (currentScreen + 1) % 3; 

      updateScreen();
    } else if (anyComponentRunning()) {
      displayComponentStatus();
    }
  }
}

void handleButtonPress(int buttonIndex) {
  if (buttonIndex == 3) { 
    automationRunning = !automationRunning;
    if (automationRunning) {
      startAutomation();
    } else {
      stopAutomation();
    }
    playAutomationToggleSound();
  } else { 
    toggleComponent(buttonIndex);
    playOnOffButtonSound();
  }
}

void startAutomation() {
  Serial.println(getTimestamp() + " Starting automation sequence...");
  automationRunning = true;
  automationStartTime = millis();
  currentComponentIndex = 0;
  digitalWrite(componentPins[0], LOW); 
  componentStates[0] = true;
  digitalWrite(ledPins[0], HIGH);
  Serial.println(getTimestamp() + " WATER PUMP CYCLE STARTED");
  updateDisplay("WATER PUMP", "RUNNING (15 SEC)");
}

void stopAutomation() {
  Serial.println(getTimestamp() + " Stopping automation sequence...");
  automationRunning = false;
  for (int i = 0; i < 3; i++) {
    digitalWrite(componentPins[i], HIGH);
    componentStates[i] = false;
    digitalWrite(ledPins[i], LOW);
  }
  currentComponentIndex = -1;
  updateDisplay("AUTOMATION", "STOPPED");
}

void toggleComponent(int index) {
  componentStates[index] = !componentStates[index];
  digitalWrite(componentPins[index], componentStates[index] ? LOW : HIGH);
  digitalWrite(ledPins[index], componentStates[index] ? HIGH : LOW);

  Serial.println(getTimestamp() + " " + componentNames[index] + " " + (componentStates[index] ? "TURNED ON" : "TURNED OFF"));
  updateDisplay(componentNames[index], componentStates[index] ? "RUNNING" : "STOPPED");
}

void handleAutomationSequence() {
  unsigned long elapsedTime = millis() - automationStartTime;

  // Water pump phase with suction pump
  if (currentComponentIndex == 0) {
    if (elapsedTime >= waterPumpOnTime) {
      digitalWrite(componentPins[0], HIGH); // Turn off water pump
      componentStates[0] = false;
      digitalWrite(ledPins[0], LOW);
      Serial.println(getTimestamp() + " WATER PUMP CYCLE FINISHED");

      if (componentStates[1]) { // If suction is still running, move to suction-only phase
        currentComponentIndex = 2;
        automationStartTime = millis(); // Reset timer for suction pump
        playComponentSwitchingSound();
        updateDisplay("SUCTION PUMP", "RUNNING (25 SEC)");
      } else { // Otherwise, move to blower phase
        currentComponentIndex = 3;
        automationStartTime = millis(); // Reset timer for blower
        playComponentSwitchingSound();
        updateDisplay("BLOWER", "RUNNING (40 SEC)");
      }
    } else if (elapsedTime >= suctionPumpDelay && !componentStates[1]) { // Start suction pump after delay
      digitalWrite(componentPins[1], LOW); // Turn on suction pump
      componentStates[1] = true;
      digitalWrite(ledPins[1], HIGH);
      Serial.println(getTimestamp() + " SUCTION PUMP STARTED");
      playComponentSwitchingSound();
      updateDisplay("WATER &", "SUCTION RUNNING");
    }
  }
  // Blower phase
  else if (currentComponentIndex == 2 && elapsedTime >= suctionPumpOnTime) {
    digitalWrite(componentPins[1], HIGH); // Turn off suction pump
    componentStates[1] = false;
    digitalWrite(ledPins[1], LOW);
    Serial.println(getTimestamp() + " SUCTION PUMP CYCLE FINISHED");

    currentComponentIndex = 3;
    automationStartTime = millis();
    playComponentSwitchingSound();
    updateDisplay("BLOWER", "RUNNING (40 SEC)");
  }
  // Automation completion
  else if (currentComponentIndex == 3 && elapsedTime >= blowerOnTime) {
    stopAutomation();
    playAutomationCompletionSound();
    Serial.println(getTimestamp() + " AUTOMATION CYCLE COMPLETE");
  }

  updateAutomationDisplay();
}

void updateAutomationDisplay() {
  String line1, line2;

  switch (currentComponentIndex) {
    case 0:
      line1 = "WATER PUMP";
      line2 = componentStates[1] ? "SUCTION PUMP ON" : "RUNNING (15 SEC)";
      break;
    case 2:
      line1 = "SUCTION PUMP";
      line2 = "RUNNING (25 SEC)";
      break;
    case 3:
      line1 = "BLOWER";
      line2 = "RUNNING (40 SEC)";
      break;
    default:
      line1 = "AUTOMATION";
      line2 = "RUNNING...";
      break;
  }

  updateDisplay(line1, line2);
}

bool anyComponentRunning() {
  for (int i = 0; i < 3; i++) {
    if (componentStates[i]) return true;
  }
  return false;
}

void updateDisplay(const String &line1, const String &line2) {
  if (line1 != lastLine1 || line2 != lastLine2) { 
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);

    lastLine1 = line1;
    lastLine2 = line2;
  }
}

void updateScreen() {
  switch (currentScreen) {
    case 0:
      displayCenteredText("WELCOME TO", "AUTO SANITATION");
      break;
    case 1:
      displayCenteredText("PRESS ANY:", "BUTTON TO START");
      break;
    case 2:
      displayComponentStatus();
      break;
  }
}

void displayCenteredText(const String &line1, const String &line2) {
  lcd.clear();
  lcd.setCursor((LCD_COLUMNS - line1.length()) / 2, 0);
  lcd.print(line1);
  lcd.setCursor((LCD_COLUMNS - line2.length()) / 2, 1);
  lcd.print(line2);
}

void displayComponentStatus() {
  String line1, line2;

  line1 = componentStates[0] ? "WATER PUMP: ON" : "WATER PUMP: OFF";
  line2 = componentStates[1] ? "SUCTION PUMP: ON" : "SUCTION PUMP: OFF";

  updateDisplay(line1, line2);
}

void checkForErrors() {
}

void handleSerialCommand(String command) {
}

String getTimestamp() {
  unsigned long currentTime = millis() - automationStartTime;
  unsigned long seconds = (currentTime / 1000) % 60;
  unsigned long minutes = (currentTime / 60000) % 60;
  return String(minutes) + ":" + String(seconds);
}
