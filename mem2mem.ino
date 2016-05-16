#include "Arduino.h"
#include "wiring_private.h"
#define BYTES 4096
uint8_t src[BYTES] __attribute__ ((aligned (16)));
uint8_t dst[BYTES] __attribute__ ((aligned (16)));

#define DMA_OPTIONS (DMA_OPTION_MEMORY_TO_MEMORY | DMA_OPTION_MEMORY_DATA_SIZE_32  | DMA_OPTION_PERIPHERAL_DATA_SIZE_32 | DMA_OPTION_MEMORY_DATA_INCREMENT | DMA_OPTION_PERIPHERAL_DATA_INCREMENT)

stm32l4_dma_t dma;

void prmbs(char *lbl,unsigned long us,int bytes) {
    float mbs = 8.*bytes/us;
    Serial.print(mbs,2); Serial.print(" mbs  ");
    Serial.print(us); Serial.print(" us   ");
    Serial.println(lbl);
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
  stm32l4_dma_create(&dma, DMA_CHANNEL_DMA1_CH7_INDEX, DMA_OPTION_PRIORITY_MEDIUM);
  stm32l4_dma_enable(&dma, NULL, NULL);
}

void loop() {
  int i;
  unsigned long t1,t2;
  
  for (i=0;i<BYTES;i++){
    dst[i]=0;
    src[i]=i;
  }

  t1=micros();
  stm32l4_dma_start(&dma, (uint32_t)dst, (uint32_t)src, BYTES/4, DMA_OPTIONS);
  while( ! stm32l4_dma_done(&dma));
  t2 = micros() - t1;
    prmbs("dma32 copy",t2,BYTES);
  Serial.println(dst[3],DEC);
  for (i=0;i<BYTES;i++){
    dst[i]=0;
    src[i]=i;
  }

        
        t1=micros();
        for(i=0;i<BYTES;i++) dst[i] = src[i];
        t2 = micros() - t1;
    prmbs("loop copy",t2,BYTES);
        dst[3]=99;
        t1=micros();
        memcpy(dst,src,BYTES);
        t2 = micros() - t1;
    prmbs("memcpy",t2,BYTES);
        Serial.println(dst[3],DEC);
        t1=micros();
        memset(dst,66,BYTES);
        t2 = micros() - t1;
    prmbs("memset",t2,BYTES);
        Serial.println(dst[3],HEX);
        t1=micros();
        for(i=0;i<BYTES;i++) dst[i] = 66;
        t2 = micros() - t1;
    prmbs("set loop",t2,BYTES);
        Serial.println();
  delay(5000);
}
