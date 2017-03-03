// dragonfly ADC test   A0 PA4 ADC_INPUT_9
// could also set continuous mode  CONT in CFGR
#include "Arduino.h"
#include "stm32l4_wiring_private.h"

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define ADC_SAMPLE_TIME_2_5    0
#define ADC_SAMPLE_TIME_6_5    1
#define ADC_SAMPLE_TIME_12_5   2
#define ADC_SAMPLE_TIME_24_5   3
#define ADC_SAMPLE_TIME_47_5   4
#define ADC_SAMPLE_TIME_92_5   5
#define ADC_SAMPLE_TIME_247_5  6
#define ADC_SAMPLE_TIME_640_5  7

int adcread() {
  int val;
    ADC1->ISR = ADC_ISR_EOC;
    ADC1->CR |= ADC_CR_ADSTART;
    while (!(ADC1->ISR & ADC_ISR_EOC))
    {
    }
    val = ADC1->DR & ADC_DR_RDATA;
    ADC1->ISR = ADC_ISR_EOC;
    return val;
}

//static stm32l4_adc_t stm32l4_adc;
void setup() {
  uint32_t t;
  int i,v;
  Serial.begin(9600);
  while(!Serial);
  delay(2000);

  analogRead(A0);
    t = micros();
    v = analogRead(A0);
    t= micros()-t;
    Serial.println(t);

    t = micros();
    for(i=0;i<1000;i++)v = analogRead(A0);
    t= micros()-t;
    Serial.println(t);
      PRREG(ADC123_COMMON->CCR);
  stm32l4_adc_create(&stm32l4_adc, ADC_INSTANCE_ADC1, STM32L4_ADC_IRQ_PRIORITY, 0);
  stm32l4_adc_enable(&stm32l4_adc, 0, NULL, NULL, 0);
  stm32l4_adc_calibrate(&stm32l4_adc);
#if 0
  //  oversample  (should hold, not cleared by core)
//  ADC1->CFGR2 = ADC_CFGR2_ROVSE | ADC_CFGR2_OVSR_0 | ADC_CFGR2_OVSR_1;  // 16x
  ADC1->CFGR2 = ADC_CFGR2_ROVSE | ADC_CFGR2_OVSR_0;  // 4x
#endif
  PRREG(ADC123_COMMON->CCR);
  PRREG(ADC1->CR);
  PRREG(ADC1->CFGR);
  PRREG(ADC1->CFGR2);   // oversample average
  PRREG(ADC1->SQR1);
  PRREG(ADC1->SMPR1);
  PRREG(ADC1->SMPR2);
}

void loop() {
  int val;
  uint32_t t;
  char str[64];

  stm32l4_adc_convert(&stm32l4_adc,9);
  t=micros();
  val = stm32l4_adc_convert(&stm32l4_adc,9);    // A0
  t=micros()-t;

  sprintf(str,"raw: A0 %d %d us",val,t);
  Serial.println(str);    

#if 0
  int channel =9;
  int adc_smp = ADC_SAMPLE_TIME_247_5;
  ADC1->SMPR1 = (channel < 10) ? (adc_smp << (channel * 3)) : 0;
  ADC1->SMPR2 = (channel >= 10) ? (adc_smp << ((channel * 3) - 30)) : 0;
#endif
  adcread();
  t=micros();
  val = adcread();    // A0
  t=micros()-t;

  sprintf(str,"my adc: A0 %d %d us",val,t);
  Serial.println(str);

  delay(3000);
}
