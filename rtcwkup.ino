// dragonfly test version pre RTC software   poll  version

// RTC wakeup interrupt  countdown is +1, prescale 32khz  16 8 4 2
//  assume RTC is running and set

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define RTC_TR_RESERVED_MASK    ((uint32_t)0x007F7F7F)

volatile uint32_t ticks;

extern "C" void RTC_WKUP_IRQHandler(void) {
  if (RTC->ISR & RTC_ISR_WUTF) {
	  ticks++;
	  RTC->ISR &= (uint32_t)~RTC_ISR_WUTF;   // clear wakeup
    EXTI->PR1 |= EXTI_PR1_PIF20;
  }
}


uint8_t RTC_Bcd2ToByte(uint8_t Value)
{
  uint32_t tmp = 0;
  tmp = ((uint8_t)(Value & (uint8_t)0xF0) >> (uint8_t)0x4) * 10;
  return (tmp + (Value & (uint8_t)0x0F));
}


uint32_t rtc_ms() {
		uint32_t ms, tmpreg;
		uint8_t hrs,mins,secs,ss;

    ss = RTC->SSR;  // subseconds /256 counting down, rollover issue
		tmpreg = (uint32_t)(RTC->TR & RTC_TR_RESERVED_MASK);
    RTC->DR;   // must read DR after TR 
		hrs = (uint8_t)RTC_Bcd2ToByte((tmpreg & (RTC_TR_HT | RTC_TR_HU)) >> 16);
		mins = (uint8_t)RTC_Bcd2ToByte((tmpreg & (RTC_TR_MNT | RTC_TR_MNU)) >>8);
		secs = (uint8_t)RTC_Bcd2ToByte(tmpreg & (RTC_TR_ST | RTC_TR_SU));
		tmpreg = 3600*hrs + 60*mins + secs;
        ms = 1000*(tmpreg + (255-ss)/256.);
		return ms;
}

void rtc_init() {
	// from pyboard core
	/* Disable the write protection for RTC registers */
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

  __disable_irq();         // no interrupts
  //  enable wakeup
  EXTI->PR1 |= EXTI_PR1_PIF20;    // clear
  EXTI->IMR1 |= EXTI_IMR1_IM20;      // line 20 RTC WKUP EXTI
  EXTI->RTSR1 |= EXTI_RTSR1_RT20;   // rising

  RTC->CR &= ~(RTC_CR_WUTIE |RTC_CR_WUTE);   // clear for WUT updates
  while((RTC->ISR & RTC_ISR_WUTWF)  == 0);   // wait
  RTC->CR &= ~7 ;   // clear WUCKSEL
  RTC->WUTR = 15;   // +1 is 16   1024 ticks/sec
  RTC->CR |= RTC_CR_WUTE | RTC_CR_WUTIE | 3 ;   // enable wakeup 32khz/2

	//  enable write protection
	RTC->WPR = 0xff;

  __enable_irq();         //Alarm is set, so irqs can be enabled again

  NVIC_DisableIRQ(RTC_WKUP_IRQn);
  NVIC_ClearPendingIRQ(RTC_WKUP_IRQn);
 // NVIC_SetPriority(RTC_WKUP_IRQn,0);
  NVIC_EnableIRQ(RTC_WKUP_IRQn);
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
	rtc_init();
}

void display() {
	uint32_t ms;
  char str[128];
  static int prev = 0;

	ms = rtc_ms();
  sprintf(str,"%d ms %d   %d ticks/s",ms,ticks, (ticks-prev)/5);
  Serial.println(str);
  prev=ticks;
  delay(5000); 
}

void logger() {
  // check drift with hostdrift
  static long cnt=0;
  long ms;

  while(Serial.available() < 4);   // wait for host request
  ms = ticks;
  Serial.read();
  Serial.read();
  Serial.read();
  Serial.read();
  Serial.write((uint8_t *)&ms,4);
  cnt++;
  digitalWrite(13, cnt & 1);
}

void loop() {
  display();
 // logger();   // to hostdrift -f 1024
}

