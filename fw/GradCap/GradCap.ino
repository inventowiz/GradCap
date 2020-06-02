#include <Adafruit_NeoPixel.h>
#include <avr/pgmspace.h>

const PROGMEM uint8_t MAP_blockS[] = {2,3,5,6,7,8,12,13,14,15,16,18,19,20,22,23,24,25,27,30,31,32,33,34,35,38,41,42,43,44,45,46,49,52,53,54,55,56,57,58,60,63,64,65,67,68,69,70,71,73,74,75,79,80,82,83,84,85};
const PROGMEM uint8_t MAP_blockGO[] = {1,2,3,7,8,9,11,15,17,21,22,28,32,33,37,43,44,47,48,50,54,55,59,61,65,66,70,72,76,78,79,80,84,85,86};
const PROGMEM uint8_t MAP_blockMSU[] = {0,2,4,5,6,8,10,11,13,15,17,19,20,21,22,24,26,30,32,33,35,37,38,39,41,43,44,46,50,52,54,55,57,59,63,65,66,68,70,72,74,76,77,79,81,82,83,85,87};


#define PIN 0
#define BTN 3

volatile char state = 0;
volatile char count = 0;
volatile uint32_t last = 0;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(88, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  pinMode(BTN,INPUT);
  
  GIMSK = 0b00100000; // enables PCINT
  PCMSK = 0b00001000; // look on pin 3
  
  // white
  for(int i=0;i<strip.numPixels();i++){
    strip.setPixelColor(i,50,50,50);
  }
  strip.show();
  
  while(millis()<4000){
    strip.setBrightness(map(millis(),0,4000,255,0));
    strip.show();
  }
  strip.setBrightness(255);
}

void loop() {
  if(state){
    // Disp S, constant for less disctraction
    for(int i=0;i<strip.numPixels();i++){
      if(binaryExists(i,MAP_blockS,52))
        strip.setPixelColor(i,26,69,60);
      else
        strip.setPixelColor(i,200,200,200);
      strip.show();
      delay(2);
    }
  }else{
    colorWipe(strip.Color(26,69,60), 10,0);
    
    // Disp S
    for(int i=0;i<strip.numPixels();i++){
      if(binaryExists(i,MAP_blockS,52))
        strip.setPixelColor(i,26,69,60);
      else
        strip.setPixelColor(i,200,200,200);
      strip.show();
      delay(2);
    }
    delay(3000);
    
    colorWipe(strip.Color(26,69,60), 10,1);
    
    // Disp GO 
    for(int i=0;i<strip.numPixels();i++){
      if(binaryExists(i,MAP_blockGO,52))
        strip.setPixelColor(i,26,69,60);
      else
        strip.setPixelColor(i,200,200,200);
      strip.show();
      delay(2);
    }
    delay(3000);
    
    // Disp MSU
    for(int i=0;i<strip.numPixels();i++){
      if(binaryExists(i,MAP_blockMSU,52))
        strip.setPixelColor(i,26,69,60);
      else
        strip.setPixelColor(i,200,200,200);
      strip.show();
      delay(2);
    }
    delay(3000);
  
    for(int i=0;i<2;i++){
      // Chase MSU
      theaterChase(strip.Color(26,69,60),strip.Color(200,200,200), 25);
      
      // Wipe Maize/Blue 2x
      for(int i=0;i<2;i++){
        colorWipe(strip.Color(26,69,60), 10,0);
        colorWipe(strip.Color(200,200,200), 10,1);
      }
    }
  }
}

ISR(PCINT0_vect){
  if(!digitalRead(BTN))
    return; //falling edge
    
  // toggle state
  state = !state;
  
  // if successive less than 1sec
  if((millis() - last) < 1000)
    count++;
  // otherwise if last hasn't been seen in >2sec
  else if((millis() - last) > 2000)
    count = 1;
  
  // if we see more than 3, reset
  if( count > 2)
    asm volatile("rjmp 0");
    
  last = millis();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait,uint8_t dir) {
  if(dir){
    uint16_t i=(strip.numPixels());
    while((i--)!=0) {
        strip.setPixelColor(i, c);
        strip.show();
        delay(wait);
    }
  }else{
    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
        strip.show();
        delay(wait);
    }
  }
}

boolean binaryExists(uint8_t needle,const uint8_t *haystack,const uint8_t haystackLen){
   int lower = 0;
   int upper = haystackLen-1;
   int middle;
   while(lower <= upper){
      middle = (lower+upper)/2;
      if(pgm_read_byte_near(haystack+middle) == needle){
        return true;
      }else if(needle < pgm_read_byte_near(haystack+middle)){
        upper = middle - 1;
      }else{
        lower = middle + 1;
      }
   }
   return false;
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c,uint32_t c2, uint8_t wait) {
  for (int j=0; j<40; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        if(binaryExists(i+q,MAP_blockM,52))
          strip.setPixelColor(i+q, c);    //turn every third pixel on
        else
          strip.setPixelColor(i+q, c2);
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}
