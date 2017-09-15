// TIMx DMA input capture  dragonfly
//  pin 31 PA1  lib TIM5_CH2 AF2 DMA2 CH4   TIM5 is 32 bits
// test with PWM  jumper pin 40 to 31

#include "Arduino.h"
//#include "stm32l4_timer.h"
#include "stm32l4_wiring_private.h"

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define DMA_OPTIONS (DMA_OPTION_PERIPHERAL_TO_MEMORY | DMA_OPTION_MEMORY_DATA_SIZE_32  | DMA_OPTION_PERIPHERAL_DATA_SIZE_32 | DMA_OPTION_MEMORY_DATA_INCREMENT )

stm32l4_dma_t dma;
//number of captures to do..
#define SAMPLES 128
volatile int DMAbusy = 1;          //set to 0 when dma complete.
volatile uint32_t data[SAMPLES];   //place to put the data via dma

static stm32l4_timer_t mytimer;
#define TIMx TIM5
#define TIMER_INSTANCEx  TIMER_INSTANCE_TIM5
#define FREQHZ 1000

#define CONTROL (TIMER_CONTROL_CAPTURE_RISING_EDGE)


void dma_init() {
  stm32l4_dma_create(&dma, DMA_CHANNEL_DMA2_CH4_TIM5_CH2, DMA_OPTION_PRIORITY_MEDIUM);
  stm32l4_dma_enable(&dma, NULL, NULL);
  stm32l4_dma_start(&dma, (uint32_t)data, (uint32_t) & (TIMx->CCR2),  sizeof(data) / 4, DMA_OPTIONS);
}

//dump routine to show content of captured data.  do subset of SAMPLES
void printData() {
  Serial.print("DATA: ");
  for (int i = 0; i < 10; i++) {
    Serial.print(data[i]);
    Serial.print(" ");
  }
  Serial.println("...");
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  //  analogWriteRange(40,256);
  //  analogWriteFrequency(40,1000);   // set frequency hz
  analogWrite(40, 128); // jumper to pin 31 PA1

  pinMode(31, INPUT);   // PA1
  RCC->AHB1ENR |= 3 << 21; //RCC_AHBENR_DMA1EN | RCC_AHBENR_DMA2EN;
  stm32l4_timer_create(&mytimer, TIMER_INSTANCEx, STM32L4_TONE_IRQ_PRIORITY, 0);
  uint32_t modulus = (stm32l4_timer_clock(&mytimer) / FREQHZ) ;
  uint32_t scale   = 1;

  while (modulus > 65536) {
    modulus /= 2;
    scale++;
  }
  stm32l4_timer_enable(&mytimer, scale - 1, modulus - 1, 0, NULL, NULL, 0);
  stm32l4_timer_channel(&mytimer, TIMER_CHANNEL_2, 0, CONTROL);
  PRREG(TIMx->CR1);
  char str[64];
  sprintf(str, " %d hz  scale %d modulus %d  clockhz %d", FREQHZ, scale, modulus, stm32l4_timer_clock(&mytimer));
  Serial.println(str);
  TIMx->CR1 = 0x81;
  TIMx->PSC = 79;    // hack to 1mhz
  TIMx->ARR = 0xffffffff;
  TIMx->DIER = TIM_DIER_CC2DE;   // DMA for ch2
  GPIOA->MODER |=  2 << 2; // hack PA1 into AF mode after pinMode,

  dma_init();
  stm32l4_timer_start(&mytimer, false);
  while ( ! stm32l4_dma_done(&dma));  // busy wait
  printData();

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

}
