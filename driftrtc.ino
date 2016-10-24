// dragonfly test version pre RTC software

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define RTC_TR_RESERVED_MASK    ((uint32_t)0x007F7F7F)

#define RTC_INIT_MASK           ((uint32_t)0xFFFFFFFF)  


uint8_t RTC_Bcd2ToByte(uint8_t Value)
{
  uint32_t tmp = 0;
  tmp = ((uint8_t)(Value & (uint8_t)0xF0) >> (uint8_t)0x4) * 10;
  return (tmp + (Value & (uint8_t)0x0F));
}


uint32_t rtc_ms() {
		uint32_t ms, tmpreg, SSticks = (RTC->PRER & RTC_PRER_PREDIV_S) +1;
		uint8_t hrs,mins,secs,ss;

  do {
    ss = RTC->SSR;  // subseconds  counting down, rollover issue
    tmpreg = (uint32_t)(RTC->TR & RTC_TR_RESERVED_MASK);
    RTC->DR;   // must read DR after TR 
  } while(ss != RTC->SSR);

		hrs = (uint8_t)RTC_Bcd2ToByte((tmpreg & (RTC_TR_HT | RTC_TR_HU)) >> 16);
		mins = (uint8_t)RTC_Bcd2ToByte((tmpreg & (RTC_TR_MNT | RTC_TR_MNU)) >>8);
		secs = (uint8_t)RTC_Bcd2ToByte(tmpreg & (RTC_TR_ST | RTC_TR_SU));
		tmpreg = 3600*hrs + 60*mins + secs;
    ms = 1000*tmpreg + 1000*(SSticks-1 - ss)/SSticks;
		return ms;
}

void rtc_init() {
	// from pyboard core
	/* Disable the write protection for RTC registers */
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	// set init mode
	RTC->ISR = (uint32_t)RTC_INIT_MASK;
	while((RTC->ISR &  RTC_ISR_INITF) == 0);  // ? timeout

// default/reset prescale  256 ticks/sec
  RTC->PRER = 0x00ff | (0x007f)<<16;
//   RTC->PRER = RTC_PRER_PREDIV_S;     // 32768 ticks/sec
	RTC->DR = 0x162514;     // BCD calendar
  RTC->TR = 0;
  RTC->CR |= RTC_CR_BYPSHAD;   // need do-while if set ?

	// exit init mode
	RTC->ISR &= (uint32_t)~RTC_ISR_INIT;
	//  enable write protection
	RTC->WPR = 0xff;
 
}


#define RCC_LSI_ON 1
#define RCC_LSI_RDY 2
#define RCC_RTCCLKSOURCE_LSI ((uint32_t)0x00000200)
#define RCC_RTCCLKSOURCE_LSE ((uint32_t)0x00000100)

void initLSI() {
#if 0
    // setup LSI for RTC use   no HAL support
    PRREG(PWR->CR1);
    PRREG(RCC->CSR);
    PRREG(RCC->BDCR);
    RCC->CSR |= RCC_LSI_ON;
    while ((RCC->CSR & RCC_LSI_RDY ) ==0) ;

        // Reset Backup domain
    RCC->BDCR |= RCC_BDCR_BDRST;
    RCC->BDCR &= ~RCC_BDCR_BDRST;

    RCC->BDCR = (RCC->BDCR & ~RCC_BDCR_RTCSEL) | RCC_RTCCLKSOURCE_LSI;

    PRREG(PWR->CR1);
    PRREG(RCC->CSR);
    PRREG(RCC->BDCR);
#else
  // re enable LSE
   // RCC->BDCR |= RCC_BDCR_BDRST;
   // RCC->BDCR &= ~RCC_BDCR_BDRST;

   // RCC->BDCR = (RCC->BDCR & ~RCC_BDCR_RTCSEL) | RCC_RTCCLKSOURCE_LSE;
 
#endif
}


uint32_t s0;
void setup() {
  Serial.begin(9600);
  while(!Serial);
  pinMode(13,OUTPUT);
  delay(3500);
   //PRREG(RCC->BDCR);
   initLSI();
   RCC->BDCR |= RCC_BDCR_RTCEN;   // enable RTC

	 rtc_init();
#if 0
     PRREG(RTC->TR);
     PRREG(RTC->DR);
     PRREG(RTC->SSR);
     PRREG(RTC->ISR);
     PRREG(RTC->CR);
     PRREG(RTC->PRER);
     PRREG(RCC->BDCR);
     PRREG(RCC->CSR);
     delay(3000);
     PRREG(RTC->TR);
     PRREG(RTC->DR);
     PRREG(RTC->SSR);
     PRREG(RTC->ISR);  
#endif
}

void logger() {
  // check drift with hostdrift
  static long cnt=0;
  long ms;

  while(Serial.available() < 4);   // wait for host request
  ms = rtc_ms();
  Serial.read();
  Serial.read();
  Serial.read();
  Serial.read();
  Serial.write((uint8_t *)&ms,4);
  cnt++;
  digitalWrite(13, cnt & 1);
}

void display() {
  uint32_t ms;

  while(1) {
		ms = rtc_ms();
    Serial.println(ms);
    delay(5000);
  } 
}

void loop() {
  // logger();
  display();
}

