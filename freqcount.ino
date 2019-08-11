// frequency count from external pin   TODO
// use interval timer to report counts
// TIM2 and TIM5 32-bit timers
// test with PWM pin 3 jumper to 31
// TIM5_CH2 AF2 PA1 pin 31  and TIM2_CH2 AF1

#include "Arduino.h"
#include "stm32l4_wiring_private.h"

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
#define TIMx TIM5
#define TIMER_INSTANCEx  TIMER_INSTANCE_TIM5

#define RCC_LSI_ON 1

uint32_t prev;

static stm32l4_timer_t mytimer;


volatile uint32_t pulses, ticks, dataReady;;
void timer_callback(void *context, uint32_t events) {
  //  pulses = LPTIM1->CNT;
  pulses = TIMx->CNT;
  ticks++;
  if (ticks % 1000 == 0) dataReady = 1;  // every second @1khz
}

void interval_timer_init(int hz) {
  stm32l4_timer_create(&mytimer, TIMER_INSTANCE_TIM7, STM32L4_TONE_IRQ_PRIORITY, 0);
  uint32_t modulus = (stm32l4_timer_clock(&mytimer) / hz) ;
  uint32_t scale   = 1;

  while (modulus > 65536) {
    modulus /= 2;
    scale++;
  }
  stm32l4_timer_enable(&mytimer, scale - 1, modulus - 1, 0, timer_callback, NULL, TIMER_EVENT_PERIOD);
  stm32l4_timer_start(&mytimer, false);

  printf("%d hz  scale %d modulus %d  clockhz %d\n", hz, scale, modulus, stm32l4_timer_clock(&mytimer));

}

// clock timer's external pin


void timx_init() {
  //pinMode(31, INPUT); // necessary?

  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM5EN;   // power up TIM5
  TIMx->CR1 = 0x80;  // disable
  GPIOA->MODER &= ~(3 << 2); // clear PA1 mode
  GPIOA->MODER |= 2 << 2; // AF mode for PA1
  GPIOA->AFR[0] |= 2 << 4; // AF2 for PA1

  TIMx->CCMR1 = 0x100;   // CC2S(1) ch2 rising
  TIMx->CCER = 0;        //rising  pg 783
  TIMx->SMCR = 0x67;   //  TS(6) TI12 edge detection SMS(7)
  TIMx->SR = 0;
  TIMx->CR1 = 0x81;  // enable
  PRREG(TIMx->CR1);
  PRREG(GPIOA->AFR[0]);
  PRREG(TIMx->SMCR);
  PRREG(TIMx->CCMR1);
  PRREG(GPIOA->MODER);
}

// LPTIM  external pin PC0  AF1 LPTIM1_IN1  A5
void lptim_init() {
  RCC->CCIPR |= 3 << RCC_CCIPR_LPTIM1SEL_Pos;  // 0 pclk, 1 lsi 2 hsi16, 3 lse
  RCC->APB1ENR1 |= RCC_APB1ENR1_LPTIM1EN;
  RCC->CSR |= RCC_LSI_ON;   // if using LSI
  RCC->CR |= RCC_CR_HSION;   // if using HSI16
  LPTIM1->CR = 0;   // disable
  LPTIM1->CNT = 0;
  //LPTIM1->CFGR = 7<<LPTIM_CFGR_PRESC_Pos;  // prescale 1 to /128

  LPTIM1->CR =  LPTIM_CR_ENABLE;
  //  LPTIM1->CMP = 32;
  LPTIM1->CR |= LPTIM_CR_CNTSTRT;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(2000);
  Serial.println("starting");
  delay(2000);

  analogWriteRange(3, 2);  // for high speed
  analogWriteFrequency(3, 40000000);
  analogWrite(3, 1);  // duty

  interval_timer_init(1000);
  timx_init();
  // lptim_init();  // TODO
}

void loop() {
  if (dataReady) {
    // Serial.println(pulses - prev);
    printf("delta %d  %d pulses\n", pulses - prev, pulses);
    prev = pulses;
    dataReady = 0;
  }

}
