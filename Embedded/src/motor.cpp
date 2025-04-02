#include "motor.h"

// Need to limit the length of the loop to avoid missing I2C messages
#define MAX_SPIN 500

const int DIR = 2;
const int STEP = 33;
const float  steps_per_rev = 3150;

float curr_ht = 0;
float target_ht = 0;

void setup_motor()
{
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
}

void update_target(float new_ht) {
    target_ht = new_ht;
    Serial.printf("Target Height set to %f\n", new_ht);
}

void motor_loop() {
  //set spin direction
  if(curr_ht == target_ht){
    return; // if no height change
  }
  int dir = curr_ht < target_ht; // spin up is 1

  digitalWrite(DIR, dir);
  if (dir)
    Serial.println("Spinning up...");
  else
    Serial.println("Spinning down...");

  float step = steps_per_rev / 3.25; // steps per inch of height 
  float dist = abs( curr_ht - target_ht);
  Serial.printf("Distance %f...\n", dist);

  int spin_steps = int(dist * step);
 
  int i;
  // Limit the downtime of the controller per iteration of the loop by MAX_SPIN
  for(i = 0; i < spin_steps && i < MAX_SPIN; i++)
  {
    digitalWrite(STEP, HIGH);
    delayMicroseconds(2000);
    digitalWrite(STEP, LOW);
    delayMicroseconds(2000);
  }

  // Move up
  if (dir) {
    curr_ht += dist * (float(i) / spin_steps);
  } else { // Moved Down
    curr_ht -= dist * (float(i) / spin_steps);
  }

  Serial.printf("Height set to %f\n", curr_ht);
}

