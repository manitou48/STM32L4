// TIMx input capture of GPS PPS dragonfly  TIM5 @80mhz
//  pin 31 PA1 TIMx CH2 AF1  DMA1 CH7  or lib TIM5_CH2 AF2 DMA2 CH4
// test with PWM  jumper pin 40 to 31

#include "Arduino.h"
#include "stm32l4_wiring_private.h"

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
static stm32l4_timer_t mytimer;
#define TIMx TIM5
#define TIMER_INSTANCEx  TIMER_INSTANCE_TIM5
#define FREQHZ 1000

volatile uint32_t ticks, period, pulse;

void timer_callback(void *context, uint32_t events) {
  ticks++;
  period = TIMx->CCR2;
  pulse = TIMx->CCR1;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  analogWriteRange(40, 256);
  analogWriteFrequency(40, 10);  // set frequency hz
  analogWrite(40, 64); // jumper to pin 31 PA1 default 488 hz
  pinMode(31, INPUT);   // PA1

  stm32l4_timer_create(&mytimer, TIMER_INSTANCEx, STM32L4_TONE_IRQ_PRIORITY, 0);
  uint32_t modulus = (stm32l4_timer_clock(&mytimer) / FREQHZ) ;
  uint32_t scale   = 1;

  while (modulus > 65536) {
    modulus /= 2;
    scale++;
  }
  //stm32l4_timer_enable(&mytimer, scale - 1, modulus - 1, 0, NULL, NULL, 0);
  stm32l4_timer_enable(&mytimer, scale - 1, modulus - 1, 0, timer_callback, NULL, TIMER_EVENT_CHANNEL_2);
  //stm32l4_timer_channel(&mytimer, TIMER_CHANNEL_2, 0, capturestuff);
  PRREG(TIMx->CR1);
  PRREG(GPIOA->MODER);
  PRREG(GPIOA->AFR[0]);

  TIMx->CR1 = 0x80;
  TIMx->PSC = 0;    // hack to 1mhz
  TIMx->ARR = 0xffffffff;
  GPIOA->MODER |= 2 << 2; // AF mode for PA1

  TIMx->DIER = 4;
  TIMx->CCMR1 = 0x102;   // CCER must be clear
  TIMx->CCER = 0x13;
  TIMx->SMCR = 0x8064;
  TIMx->SR = 0;
  stm32l4_timer_start(&mytimer, false);

#if 1
  PRREG(TIMx->CR1);
  PRREG(TIMx->CR2);
  PRREG(TIMx->CCER);
  PRREG(TIMx->CCMR1);   // channels 1 and 2
  PRREG(TIMx->PSC);
  PRREG(TIMx->DIER);
  PRREG(TIMx->ARR);
  PRREG(TIMx->SMCR);
  PRREG(TIMx->SR);
  PRREG(GPIOA->MODER);
  PRREG(GPIOA->AFR[0]);
#endif
}

void loop() {
  if (ticks) {
    uint32_t period = TIMx->CCR2;
    int delta = (int)period - 80000000;
    float ppm = delta / 80.;
    ticks = 0;
    Serial.print("period "); Serial.print(period);
    Serial.print("  duty: "); Serial.print(TIMx->CCR1);
    Serial.print("  ppm: "); Serial.println(ppm, 3);
  }
}
