#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
void setup() {
  Serial.begin(9600);
  while(!Serial);
  delay(3000);
    Serial.println();Serial.print(F_CPU); Serial.print(" ");
    Serial.print(__TIME__);Serial.print(" ");Serial.println(__DATE__);

}

void loop() {
    PRREG(SysTick->VAL);
    PRREG(PWR->CR1);
    PRREG(RCC->CR);
    PRREG(RCC->PLLCFGR);
    PRREG(RCC->CFGR);
    PRREG(RCC->CSR);
    PRREG(RCC->CCIPR);
    PRREG(RCC->BDCR);
    PRREG(RCC->APB1ENR1);
    PRREG(RCC->APB1ENR2);
    PRREG(RCC->APB2ENR);
    PRREG(RCC->AHB1ENR);
    PRREG(RCC->AHB2ENR);
    PRREG(RCC->AHB3ENR);
    PRREG(DAC->CR);
    PRREG(RTC->TR);
    PRREG(RTC->DR);
    PRREG(RTC->ISR);
    PRREG(RTC->SSR);
    PRREG(RTC->CR);
    PRREG(RTC->PRER);
    PRREG(IWDG->PR);
    PRREG(IWDG->RLR);

  delay(5000);
}
