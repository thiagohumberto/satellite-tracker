#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// display settings
#define I2C_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2
bool display_c = true; // Display commutation
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLS, LCD_ROWS);

// motor settings
const int stepsPerRevolution = 200;
const float degreesPerStep = 1.8;
int stepCount = 0;
int dirStep = 1;
int currentPosition = 0;
int currentPositionElevation = 0;
const int StepX = 2;
const int DirX = 5;
const int StepY = 3;
const int DirY = 6;

const int resetButtonPin = 53;

// Init
void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 1);
  lcd.print("Waiting for data");
  Serial.begin(9600);
  pinMode(StepX,OUTPUT);
  pinMode(DirX,OUTPUT);
  pinMode(StepY,OUTPUT);
  pinMode(DirY,OUTPUT);
  pinMode(resetButtonPin, INPUT_PULLUP);
  delay(2000);
}

// Reset Position
void reset() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Reseting...");
  moveTo(360,0);
  stepCount = 0;
  dirStep = 1;
  currentPosition = 0;
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("Waiting for data");
}

// Move to Azymuth
void moveTo(int az, int el) {
  if(el < 0) el = 0; // out of view

  Serial.print("Moving to angle: ");
  Serial.println(az);

  int stepsToMove = az/degreesPerStep;
  int stepsToMoveElevation = el/degreesPerStep;
 
  // Calculate number of steps to move
  int stepsRemaining = abs(stepsToMove - currentPosition);
  int stepsRemainingElevation = abs(stepsToMoveElevation - currentPositionElevation);

  int clock = 1;
  int clockElevation = 1;
  
  if(az > currentPosition*degreesPerStep) {
    digitalWrite(DirX, HIGH); // set direction, HIGH for clockwise
    clock = 1;
  } else {
    digitalWrite(DirX, LOW); // set direction, LOW for anticlockwise
    clock = 0;
  }

  if(el > currentPositionElevation*degreesPerStep) {
    digitalWrite(DirY, LOW); // set direction, HIGH for clockwise
    clockElevation = 1;
  } else {
    digitalWrite(DirY, HIGH); // set direction, LOW for anticlockwise
    clockElevation = 0;
  }

  Serial.print("Steps remaining: ");
  Serial.println(stepsRemaining);

  // Azymyth: Move the motor until all steps are completed 
  while (stepsRemaining > 0) {
    digitalWrite(StepX,HIGH);
    delayMicroseconds(4000);
    digitalWrite(StepX,LOW); 
    delayMicroseconds(4000);
    if( clock == 1 ) currentPosition += 1; else currentPosition -= 1;
    stepsRemaining--;
    delay(10);  // Adjust delay as necessary for your motor
  }

  // Elevationh: Move the motor until all steps are completed 
  while (stepsRemainingElevation > 0) {
    digitalWrite(StepY,HIGH);
    delayMicroseconds(4000);
    digitalWrite(StepY,LOW); 
    delayMicroseconds(4000);
    if (clockElevation == 1) currentPositionElevation += 1; else currentPositionElevation -= 1;
    stepsRemainingElevation--;
    delay(10);  // Adjust delay as necessary for your motor
  }

  if(currentPosition==200) currentPosition = 0;
  if(currentPosition>200) currentPosition = 200-currentPosition;

  delayMicroseconds(200);
}

// Print on LCD: Sat name, Azymuth, Elevation, down and up freq.
void printLCD(String data) {
  
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print("NAME    AZ   EL"); // adjust space between words
  
  int pos1 = data.indexOf(' ');
  String name = data.substring(0, pos1);
  name.replace("SN","");
  int pos2 = data.indexOf(' ', pos1 + 1);
  String az = data.substring(pos1 + 1, pos2);
  int pos3 = data.indexOf(' ', pos2 + 1);
  String el = data.substring(pos2 + 1, pos3); 
  az.trim();
  el.trim();
  int az_number = (int) (az.substring(2).toFloat());
  int el_number = (int) (el.substring(2).toFloat());

  moveTo(az_number, el_number); // move to az

 if(display_c) {
    lcd.setCursor(0, 1);
    lcd.print(name);  
    lcd.setCursor(7, 1);
    lcd.print(" "); //aways truncate the last sat name char, in order to avoid mess with Az
    lcd.setCursor(8, 1);
    lcd.print(az_number);
    lcd.setCursor(12, 1);
    lcd.print(el_number);
  } else {
    int posDN = data.indexOf("DN");
    int posUP = data.indexOf("UP");
    int posDM = data.indexOf("DM");
    String downFreq = data.substring(posDN + 2,posUP);
    String upFreq = data.substring(posUP + 2,posDM);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DN: "+ downFreq.substring(0,6));
    lcd.setCursor(0, 1);
    lcd.print("UP: "+ upFreq.substring(0,6));
  }
  display_c = !display_c;
}

void loop() {
  
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    printLCD(data);
  }

  if (digitalRead(resetButtonPin) == LOW) reset(); 

}
