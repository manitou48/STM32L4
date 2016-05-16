// test CRC hardware, default is ether poly
// CRC32, CRC-32/ADCCP, PKZIP, ETHERNET, 802.3 
//  (poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926)
// CRC_REV_OUT

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
#define POLY16 0x08
#define POLY8 0x10
#define POLY7 0x18
#define REV_OUT 0x80
#define REV_IN32 0x60
#define REV_IN16 0x40
#define REV_IN8 0x20

#define BUFSIZE 16384

uint8_t buf[BUFSIZE] __attribute__((aligned(4)));

void setup() {
  Serial.begin(9600);
  while(!Serial);
  delay(3000);
  
  RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;   // enable CRC
  PRREG(CRC->CR);
  PRREG(CRC->POL);
	CRC-> CR = REV_OUT | REV_IN32;
  
    for (int i=0; i<BUFSIZE; i++) {
        buf[i] = (i+1) & 0xff;
    }
}

void loop() {
          uint32_t i,us, *p = (uint32_t *)buf;
    
        CRC->CR |= 1;  // reset
        while(CRC->DR != ~0);  // wait for reset
        us = micros();
        for(i=0;i<BUFSIZE/4;i++) CRC->DR = p[i];  
        us = micros() - us;
        Serial.println(~0 ^ CRC->DR,HEX);
        Serial.println(us);
        float mbs = (8.*BUFSIZE)/us;
        Serial.println(mbs);
        
        us = 0x41414141;   // AAAA   crc should be 9b0d08f1
        CRC->CR |= 1;
        while(CRC->DR != ~0);  // wait for reset
        CRC->DR = us;
        Serial.println(~0 ^ CRC->DR,HEX);

        CRC->CR |= 1;
        while(CRC->DR != ~0);  // wait for reset
        CRC->DR = 0;
        Serial.println(CRC->DR,HEX);

    
        delay(5000); 

}
