// DEPRECATED, now part of core 
//   see STM32.getVBAT(), STM32.getVREF() and STM32.getTemperature()
#include "Arduino.h"
#include "wiring_private.h"

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define VREFMV 1212

static stm32l4_adc_t stm32l4_adc;
void setup() {
  Serial.begin(9600);
  while(!Serial);
  stm32l4_adc_create(&stm32l4_adc, ADC_INSTANCE_ADC1, STM32L4_ADC_IRQ_PRIORITY, 0);
  stm32l4_adc_enable(&stm32l4_adc, 0, NULL, NULL, 0);
  stm32l4_adc_calibrate(&stm32l4_adc);
  PRREG(ADC123_COMMON->CCR);
}

void loop() {
  int vbat,temp,vref;
  char str[64];

  
  temp = stm32l4_adc_convert(&stm32l4_adc,17);
  vbat = stm32l4_adc_convert(&stm32l4_adc,18);
  vref = stm32l4_adc_convert(&stm32l4_adc,0);
  sprintf(str,"raw: temp %d  vbat %d vref %d",temp,vbat,vref);
  Serial.println(str);
  vbat=3300*vbat/4096;
  //vref=3300*vref/4096;
  vref = 4096*VREFMV/vref;
  sprintf(str,"vbat %d mv  vref %d mv",vbat,vref);
  Serial.println(str);
  delay(3000);
}
