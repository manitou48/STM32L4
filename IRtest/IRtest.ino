/*
  IRtest
     hack of maple version
      http://www.arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
    use PWM to generate mark/space at 38Khz
     IR LED xmit   pin to 100ohm to +IR  - to grnd (short/flat)
    use timer 50us to count mark/space on input
    it's possible to run both xmit and recv, loopback
    stm32l4 timers 16/17 are IR timer, but we'll use old IR style
    TestPin for feedback tests, jumper to RecvPin (don't hook PWMPin)
*/
#include "Arduino.h"
#include "stm32l4_wiring_private.h"
#include "IRremote.h"

#define RecvPin 4
#define PWMPin 3
#define TestPin 12
#define LEDPin 13

static stm32l4_timer_t mytimer;


volatile unsigned long myticks;
int rawbuf[RAWBUF], rawlen;
uint8_t rcvstate;
int results_decode_type; // NEC, SONY, RC5, UNKNOWN
unsigned long results_value;
int results_bits; // Number of bits in decoded value


void timer_ISR(void *context, uint32_t events) {
    // interrupt every 50us
    uint8_t irdata = digitalRead(RecvPin);   // read IR sensor
    myticks++;


  if (rawlen >= RAWBUF) {
    // Buffer overflow
    rcvstate = STATE_STOP;
  }
  switch(rcvstate) {
  case STATE_IDLE: // In the middle of a gap
    if (irdata == MARK) {
      if (myticks < GAP_TICKS) {
        // Not big enough to be a gap.
        myticks = 0;
      }
      else {
        // gap just ended, record duration and start recording transmission
        rawlen = 0;
        rawbuf[rawlen++] = myticks;
        myticks = 0;
        rcvstate = STATE_MARK;
      }
     }
    break;
  case STATE_MARK: // timing MARK
    if (irdata == SPACE) {   // MARK ended, record time
      rawbuf[rawlen++] = myticks;
      myticks = 0;
      rcvstate = STATE_SPACE;
    }
    break;
  case STATE_SPACE: // timing SPACE
    if (irdata == MARK) { // SPACE just ended, record it
      rawbuf[rawlen++] = myticks;
      myticks = 0;
      rcvstate = STATE_MARK;
    }
    else { // SPACE
      if (myticks > GAP_TICKS) {
        // big SPACE, indicates gap between codes
        // Mark current code as ready for processing
        // Switch to STOP
        // Don't reset timer; keep counting space width
        rcvstate = STATE_STOP;
      }
    }
    break;
  case STATE_STOP: // waiting, measuring gap
    if (irdata == MARK) { // reset gap timer
      myticks = 0;
    }
    break;
  }

}

// set up recv timer  50us  20khz
void enableIRIn(){
  uint32_t modulus = (stm32l4_timer_clock(&mytimer) / 20000) ;
  uint32_t scale   = 1;

  while (modulus > 65536) {
    modulus /= 2;
    scale++;
  }
  stm32l4_timer_enable(&mytimer, scale-1, modulus-1, 0, timer_ISR, NULL, TIMER_EVENT_PERIOD);
  stm32l4_timer_start(&mytimer, false);
    rcvstate = STATE_IDLE;
    rawlen = 0;
}

void irrecv_resume() {
    rcvstate = STATE_IDLE;
    rawlen = 0;
}

long decodeSony() {
  long data = 0;
  if (rawlen < 2 * SONY_BITS + 2) {
    return ERR;
  }
  int offset = 1; // Skip first space
  // Initial mark
  if (!MATCH_MARK(rawbuf[offset], SONY_HDR_MARK)) {
    return ERR;
  }
  offset++;

  while (offset + 1 < rawlen) {
    if (!MATCH_SPACE(rawbuf[offset], SONY_HDR_SPACE)) {
      break;
    }
    offset++;
    if (MATCH_MARK(rawbuf[offset], SONY_ONE_MARK)) {
      data = (data << 1) | 1;
    }
    else if (MATCH_MARK(rawbuf[offset], SONY_ZERO_MARK)) {
      data <<= 1;
    }
    else {
      return ERR;
    }
    offset++;
  }

  // Success
  results_bits = (offset - 1) / 2;
  if (results_bits < 12) {
    results_bits = 0;
    return ERR;
  }
  results_value = data;
  results_decode_type = SONY;
  return DECODED;
}

void enableIROut(int khz) {
    int freq = 1000 * khz;
    analogWriteFrequency(PWMPin,freq);
    analogWrite(PWMPin,0);  //  start PWM with low
}

void mark(int time) {
    analogWrite(PWMPin,128);  // PWM on
    digitalWrite(TestPin,LOW);          // test   invert
    delayMicroseconds(time);
}

void space(int time) {
    analogWrite(PWMPin,0);  // off
    digitalWrite(TestPin,HIGH);
    delayMicroseconds(time);
}

void sendSony(unsigned long data, int nbits) {
  enableIROut(40);
  mark(SONY_HDR_MARK);
  space(SONY_HDR_SPACE);
  data = data << (32 - nbits);
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(SONY_ONE_MARK);
      space(SONY_HDR_SPACE);
    }
    else {
      mark(SONY_ZERO_MARK);
      space(SONY_HDR_SPACE);
    }
    data <<= 1;
  }
}

void sendRaw(unsigned int buf[], int len, int khz)
{
  enableIROut(khz);
  for (int i = 0; i < len; i++) {
    if (i & 1) {
      space(buf[i]);
    }
    else {
      mark(buf[i]);
    }
  }
  space(0); // Just to be sure
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
  pinMode(LEDPin, OUTPUT);
  pinMode(TestPin, OUTPUT);
  pinMode(RecvPin, INPUT);
  analogWriteRange(PWMPin,256);  // analogWrite range 0-255
  analogWrite(PWMPin,0);
  stm32l4_timer_create(&mytimer, TIMER_INSTANCE_TIM7, STM32L4_TONE_IRQ_PRIORITY, 0);
}

void loop() {
  long sonycmd[] = {0xA9A,0x91A,0x61A}; // power 0 7
  char str[64];

  enableIRIn();
  while(true) {   
        Serial.println("xmit");
        digitalWrite(LEDPin,LOW);
        sendSony(sonycmd[0],SONY_BITS); 
        digitalWrite(LEDPin,HIGH);
        delay(6);   // let gap time grow

        if (rcvstate == STATE_STOP) {
            if (decodeSony() ) {
                sprintf(str,"sony decoded. value %0x  %d bits",results_value, results_bits);
                Serial.println(str);
            }
            sprintf(str,"rawlen %d",rawlen); Serial.println(str);

            for (int i=0; i < rawlen; i++) {
                if (i%2) Serial.print(" ");
                Serial.println(rawbuf[i]*USECPERTICK);
            }
            irrecv_resume();
        } 

        delay(2000);
  }
}
