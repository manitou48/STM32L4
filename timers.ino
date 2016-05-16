// timer tests
#include "Arduino.h"
#include "wiring_private.h"

static stm32l4_timer_t mytimer;
#define FREQHZ 1000

volatile uint32_t ticks;
void timer_callback(void *context, uint32_t events) {
  ticks++;
}

void setup() {
  Serial.begin(9600);
  while(!Serial);

  stm32l4_timer_create(&mytimer, TIMER_INSTANCE_TIM7, STM32L4_TONE_IRQ_PRIORITY, 0);
  uint32_t modulus = (stm32l4_timer_clock(&mytimer) / FREQHZ) ;
  uint32_t scale   = 1;

  while (modulus > 65536) {
    modulus /= 2;
    scale++;
  }
  stm32l4_timer_enable(&mytimer, scale-1, modulus-1, 0, timer_callback, NULL, TIMER_EVENT_PERIOD);
  stm32l4_timer_start(&mytimer, false);
  char str[64];
  sprintf(str," %d hz  scale %d modulus %d  clockhz %d",FREQHZ,scale,modulus,stm32l4_timer_clock(&mytimer));
  Serial.println(str);
}

void loop() {
 Serial.println(ticks);
 delay(4000);
}
