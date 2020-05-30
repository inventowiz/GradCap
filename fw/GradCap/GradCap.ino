#include <Adafruit_NeoPixel.h>
#include <avr/pgmspace.h>

const PROGMEM uint8_t MAP_blockM[] = {0,1,2,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,30,31,32,38,39,40,47,48,49,55,56,57,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,85,86,87};


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
    // Disp M
    for(int i=0;i<strip.numPixels();i++){
      if(binaryExists(i,MAP_blockM,52))
        strip.setPixelColor(i,255, 203, 5);
      else
        strip.setPixelColor(i,0, 39, 50);
      strip.show();
      delay(2);
    }
  }else{
    colorWipe(strip.Color(255, 203, 5), 10,0); // Wipe Maize
    
    // Disp M
    for(int i=0;i<strip.numPixels();i++){
      if(binaryExists(i,MAP_blockM,52))
        strip.setPixelColor(i,255, 203, 5);
      else
        strip.setPixelColor(i,0, 39, 50);
      strip.show();
      delay(2);
    }
    delay(3000);
    
    colorWipe(strip.Color(0, 39, 50), 10,1); // Wipe Blue
    
    // Disp M
    for(int i=0;i<strip.numPixels();i++){
      if(binaryExists(i,MAP_blockM,52))
        strip.setPixelColor(i,255, 203, 5);
      else
        strip.setPixelColor(i,0, 39, 50);
      strip.show();
      delay(2);
    }
    delay(3000);
  
    for(int i=0;i<2;i++){
      // Chase M
      theaterChase(strip.Color(255, 203, 5),strip.Color(0,  39,  50), 25); // Maize
      
      // Wipe Maize/Blue 2x
      for(int i=0;i<2;i++){
        colorWipe(strip.Color(255, 203, 5), 10,0); // Maize
        colorWipe(strip.Color(0, 39, 50), 10,1); // Blue
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
