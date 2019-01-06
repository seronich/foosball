#include <TM1638.h>

// variables 

// inputs for IR beam break detector (Left and Right)
// mapped to interrupt pins INT0 and INT1
int irL=2;
int irR=3;

// pins for goal indicator LEDs digital output D4 and D5
int goalL=6;
int goalR=7;


// pins for the TM1638 display module from DX
int clk=8;
int dio=9;
int stb0=10;

// pin for (goal) buzzer
int bz=A0;

// counter for goals
volatile int goalcounterL=0;
volatile int goalcounterR=0;
// and something to keep track of to play goal horn
int previousgoalL=0;
int previousgoalR=0;

// set up custom numbers for display
// displayDigits[0] = 63 which displays a 0
// displayDigits[1] = 6 which displays a 1
// displayDigits[2] = 91 which displays a 2...etc
// Add 128 to value to display the same number with a dp following it.
// e.g. display a 2 with 91, display a 2. with 91+128=219
byte displayDigits[] = {63,6,91,79,102,109,124,7,127,103 };

// set up placeholders for display with "-" in the middle
byte values[] = { 0,0,0,64,64,0,0,0 };

// location to display goal scores
int dispL=1;
int dispR=7;

// variable to read buttons
byte buttons=0;
byte prevbuttons=0;



// some debouncing for interrupts 
long debouncing_time = 10000; //Debouncing Time in Milliseconds - give time to remove ball without tripping beam break
volatile unsigned long last_microsL;
volatile unsigned long last_microsR;

// initialize module
TM1638 module(dio,clk,stb0);

void goalhorn(int,int);

void setup() {
// initialize pins
pinMode(irL,INPUT_PULLUP);  
pinMode(irR,INPUT_PULLUP);  

pinMode(goalL,OUTPUT);
pinMode(goalR,OUTPUT);

pinMode(bz,OUTPUT);
reset();

// attach interrupts for goal detection
attachInterrupt(digitalPinToInterrupt(irL), goalscoredL, FALLING);
attachInterrupt(digitalPinToInterrupt(irR), goalscoredR, FALLING);

}

void loop() {
buttons=module.getButtons();
if (buttons != prevbuttons) {
  prevbuttons=buttons;
  switch (buttons) {
    case (1): 
      goalcounterL--;
      if (goalcounterL<0) goalcounterL=0;
      previousgoalL=goalcounterL; // suppress goal horn
      break;
    case (2):
      goalcounterL++;
      if (goalcounterL>99) goalcounterL=99;
      previousgoalL=goalcounterL; // suppress goal horn
      break;
    case (64): 
      goalcounterR--;
      if (goalcounterR<0) goalcounterR=0;
      previousgoalR=goalcounterR; // suppress goal horn
      break;
    case (128):
      goalcounterR++;
      if (goalcounterR>99) goalcounterR=99;
      previousgoalR=goalcounterR; // suppress goal horn
      break;
    case (8):
      reset();
      break;
  }
}

updatedisplay();

if (previousgoalL != goalcounterL) {  
  updatedisplay();
  goalhorn(200,1);
  previousgoalL=goalcounterL;
} 
if (previousgoalR!= goalcounterR) {  
  updatedisplay();
  goalhorn(200,2);
  previousgoalR=goalcounterR;
} 


}


void reset() { // resets all to start mode

//note: on my module, the green and red seem to be reversed
module.setLED(TM1638_COLOR_RED, 0);  // set LED number x to red
module.setLED(TM1638_COLOR_RED, 1); // set LED number x to red
module.setLED(TM1638_COLOR_GREEN, 6);  // set LED number x to green
module.setLED(TM1638_COLOR_GREEN, 7); // set LED number x to green
module.clearDisplay();
module.setupDisplay(true,3);
goalcounterL=0;
goalcounterR=0;
previousgoalR=goalcounterR; // suppress goal horn
previousgoalL=goalcounterL; // suppress goal horn
digitalWrite(goalL,LOW);
digitalWrite(goalR,LOW);
updatedisplay();
goalhorn(50,3);
}

void updatedisplay() {
  int L1=0;
  int L2=0;
  int R1=0;
  int R2=0;
  if (goalcounterL>9) {
    if (goalcounterL > 99) { 
      goalcounterL=0; 
      L1=0;
      L2=0;
      } else {
      L1 = (int)(goalcounterL/10);
      L2 = goalcounterL-L1*10;
    }
  } else {
    L2=goalcounterL;
  }
  if (goalcounterR>9) {
    if (goalcounterR > 99) { 
      goalcounterR=0;
      R1=0;
      R2=0; 
      } else {
      R1 = (int)(goalcounterR/10);
      R2 = goalcounterR-R1*10;
    }
  } else {
    R2=goalcounterR;
  }
  values[dispL-1] = displayDigits[L1];
  values[dispL] = displayDigits[L2];
  values[dispR-1] = displayDigits[R1];  
  values[dispR] = displayDigits[R2];
  module.setDisplay(values);
}

void goalscoredL() {
  if((long)(micros() - last_microsL) >= debouncing_time * 1000) {
    goalcounterL++;
    if (goalcounterL>99) goalcounterL=99;
    last_microsL = micros();
  }
}

void goalscoredR() {
  if((long)(micros() - last_microsR) >= debouncing_time * 1000) {
    goalcounterR++;
    if (goalcounterR>99) goalcounterR=99;
    last_microsR = micros();
  }
}


void goalhorn(int standard=500,int which=3) {
  switch (which) {
    case (1):
      digitalWrite(goalL,LOW);
      break;
    case (2):
      digitalWrite(goalR,LOW);
      break;
    default:
      digitalWrite(goalL,LOW);
      digitalWrite(goalR,LOW);
      break;
  }
  for (int i=0;i<=standard;i++) {
    tone (bz,2000,5);
    delay(10);
  }
  digitalWrite(goalL,HIGH);
  digitalWrite(goalR,HIGH);
}
