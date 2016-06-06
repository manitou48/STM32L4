// adcdma  timer trigger ADC DMA   TODO
//  ADC timer triggers TRGO table 88
// variations: circular with ping-pong, or ADC triggered DMA
//    ADC1 DMA trigger DMA1/ch1 (REQ0)  or DMA2/ch3(REQ2)


#include "Arduino.h"
#include "wiring_private.h"

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

volatile uint32_t ticks;
void timer_isr(void *context, uint32_t events) {
  ticks++;
}



static stm32l4_timer_t mytimer;

#define DMA_OPTIONS (DMA_OPTION_PERIPHERAL_TO_MEMORY | DMA_OPTION_MEMORY_DATA_SIZE_16  | DMA_OPTION_PERIPHERAL_DATA_SIZE_16 | DMA_OPTION_MEMORY_DATA_INCREMENT )

stm32l4_dma_t dma;

#define FREQHZ 1000

#define ADC_SAMPLE_TIME_2_5    0
#define ADC_SAMPLE_TIME_6_5    1
#define ADC_SAMPLE_TIME_12_5   2
#define ADC_SAMPLE_TIME_24_5   3
#define ADC_SAMPLE_TIME_47_5   4
#define ADC_SAMPLE_TIME_92_5   5
#define ADC_SAMPLE_TIME_247_5  6
#define ADC_SAMPLE_TIME_640_5  7

uint16_t adcbuf[16];   //  ADC DMA buffer
uint32_t t1,t2;


void initADC(int pin) {
  int channel = g_APinDescription[pin].adc_input;
  // configure
  stm32l4_system_periph_enable(SYSTEM_PERIPH_ADC);  
  ADC1->CR &= ~ADC_CR_DEEPPWD;
  ADC1->CR |= ADC_CR_ADVREGEN;
  armv7m_clock_spin(20000);

  ADC1->ISR = ADC_ISR_ADRDY;
  ADC1->CR |= ADC_CR_ADEN;
  while (!(ADC1->ISR & ADC_ISR_ADRDY));
  // configure:  select TIM2 TRGO rising external event
  ADC1->CFGR = ADC_CFGR_OVRMOD | ADC_CFGR_JQDIS | ADC_CFGR_EXTEN_0 | ADC_CFGR_EXTSEL_3 | ADC_CFGR_EXTSEL_1 |  ADC_CFGR_EXTSEL_0 | ADC_CFGR_DMAEN;


  stm32l4_gpio_pin_configure(g_APinDescription[pin].pin, (GPIO_PUPD_NONE | GPIO_MODE_ANALOG | GPIO_ANALOG_SWITCH));
    uint32_t adc_smp = ADC_SAMPLE_TIME_47_5;
    ADC1->SQR1 = (channel << 6);
    ADC1->SMPR1 = (channel < 10) ? (adc_smp << (channel * 3)) : 0;
    ADC1->SMPR2 = (channel >= 10) ? (adc_smp << ((channel * 3) - 30)) : 0;

//  ? start and discard ?
    ADC1->ISR = ADC_ISR_EOC;
    ADC1->CR |= ADC_CR_ADSTART;
#if 0
    while (!(ADC1->ISR & ADC_ISR_EOC));   // wait
    ADC1->DR & ADC_DR_RDATA;   // discard
#endif

   
 #if 1
  PRREG(ADC1->SQR1);
  PRREG(ADC1->SMPR1);
  PRREG(ADC1->ISR);
  PRREG(ADC1->CR);
  PRREG(ADC1->CFGR);
#endif
}

void disableADC() {
    
    // disable
    ADC1->CR |= ADC_CR_ADDIS;

    while (ADC1->CR & ADC_CR_ADEN)
    {
    }

    ADC1->CR &= ~ADC_CR_ADVREGEN;
    ADC1->CR |= ADC_CR_DEEPPWD;
    stm32l4_system_periph_disable(SYSTEM_PERIPH_ADC);
}

void readADC() {
  ADC1->CR |= ADC_CR_ADSTART;
  stm32l4_dma_start(&dma, (uint32_t)adcbuf,(uint32_t)&(ADC1->DR),  sizeof(adcbuf)/2, DMA_OPTIONS);
  while( ! stm32l4_dma_done(&dma));
}

void setup() {
  Serial.begin(9600);
  while(!Serial);

  // init ADC
  analogRead(A2);   // init ADC1
  initADC(A2);


  // init DMA
  stm32l4_dma_create(&dma, DMA_CHANNEL_DMA1_CH1_ADC1, DMA_OPTION_PRIORITY_MEDIUM);
  stm32l4_dma_enable(&dma, NULL, NULL);
  stm32l4_dma_start(&dma, (uint32_t)adcbuf,(uint32_t)&(ADC1->DR),  sizeof(adcbuf)/2, DMA_OPTIONS);

  // init timer
  stm32l4_timer_create(&mytimer, TIMER_INSTANCE_TIM2, STM32L4_TONE_IRQ_PRIORITY, 0);
  uint32_t modulus = (stm32l4_timer_clock(&mytimer) / FREQHZ) ;
  uint32_t scale   = 1;

  while (modulus > 65536) {
    modulus /= 2;
    scale++;
  }
  stm32l4_timer_enable(&mytimer, scale-1, modulus-1, 0, timer_isr, NULL, TIMER_EVENT_PERIOD);
  //stm32l4_timer_notify(&mytimer, NULL, NULL, TIMER_EVENT_PERIOD);  // DIER notify DMA

  //TIM2->DIER = TIM_DIER_UDE | TIM_DIER_UIE;
  TIM2->CR2 = 0x20;  // MMS TRGO update

  char str[64];
  sprintf(str,"%d hz  scale %d modulus %d  clockhz %d",FREQHZ,scale,modulus,stm32l4_timer_clock(&mytimer));
  Serial.println(str);

  stm32l4_timer_start(&mytimer, false);
  //  wait for DMA to complete and stop timer
  t1=micros();
  while( ! stm32l4_dma_done(&dma));
  t1=micros()-t1;
  Serial.print("us ");Serial.println(t1);
  for (int i=0;i<16;i++) Serial.println(adcbuf[i]);
  #if 1
  PRREG(ADC1->SQR1);
  PRREG(ADC1->SMPR1);
  PRREG(ADC1->ISR);
  PRREG(ADC1->CR);
  PRREG(ADC1->CFGR);
#endif
  memset(adcbuf,7,sizeof(adcbuf));
  ADC1->CR |= ADC_CR_ADSTART;
  stm32l4_dma_start(&dma, (uint32_t)adcbuf,(uint32_t)&(ADC1->DR),  sizeof(adcbuf)/2, DMA_OPTIONS);

  t1=micros();
  while( ! stm32l4_dma_done(&dma));
  t1=micros()-t1;
  Serial.print("us ");Serial.println(t1);
  Serial.println(adcbuf[3]);
}

void loop() {
  memset(adcbuf,7,sizeof(adcbuf));
  t1=micros();
  readADC();
  t1=micros()-t1;
  Serial.print("us ");Serial.println(t1);
	Serial.print(ticks); Serial.print(" ticks ");
  Serial.println(adcbuf[3]);
	delay(3000);
}
