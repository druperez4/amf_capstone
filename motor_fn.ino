const int DIR = 2;
const int STEP = 4;
const int  steps_per_rev = 3150;

float currht = 0;

void setup()
{
  Serial.begin(115200);
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  
}

void motor(float curr_ht, float new_ht){
  //set spin direction
  if(curr_ht == new_ht){
    return; // if no height change
  }
  int dir = 1; // default spin up
  if (curr_ht > new_ht){
    dir = 0;
  }
  else if (curr_ht < new_ht){
    dir = 1;
  }

  digitalWrite(DIR, dir);
  Serial.println("Spinning...");

  float step = 9.69; // 3150(.01) / 3.25 --> 3.2 in. rise
  float dist = abs( curr_ht - (new_ht / .01) );
  int spin_steps = int(dist * step);
 
  for(int i = 0; i < spin_steps; i++)
  {
    digitalWrite(STEP, HIGH);
    delayMicroseconds(2000);
    digitalWrite(STEP, LOW);
    delayMicroseconds(2000);
  }


}




void loop()
{

  float newht = 2;
  motor(currht, newht);
  currht = newht;
}
