// dacdma  timer trigger circular DMA 16-bits to DAC
// DMA trigger timer2 ch3  DMA1 ch 1 req 4. or  TIM7UP DMA1 ch 4  req 5
// view DAC output on scope or jumper to ADC

#include "Arduino.h"
#include "wiring_private.h"

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

static volatile uint16_t sinetable[] = {
   2047,    2147,    2248,    2348,    2447,    2545,    2642,    2737,
   2831,    2923,    3012,    3100,    3185,    3267,    3346,    3422,
   3495,    3564,    3630,    3692,    3750,    3804,    3853,    3898,
   3939,    3975,    4007,    4034,    4056,    4073,    4085,    4093,
   4095,    4093,    4085,    4073,    4056,    4034,    4007,    3975,
   3939,    3898,    3853,    3804,    3750,    3692,    3630,    3564,
   3495,    3422,    3346,    3267,    3185,    3100,    3012,    2923,
   2831,    2737,    2642,    2545,    2447,    2348,    2248,    2147,
   2047,    1948,    1847,    1747,    1648,    1550,    1453,    1358,
   1264,    1172,    1083,     995,     910,     828,     749,     673,
    600,     531,     465,     403,     345,     291,     242,     197,
    156,     120,      88,      61,      39,      22,      10,       2,
      0,       2,      10,      22,      39,      61,      88,     120,
    156,     197,     242,     291,     345,     403,     465,     531,
    600,     673,     749,     828,     910,     995,    1083,    1172,
   1264,    1358,    1453,    1550,    1648,    1747,    1847,    1948,
};

static stm32l4_timer_t mytimer;

#define DMA_OPTIONS (DMA_OPTION_MEMORY_TO_PERIPHERAL | DMA_OPTION_CIRCULAR | DMA_OPTION_MEMORY_DATA_SIZE_16  | DMA_OPTION_PERIPHERAL_DATA_SIZE_16 | DMA_OPTION_MEMORY_DATA_INCREMENT )

stm32l4_dma_t dma;

#define FREQHZ 1000

void setup() {
  Serial.begin(9600);
  while(!Serial);

  analogWriteResolution(12);  ? needed
  analogWrite(A0,0);   // init DAC
  
  stm32l4_dma_create(&dma, DMA_CHANNEL_DMA1_CH4_TIM7_UP, DMA_OPTION_PRIORITY_MEDIUM);
  stm32l4_dma_enable(&dma, NULL, NULL);
  stm32l4_dma_start(&dma, (uint32_t)&(DAC->DHR12R1), (uint32_t)sinetable, sizeof(sinetable)/2, DMA_OPTIONS);
  
  stm32l4_timer_create(&mytimer, TIMER_INSTANCE_TIM7, STM32L4_TONE_IRQ_PRIORITY, 0);
  uint32_t modulus = (stm32l4_timer_clock(&mytimer) / FREQHZ) ;
  uint32_t scale   = 1;

  while (modulus > 65536) {
    modulus /= 2;
    scale++;
  }
  stm32l4_timer_enable(&mytimer, scale -1, modulus -1, 0, NULL, NULL, TIMER_EVENT_PERIOD);
  //stm32l4_timer_notify(&mytimer, NULL, NULL, TIMER_EVENT_PERIOD);  // DIER notify DMA
  TIM7->DIER = TIM_DIER_UDE ;//| TIM_DIER_UIE;
  char str[64];
  sprintf(str,"%d hz  scale %d modulus %d  clockhz %d",FREQHZ,scale,modulus,stm32l4_timer_clock(&mytimer));
  Serial.println(str);
  stm32l4_timer_start(&mytimer, false);
}

void loop() {
  Serial.println(analogRead(A5));   // jumper DAC A0 to A5
  delay(1000);
}
