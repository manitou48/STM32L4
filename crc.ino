// test CRC hardware, default is ether poly
// CRC32, CRC-32/ADCCP, PKZIP, ETHERNET, 802.3 
//  (poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926)
// ref https://github.com/FrankBoesing/FastCRC

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

uint8_t testv[9] = {'1','2','3','4','5','6','7','8','9'};  // validation vector check

void do_crc16() {
  // CRC16 poly=0x8005 init=0xffff refin=true refout=true xorout=0x0000 check=0x4b37
  uint32_t init = 0xffff;

  CRC->CR = POLY16 | REV_OUT | REV_IN32;
  CRC->POL = 0x8005;
  Serial.print("CRC16 0x");

  // test vector "123456789"  result 4B37
  CRC->INIT = init;                 // start
  CRC->DR = 0x34333231;
  CRC->DR = 0x38373635;
  *(uint8_t *)(&(CRC->DR)) = 0x39;   // byte access
  Serial.println( CRC->DR,HEX);
  Serial.println();
}

void do_crc32(){
        uint32_t i,us,init, *p = (uint32_t *)buf;

        init = 0xffffffff;   //  seed
        
        CRC-> CR = REV_OUT | REV_IN32;
        CRC->POL = 0x04c11db7;

        Serial.print("CRC32 0x");
        // test vector "123456789"  result CBF43926
        CRC->INIT = init;
        CRC->DR = 0x34333231;
        CRC->DR = 0x38373635;
        *(uint8_t *)(&(CRC->DR)) = 0x39;   // byte access
        Serial.println(~0 ^ CRC->DR,HEX);

        CRC->CR |= 1;  // reset
        while(CRC->DR != ~0);  // wait for reset
        us = micros();
        for(i=0;i<BUFSIZE/4;i++) CRC->DR = p[i];  
        us = micros() - us;
        
        Serial.print(~0 ^ CRC->DR,HEX);  // should be 1271457F
        Serial.print(" "); 
        Serial.print(us); Serial.print(" us  ");
        float mbs = (8.*BUFSIZE)/us;
        Serial.print(mbs); Serial.println(" mbs");

        CRC->CR |= 1;
        while(CRC->DR != ~0);  // wait for reset
        CRC->DR = 0;    // should be 2144DF1C
        Serial.println(~0 ^ CRC->DR,HEX);

        // instead of reset
        CRC->INIT = init;
        CRC->DR = 0x41414141;   // AAAA   crc should be 9b0d08f1
        Serial.println(~0 ^ CRC->DR,HEX);

        Serial.println();
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
  delay(3000);
  
  RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;   // enable CRC
  PRREG(CRC->CR);
  PRREG(CRC->POL);

  for (int i=0; i<BUFSIZE; i++) {
        buf[i] = (i+1) & 0xff;
  }
}

void loop() {
  do_crc32();
  do_crc16();

  delay(5000); 
}
