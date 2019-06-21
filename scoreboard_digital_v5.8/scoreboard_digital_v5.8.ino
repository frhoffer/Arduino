#include <ShifterStr.h>
 
/* 7 Segment LED Counter - 1 Court 2 Teams & 1 Game Counter*/
/* v5.8 DIGITAL FRH 6/19/2019 */
/* Added ability to recall scores by pressing reset three times in a row */

#define ButtonNum 6 // Number of buttons

const byte ButtonSet[ButtonNum]={7,8,9,10,11,12}; // Push Button Pins connected to Digital Pins 7-12

#define courtSize 5   // This is the number of digits for the court
#define courtNum 1  // Court Number which maybe used later
 
Shifter shifterC1(courtSize, 2, 3, 4);  //Setup Shifter instance with number of digits and pin numbers
 
const int maxSample = 10;       // Number of samples to use for software button debouncing
unsigned int buttonTotal = 0;   // The Running Total
int buttonAverage = 0;          // The Button Average

int resetCounter = 0;           // Count the number of reset buttons(once - scores, twice - scores and game, thrice - recall scores
  
// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long debounceDelay = 300;    // the debounce time; increase if the output flickers
long lastDebounceTime = 0;  // the last time the output pin was toggled
boolean justPressed = false; // was a button just pressed
 
 
// homeScore,awayScore,gameCount  get initialized in the array
// When printing the digits out homeScore,gameCount,awayScore (because wiring was easier)
// Store Scores in a Recall Array in case the score accidentally resets

int CourtInfo[3] = {0,0,1};
int RecallInfo[3] = {0,0,1};
	 
void setup() {
  Serial.begin(9600);
 
   //set pins to input so you can control the score
   for (int thisButton = 0; thisButton < ButtonNum; thisButton++) {
		pinMode(ButtonSet[thisButton], INPUT_PULLUP);
	}
	
  // initialize the leds
 
   shifterC1.clear();
   delay(1500);
   shifterC1.test();

   Serial.println("Scoreboard Initialization Complete");
   
   // print out the initial scoreboard
   display_scores();
}
 
int setCounterPlus(int countValue, int numPlaces){
    int placesIndex;
    int numDivisor=1;
    // Figure out the threshold for variable 1 digit = 10, 2 digits = 100, etc.
	// This allows the counter to roll over to 0 from 9, or 00 from 99.
    for (placesIndex = 1; placesIndex <= numPlaces; placesIndex++) {
     numDivisor = numDivisor*10;
    }            
    countValue++;
    countValue %= numDivisor;
    return countValue;
}

int setCounterMinus(int countValue){
  if(countValue==0){
    countValue = 0;
  }
  else{
    countValue--;
  }
  return countValue;
}
 
byte check_buttons(){
  byte buttonResult=0;
	for (int thisButton = 0; thisButton < ButtonNum; thisButton++) {
		if (digitalRead(ButtonSet[thisButton]) == LOW) {
			buttonResult = thisButton+1;
		}
	}
	return buttonResult;
}

void boxscore_serial_output() {
	Serial.print(" - Court:");
	Serial.print(courtNum, DEC);
	Serial.print(" - Game:");
	Serial.print(CourtInfo[2], DEC);
	Serial.print(" - Home:");
	Serial.print(CourtInfo[0], DEC);
	Serial.print(" - Away:");
	Serial.println(CourtInfo[1], DEC);
}

void display_scores() {
 char courtResults[courtSize+1];
  sprintf (courtResults,"%02d%d%02d",CourtInfo[0],CourtInfo[2],CourtInfo[1]);
  shifterC1.display(courtResults);
  boxscore_serial_output();
}    

void store_scores() {
 // Not the best way to set them but if it grows, I will change it
 RecallInfo[0] = CourtInfo[0];
 RecallInfo[1] = CourtInfo[1];
 RecallInfo[2] = CourtInfo[2];
}    

void recall_scores() {
 // Not the best way to set them but if it grows, I will change it
 CourtInfo[0] = RecallInfo[0];
 CourtInfo[1] = RecallInfo[1];
 CourtInfo[2] = RecallInfo[2];
 }
   
void make_updates() {
   // Store current scores in the Recall Array; Display the Current Scores; and reeset the ResetCounter;
    store_scores();
    display_scores();
    resetCounter=0; 
 }  
 
void update_scores(byte buttonPressed) {
 if (buttonPressed > 0) {
    Serial.print("Button:");
    Serial.print(buttonPressed,DEC);
    switch(buttonPressed){
    case 1: 
		CourtInfo[0]=setCounterMinus(CourtInfo[0]);
    make_updates();
	break;
    case 2: 
		CourtInfo[0]=setCounterPlus(CourtInfo[0],2);
    make_updates();
	break;
    case 3: 
		CourtInfo[1]=setCounterMinus(CourtInfo[1]);
    make_updates();
		break;
    case 4: 
		CourtInfo[1]=setCounterPlus(CourtInfo[1],2);
    make_updates();
		break;
    case 5: 
		CourtInfo[2]=setCounterPlus(CourtInfo[2],1);
    make_updates();
		break;
    case 6:
    // reset button was pressed. just reset scores as first step
    // if this is the first time, increment game counter
    // if this is the second press in a row, reset game counter to 1
    // if this is the third time in a row, set everything to recall array values.
		CourtInfo[0]=0; //home score
		CourtInfo[1]=0; //away score
		if (resetCounter == 0) {
			CourtInfo[2]=setCounterPlus(CourtInfo[2],1);
      resetCounter++;
		} else if (resetCounter == 1) {
			CourtInfo[2]=1;
      resetCounter++;
		} else {
      recall_scores();
      resetCounter=0;
		}
		display_scores();
		break;
    }
  }
}

 
void loop()
{
byte currentButtonState;
byte sampleSize;

// read the state of the switches into a local variable
currentButtonState = check_buttons();
                
if ((justPressed == false) && (currentButtonState != 0)) {
  // Time to debounce the buttons
  buttonTotal = currentButtonState;
  sampleSize = 1;

  for (int sampleCounter = 0; sampleCounter < maxSample; sampleCounter++) {
    // Put in a very small delay to help normalize the button determination
    delay(5);
    // Read the button and check what button we think it is based upon the reading.
    currentButtonState = check_buttons();
    if (currentButtonState > 0) {
      sampleSize++;
      buttonTotal+=currentButtonState;
    }
    delay(1);
  }
  buttonAverage = buttonTotal / sampleSize;
      
  update_scores(buttonAverage);
  
  // delay for the next button press; helps with button accuracy
  delay(300);
  lastDebounceTime = millis();
  justPressed = true;
} else if ((justPressed == true) && ((millis() - lastDebounceTime) > debounceDelay)) {
  justPressed = false;
}
}
 
