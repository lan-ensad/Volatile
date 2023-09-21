#include <AccelStepper.h>
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

#define DFRX D8
#define DFTX D4
#define STEP 3
#define DIR 4
#define EN 21
#define BTN 10
#define SLIDER 9
#define PINLED 7
#define FPSerial Serial1

Blinker led = Blinker(PINLED, 153, 153);
AccelStepper stepper(AccelStepper::DRIVER, STEP, DIR);
DFRobotDFPlayerMini myDFPlayer;

void printDetail(uint8_t type, int value);

bool btnState, sliderState, settedUp, routine, home, down, playing = false;
int countStep = 0;
int targetStep = 0;
uint8_t delSteps[] = { 3, 31 }; //MILISS
uint8_t delHome[] = { 2, 3 };   //SEC
uint8_t delDown[] = { 5, 7 };  //SEC

void setup() {
  FPSerial.begin(9600, SERIAL_8N1, DFRX, DFTX);
  FPSerial.begin(9600);
  Serial.begin(115200);

  pinMode(EN, OUTPUT);
  digitalWrite(EN, LOW);
  pinMode(BTN, INPUT_PULLUP);
  pinMode(SLIDER, INPUT_PULLUP);
  pinMode(PINLED, OUTPUT);

  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);
  stepper.moveTo(0);

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(FPSerial, true, true)) {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true) {
      delay(0);
    }
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
}

void loop() {
  btnState = digitalRead(BTN);
  sliderState = digitalRead(SLIDER);

  //=================================
  //          SETTING UP
  if (!sliderState) {
    led.check();
    if (!btnState) {
      settedUp = true;
      if (stepper.distanceToGo() == 0) {
        countStep += 10;
        stepper.moveTo(countStep);
        stepper.setSpeed(500);
      }
      stepper.runSpeedToPosition();
    }
  }
  //=================================
  //       GO BACK 0 POSITION
  else if (sliderState && settedUp) {
    digitalWrite(PINLED, LOW);
    if (!routine) {
      if (stepper.distanceToGo() <= 9) {
        stepper.moveTo(0);
        stepper.setSpeed(500);
      }
      stepper.runSpeedToPosition();
      if (stepper.currentPosition() == 0) {
        routine = true;
        home = true;
        delay(500);
        myDFPlayer.loop(1);
      }
    }
    //=================================
    //        GO FOR ROUTINE
    else if (routine) {
      //=================================
      //            GO DOWN
      if (home) {
        if (!playing) {
          myDFPlayer.start();
          playing = true;
        }
        if (stepper.distanceToGo() == 0) {
          targetStep += int(random(5, 19));
          stepper.moveTo(targetStep);
          stepper.setSpeed(int(random(13, 51)));
          delay(random(delSteps[0], delSteps[1]));
        }
        if (stepper.currentPosition() >= countStep) {
          home = false;
          down = true;
          myDFPlayer.pause();
          playing = false;
          delay(random(delDown[0] * 60000, delDown[1] * 60000));
        }
        stepper.runSpeedToPosition();
      }
      //=================================
      //            GO HOME
      if (down) {
        if (!playing) {
          myDFPlayer.start();
          playing = true;
        }
        if (stepper.distanceToGo() == 0) {
          targetStep -= int(random(8, 21));
          stepper.moveTo(targetStep);
          stepper.setSpeed(int(random(13, 51)));
          delay(random(delSteps[0], delSteps[1]));
        }
        if (stepper.currentPosition() <= 0) {
          home = true;
          down = false;
          myDFPlayer.pause();
          playing = false;
          delay(random(delHome[0] * 60000, delHome[1] * 60000));
        }
        stepper.runSpeedToPosition();
      }
    }
  }
  //=================================
  delay(5);
}