#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// Use pins 11 and 10 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 11; // Connects to module's RX  (620 ohm resistor in series with RX)
static const uint8_t PIN_MP3_RX = 10; // Connects to module's TX
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

// Create the Player object
DFRobotDFPlayerMini myDFPlayer;

// Constants and pin Definitions
int dialHasFinishedRotatingAfterMs = 100;
int debounceDelay = 10;

int rotaryPin= 3;
int phoneSwitchPin = 2;
int dialToneTrack = 11; // Track on SD card to use as dialtone

// Rotary State Variables
int cycleDone = 0;
int count;
int sel;
int lastState = 0;
int rotarySignal; // raw reading before debouncing
int trueState = 0; //stable state of rotary input
long lastStateChangeTime = 0;

// Phone Switch State Variables
int phoneUp = 0;
int phoneSwitchState = 0;
int lastPhoneSwitchState = 0;


void setup() {
    pinMode(rotaryPin, INPUT_PULLUP);                      
    pinMode(phoneSwitchPin, INPUT_PULLUP);
    Serial.begin(9600); //for printing to serial monitor
    softwareSerial.begin(9600);

    ///Initialize DFPlayer
    if (!myDFPlayer.begin(softwareSerial, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
        Serial.println("DFPLayer Failure");
        while(true);
    }
    Serial.println("DFPlayer online");

    myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
    myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);  // Equalize volume
}

void loop() {
    //Read all states
    phoneSwitchState = digitalRead(phoneSwitchPin);
    rotarySignal = digitalRead(rotaryPin);

    if (phoneSwitchState != lastPhoneSwitchState) { // Change detected in phone switch

        if (phoneSwitchState == HIGH) { //If phone has been picked up, play dialtone
            Serial.println("PHONE PICKED UP");
            myDFPlayer.playMp3Folder(dialToneTrack);
            myDFPlayer.start();

        } else {  //If phone has been put down, pause dialtone
            Serial.println("PHONE PUT DOWN");
            myDFPlayer.pause();
        }
    }

    /// Continue if no change in Phone Switch

    if ((millis() - lastStateChangeTime) > dialHasFinishedRotatingAfterMs) { // dial has finished rotating

        if (cycleDone) {
            sel = count % 10; // (%10 to get 0-9)
            myDFPlayer.play(sel+1); // play track (1-10) corresponding to number dialed

            Serial.println(String(sel) +  " dialed");
            Serial.println("Playing track: " + String(sel+1));
           
            //reset
            cycleDone = 0;
            count = 0;
           
        }
    }

    if (rotarySignal != lastState) {
        lastStateChangeTime = millis();
    }
    if ((millis() - lastStateChangeTime) > debounceDelay) { // debounce rotary switch
        if (rotarySignal != trueState) {
            trueState = rotarySignal; //input signal is stable
            if (trueState == HIGH) {
                count++;
                cycleDone = 1;
            }
        }
    }

    /// Update last states
    lastPhoneSwitchState = phoneSwitchState;
    lastState = rotarySignal;  
    delay(debounceDelay);
}
