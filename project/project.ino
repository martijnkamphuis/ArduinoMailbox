#include <avr/sleep.h>

#define SYSTEM_DELAY_BEFORE_SLEEP_SECONDS   10
#define REED_PIN                            2
#define ONBOARD_LED_PIN                     13

volatile boolean workingOnInterrupt = false;
volatile unsigned long lastInterrupt = 0l;

void setup() {
  Serial.begin(9600);
  
  pinMode(ONBOARD_LED_PIN, OUTPUT);
  pinMode(REED_PIN, INPUT_PULLUP);    // Enable internal pull-up; NOTE: always assign after attachInterrupt, because attachInterrupt might override PULLUP
}


void loop() {
  if(workingOnInterrupt) {
    Serial.println("Begining loop...");
    Serial.print("Sending message to TTN...");
    delay(1000);
    Serial.println(" done!");
    Serial.println("Going to sleep in 10 seconds...");
    
    // Wait a periode before going back to sleep
    delay(1000 * SYSTEM_DELAY_BEFORE_SLEEP_SECONDS);
    
    digitalWrite(ONBOARD_LED_PIN, LOW); // Turn the LED off
  }

  workingOnInterrupt = false;
  goToDeepSleep();
}


void magnetLossDetected() {
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(REED_PIN));   // disabled interrupts to prevent bouncing/multiple interrupts while executing

  if(millis() - lastInterrupt > 40 || millis() < lastInterrupt) {   // handle bouncing
    workingOnInterrupt = true;
    lastInterrupt = millis();

    // Code here gets executed immediately after interrupt (before returning to the 'loop' function)
    Serial.println("Magnet loss detected...");
  
    digitalWrite(ONBOARD_LED_PIN, HIGH); // Turn the LED on
  }
}


void goToDeepSleep() {
  /*
   * The 5 different modes are:
   *     SLEEP_MODE_IDLE         -the least power savings
   *     SLEEP_MODE_ADC
   *     SLEEP_MODE_PWR_SAVE
   *     SLEEP_MODE_STANDBY
   *     SLEEP_MODE_PWR_DOWN     -the most power savings
   */
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  noInterrupts();
  sleep_enable();
  setInterrupt();
  interrupts();

  sei();
  sleep_cpu();

  // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP AND HANDLING INTERRUPT
  
  Serial.println("System waking up...");
}


void setInterrupt() {
  EIFR = 0x01;    // Clear queued interrupts (mailbox closed before system sleep)
  attachInterrupt(digitalPinToInterrupt(REED_PIN), magnetLossDetected, RISING); // magnetLossDetected when pin 2 gets RAISING
}

