//A5test
//  A5 reads once and hangs ???
//  A5 pin 19  PC0  ADC1  ch 1
#include "Arduino.h"
#include "wiring_private.h"

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

int adc_read(int pin) {
  int channel = g_APinDescription[pin].adc_input;
  // configure
  stm32l4_system_periph_enable(SYSTEM_PERIPH_ADC);  
  ADC1->CR &= ~ADC_CR_DEEPPWD;
  ADC1->CR |= ADC_CR_ADVREGEN;
  armv7m_clock_spin(20000);

  ADC1->ISR = ADC_ISR_ADRDY;
  ADC1->CR |= ADC_CR_ADEN;

  while (!(ADC1->ISR & ADC_ISR_ADRDY))
  {
  }

  // read
  stm32l4_gpio_pin_configure(g_APinDescription[pin].pin, (GPIO_PUPD_NONE | GPIO_MODE_ANALOG | GPIO_ANALOG_SWITCH));
    ADC1->SQR1 = (channel << 12) | (channel << 6) | 1;
    ADC1->SMPR1 = ADC_SMPR1_SMP0_2 | ADC_SMPR1_SMP1_2;  /* 47.5 */
    ADC1->ISR = ADC_ISR_EOC;
    ADC1->CR |= ADC_CR_ADSTART;
    while (!(ADC1->ISR & ADC_ISR_EOC))
    {
    }
    int val = ADC1->DR & ADC_DR_RDATA;
    
    armv7m_clock_spin(1000); // 1 us 
 #if 0
  PRREG(ADC1->SQR1);
  PRREG(ADC1->SMPR1);
  PRREG(ADC1->ISR);
  PRREG(ADC1->CR);
#endif
    
    // disable
    ADC1->CR |= ADC_CR_ADDIS;

    while (ADC1->CR & ADC_CR_ADEN)
    {
    }

    ADC1->CR &= ~ADC_CR_ADVREGEN;
    ADC1->CR |= ADC_CR_DEEPPWD;
    stm32l4_system_periph_disable(SYSTEM_PERIPH_ADC);
    return val;
}
void setup() {
 Serial.begin(9600);
 while(!Serial);
  analogReadResolution(12);
}

void loop() {
  Serial.print("A4 "); Serial.println(analogRead(A4));

 // Serial.println(analogRead(A5));
  Serial.println(adc_read(A5));

  Serial.println(adc_read(A5));

  delay(5000);
}
