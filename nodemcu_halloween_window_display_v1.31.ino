/* Halloween Window Display v1, 9/29/2019 */

 
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#include "FastLED.h"

// Set up my Constants
#define NUM_LEDS 105
#define LED_SEGMENT 35
#define NUM_SEGMENTS 3
#define NUM_COLORS 5
#define NORM_DELAY 5000
#define WAIT_DELAY 500
#define QUIK_DELAY 100
#define PIN D4
#define LED_TYPE WS2813   // LED Type
#define COLOR_ORDER GRB 
// ORDER IS REALLY GREEN - RED - BLUE for WS2813
// ORDER IS RED - GREEN - BLUE for WS2811

// Define Global Variables

long randNumber;
long randRed;
long randBlue;
long randGreen;
int rPick;
CRGB leds[NUM_LEDS];


int myColors[NUM_COLORS][3] = {{255,50,0},
                       {128,0,128},
                       {0,255,0},
                       {255,0,0},
                       {0,0,255}};

void setup() {
   Serial.begin(115200);
   randomSeed(analogRead(0));

   FastLED.addLeds<LED_TYPE, PIN, COLOR_ORDER>(leds, NUM_LEDS);

   FastLED.clear();
   FastLED.show();
   delay(WAIT_DELAY);
 
}

// Has an orange pixel that chases through the entire string

void chase_thru() {
  FastLED.clear();
  for(int x = 0; x < NUM_LEDS; x++){
    leds[x] = CRGB(255,50,0);
    FastLED.show();
    leds[x] = CRGB(0,0,0);   
    delay(QUIK_DELAY); 
    }
  FastLED.clear(); 
}

// Sparkler Fun which uses sparkle routine

void sparkler(){
  FastLED.clear();
  for (int x= 0; x < 1000; x++){
    randRed = random(0, 255);
    randBlue = random(0, 255);
    randGreen = random(0, 255);
    sparkle(randRed, randGreen, randBlue, random(10));
  }
  FastLED.clear();
}
 
void sparkle(byte red, byte green, byte blue, int MyDelay) {
int pixel = random(NUM_LEDS);
  leds[pixel] = CRGB(red,green,blue);
  FastLED.show();
  delay(MyDelay);
  leds[pixel] = CRGB(0,0,0);
}

// Turns pixels off one by one from the start to the end

void wipe_clear() {
  for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(0,0,0);
      FastLED.show();
      delay(25);
    }
}

// Turns pixels off one by one on each segment at the same time from the start to the end


void wipe_clear_segment() {
  for (int i = 0; i < LED_SEGMENT; i++) {
      for (int x = 0; x < NUM_SEGMENTS; x++) {
        leds[i+(LED_SEGMENT*x)] = CRGB(0,0,0);
      }
      FastLED.show();
      delay(QUIK_DELAY);
    }
}

// Strobes all the leds red

void red_strobe() {
  byte redLow=20;
  byte redHigh=255;
  int ledCounter;
   
  FastLED.clear();
  for (int y = 0; y < 5; y++) {
    for( ledCounter = redLow ; ledCounter <= redHigh; ledCounter++ ) {
      fill_solid(&(leds[0]), NUM_LEDS, CRGB(ledCounter,0,0));
      FastLED.show();
      delay(2);
    }  
    for( ledCounter = redHigh ; ledCounter >= redLow; ledCounter-- ) {
      fill_solid(&(leds[0]), NUM_LEDS, CRGB(ledCounter,0,0));
      FastLED.show();
      delay(2);
    } 
  }
  wipe_clear();
} 

// Original Strobe but drawing too much power so created a half strobe as well

void fast_strobe() {
  for (int y = 0; y < 25; y++) { 
  FastLED.clear();
  fill_solid(&(leds[0]), NUM_LEDS, CRGB(255,255,255));
  FastLED.show();
  delay(100);
  FastLED.clear();
  FastLED.show();
  delay(100); 
  }
  delay(1000); 
}

void half_fast_strobe() {
  for (int y = 0; y < 25; y++) { 
  FastLED.clear();
  for (int i = 0; i < NUM_LEDS; i++) {
    if(i%2 == 0)
      { leds[i] = CRGB(255,255,255);
    }
  }
  FastLED.show();
  delay(100);
  FastLED.clear();
  FastLED.show();
  delay(100); 
  }
  delay(1000); 
}



void cycler() {
  for (int x = 0; x < NUM_COLORS; x++){
      fill_solid( &(leds[0]), NUM_LEDS, CRGB(myColors[x][0],myColors[x][1],myColors[x][2]));
      FastLED.show();
      delay(NORM_DELAY);
      wipe_clear_segment();
      delay(WAIT_DELAY);   
  }  
}

void rand_cycler() {
  for (int x = 0; x < NUM_COLORS; x++){
      for (int y = 0; y < NUM_SEGMENTS; y++) {
        // pick a random number between 0 and 3
        rPick = random(NUM_COLORS);
        Serial.println(rPick);
        fill_solid( &(leds[y*LED_SEGMENT]), LED_SEGMENT, CRGB(myColors[rPick][0],myColors[rPick][1],myColors[rPick][2]));
      }
      FastLED.show();
      delay(NORM_DELAY);
      wipe_clear_segment();
      delay(WAIT_DELAY);   
  }  
}

void rand_slider(){
int rColors[NUM_SEGMENTS];
int NewRandom;
// 
for (int y=0; y < NUM_SEGMENTS; y++) {
  rColors[y] = random(NUM_COLORS);
}
// initial set up of the led segments
for (int i=0; i < LED_SEGMENT; i++) {
  for (int x=0; x < NUM_SEGMENTS; x++) {
    rPick=rColors[x];
    leds[i+(LED_SEGMENT*x)] = CRGB(myColors[rPick][0],myColors[rPick][1],myColors[rPick][2]);
  }
  FastLED.show();
  delay(QUIK_DELAY);
}
delay(NORM_DELAY);
for (int z=0; z < 4; z++) {
  NewRandom = random(NUM_COLORS);
  for (int i=0; i < LED_SEGMENT; i++) {
    for (int x=0; x < NUM_SEGMENTS; x++) {
      if (x==0) {
        rPick=NewRandom;
        } else {
        rPick=rColors[x-1];
        }
      leds[i+(LED_SEGMENT*x)] = CRGB(myColors[rPick][0],myColors[rPick][1],myColors[rPick][2]);
    }
  FastLED.show();
  delay(QUIK_DELAY);
  }
  for (int v= NUM_SEGMENTS; v > 0; v--) {
    rColors[v]=rColors[v-1];
  }
  rColors[0]=NewRandom;
  delay(NORM_DELAY);
}
wipe_clear_segment();
}   



void loop() {
  rand_slider();
  red_strobe();  
  rand_cycler();
  red_strobe();
  cycler();
  half_fast_strobe();
  //chase_thru();
  //sparkler();
  delay(NORM_DELAY);
  }
