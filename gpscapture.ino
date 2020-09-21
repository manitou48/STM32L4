// TIMx input capture of GPS PPS dragonfly  TIM5 @80mhz  period and duty
//  pin 31 PA1 TIMx CH2 AF1  DMA1 CH7  or lib TIM5_CH2 AF2 DMA2 CH4
// test with PWM  jumper pin 40 to 31
// v1 raw configure with polling

#include "Arduino.h"
#include "stm32l4_wiring_private.h"

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define TIMx TIM5
#define TIMER_INSTANCEx  TIMER_INSTANCE_TIM5

void setup() {
  Serial.begin(9600);
  while (!Serial);
  analogWriteRange(40, 256);
  analogWriteFrequency(40, 10);  // set frequency hz
  analogWrite(40, 64); // jumper to pin 31 PA1 default 488 hz
  pinMode(31, INPUT);   // PA1
  
  armv7m_atomic_or(&RCC->APB1ENR1, RCC_APB1ENR1_TIM5EN);
  PRREG(TIMx->CR1);
  PRREG(GPIOA->MODER);
  PRREG(GPIOA->AFR[0]);

  TIMx->CR1 = 0x80;
  TIMx->PSC = 0;    // hack to 80 mhz
  TIMx->ARR = 0xffffffff;
  GPIOA->MODER |= 2 << 2; // AF mode for PA1

  TIMx->CCMR1 = 0x102;   // CCER must be clear
  TIMx->CCER = 0x13;     // CC2E   CC1P CC1E
  TIMx->SMCR = 0x8064;
  TIMx->SR = 0;

  armv7m_atomic_and(&TIMx->CR1, ~TIM_CR1_OPM);
  armv7m_atomic_or(&TIMx->CR1, TIM_CR1_CEN);

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

  if (TIMx->SR & TIM_SR_CC2IF) {
    uint32_t period = TIMx->CCR2;
    uint32_t duty = TIMx->CCR1;
    TIMx->SR = 0;
    int delta = (int)period - 80000000;
    float ppm = delta / 80.;

    Serial.print("period "); Serial.print(period);
    Serial.print("  duty: "); Serial.print(duty);
    Serial.print("  ppm: "); Serial.println(ppm, 3);
  }
}
